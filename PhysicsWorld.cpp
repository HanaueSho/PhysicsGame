/*
	PhysicsWorld cpp
	20241211 hanaue sho
*/
#include "PhysicsWorld.h"
#include "renderer.h"
#include "collision.h"
#include "time.h"

PhysicsWorld* PhysicsWorld::instance = nullptr;

void PhysicsWorld::Update()
{
	// ここでTransformの更新
	// Update -----
	for (int i = 0; i < m_pGameObjects.size(); i++)
	{
		if (m_pGameObjects[i]->GetPhysicsBody()->_BodyType == PhysicsBody::BodyType::Dynamic)
			IntegrationForce(*m_pGameObjects[i]); // 速度更新
	
		if (m_pGameObjects[i]->GetPhysicsBody()->_BodyType == PhysicsBody::BodyType::Dynamic ||
			m_pGameObjects[i]->GetPhysicsBody()->_BodyType == PhysicsBody::BodyType::Kinematic)
				IntegrationVelocity(*m_pGameObjects[i]); // 回転、位置更新
	}

	// Narrow Phase -----
	Contact contact; // Contact 格納用
	XMFLOAT3 normal; // 軸
	float penetration; // 貫入深度
	m_ContactObjects.clear(); // 初期化
	for (int i = 0; i < m_pGameObjects.size(); i++)
	{
		auto* pAcol = m_pGameObjects[i]->GetCollision();
		PhysicsBody* pApb = m_pGameObjects[i]->GetPhysicsBody();
		for (int j = i + 1; j < m_pGameObjects.size(); j++)
		{
			auto* pBcol = m_pGameObjects[j]->GetCollision();
			PhysicsBody* pBpb = m_pGameObjects[j]->GetPhysicsBody();

			// static * static は無視
			if (pApb->_BodyType == PhysicsBody::BodyType::Static &&
				pBpb->_BodyType == PhysicsBody::BodyType::Static)
			{
				continue;
			}

			// 当たり判定 -----
			if (pAcol->isOverlap(*pBcol, &normal, &penetration))
			{
				// 衝突したので Solva へ送る
				contact = ComputeContacts(pAcol, pBcol, normal, penetration); // Contact の計算
				m_ContactObjects.push_back(contact); // 格納
			}
		}
	}

	// Velocity 解決ループ
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < m_ContactObjects.size(); j++)
		{
			for (int k = 0; k < m_ContactObjects[j]._ContactPoints.size(); k++)
			{
				SolveOneContact(m_ContactObjects[j], m_ContactObjects[j]._ContactPoints[k], k, i);
			}
		}
	}

	// メモ　ーーーーー　BodyTypeによっては動かなくしたい
	// 位置補正ループ
	for (int i = 0; i < m_ContactObjects.size(); i++)
	{
		CorrectPosition(m_ContactObjects[i]);
	}
	
}

// --------------------------------------------------
// フォース、トルクから速度更新
// --------------------------------------------------
void PhysicsWorld::IntegrationForce(GameObject& gameObject)
{
	PhysicsBody* pBody = gameObject.GetPhysicsBody();

	// 重力を加える
	XMFLOAT3 gravityForce = {
		m_Gravity.x * pBody->_Mass,
		m_Gravity.y * pBody->_Mass,
		m_Gravity.z * pBody->_Mass
	};
	pBody->AddForce(gravityForce);

	// 平行方向の加速度 a = F * (1/m)
	XMFLOAT3 acceleration = pBody->_ForceAccum * pBody->_MassInverse;
	// 角速度 a = torpue * (1/I)
	float angularAcceleration = pBody->_TorqueAccum * pBody->_InertiaInverse;

	// 半陰的オイラー積分（Semi-implicit Eular）
	pBody->_Velocity = pBody->_Velocity + acceleration * Time::deltaTime;
	pBody->_AngularVelocity = pBody->_AngularVelocity + angularAcceleration * Time::deltaTime;

	// フォース、トルク蓄積をクリア -----
	pBody->_ForceAccum = { 0, 0, 0 };
	pBody->_TorqueAccum = 0;

	// 減衰 -----
	pBody->_Velocity.x *= 1.0f / (1.0f + pBody->_LinearDamping * Time::deltaTime);
	pBody->_Velocity.y *= 1.0f / (1.0f + pBody->_LinearDamping * Time::deltaTime);
	pBody->_AngularVelocity *= 1.0f / (1.0f + pBody->_AngularDamping * Time::deltaTime);
}

// --------------------------------------------------
// 速度から位置、回転の更新
// --------------------------------------------------
void PhysicsWorld::IntegrationVelocity(GameObject& gameObject)
{
	PhysicsBody* pBody = gameObject.GetPhysicsBody();

	// 位置更新
	gameObject.GetTransform()->position.x += pBody->_Velocity.x * Time::deltaTime;
	gameObject.GetTransform()->position.y += pBody->_Velocity.y * Time::deltaTime;

	// 回転更新
	gameObject.Rotate(pBody->_AngularVelocity * Time::deltaTime);
}

// --------------------------------------------------
// 平面クリップ（Sutherland-Hodgman）
// --------------------------------------------------
Contact PhysicsWorld::ComputeContacts(Collision* pAcoll, Collision* pBcoll, const XMFLOAT3& pNormal, const float& pPenetration)
{
	// ０）形状を具体的なクラスにダウンキャスト
	BoxCollision& boxA = static_cast<BoxCollision&>(*pAcoll);
	BoxCollision& boxB = static_cast<BoxCollision&>(*pBcoll);
	BoxCollision referenceBox = boxA;
	BoxCollision incidentBox = boxB;

	Edge referenceEdge = GetReferenceEdge(pNormal, boxA, boxB, referenceBox, incidentBox);
	Edge incidentEdge = GetIncidentEdge(pNormal, incidentBox);

	std::vector<XMFLOAT3> contactPoints;
	contactPoints.push_back(incidentEdge._StartPoint);
	contactPoints.push_back(incidentEdge._EndPoint);
	contactPoints = ClipEdge(contactPoints, referenceEdge._StartPoint, referenceEdge._EndPoint - referenceEdge._StartPoint);
	contactPoints = ClipEdge(contactPoints, referenceEdge._EndPoint, referenceEdge._StartPoint - referenceEdge._EndPoint);
	contactPoints = GetContactPoints(pNormal, referenceEdge, contactPoints);

	//printf("%f\n", referenceBox.GetPosition().y);
	//printf("%f\n", boxA.GetPosition().y);
	//printf("%f\n\n", boxB.GetPosition().y);

	// ５）その他の Contact 情報と合わせて返却
	PhysicsBody* bodyA = referenceBox.GetOwner()->GetPhysicsBody(); // A を reference
	PhysicsBody* bodyB = incidentBox.GetOwner()->GetPhysicsBody(); // B を incident
	
	// ６）摩擦・反発係数を混合
	float e = std::min(bodyA->_Restitution, bodyB->_Restitution);
	float frictionS = sqrt(bodyA->_StaticFriction * bodyB->_StaticFriction);
	float frictionD = sqrt(bodyA->_DynamicFriction * bodyB->_DynamicFriction);

	// ７）Contact を構築して返却
	return Contact(
		bodyA, bodyB,
		pNormal,
		contactPoints,
		pPenetration,
		e, frictionS, frictionD
	);
}

Edge PhysicsWorld::GetReferenceEdge(const XMFLOAT3& normal, const BoxCollision& boxA, const BoxCollision& boxB, BoxCollision& outReferenceBox, BoxCollision& outIncidentBox)
{
	//float dotMax = -1; // 最大 ※今回は使わない
	float dotAx = InnerProduct(boxA.GetVectorRight(), normal);
	float dotAy = InnerProduct(boxA.GetVectorTop()  , normal);
	float dotBx = InnerProduct(boxB.GetVectorRight(), normal);
	float dotBy = InnerProduct(boxB.GetVectorTop()  , normal);
	float dotMax = dotAx;
	bool isVectRight = false;
	BoxCollision refBox = boxA;
	BoxCollision incBox = boxB;

	// ※※※　今回は矩形だけなのでこのような条件文　※※※
	if (abs(dotAx) == 1)
	{
		//dotMax = dotAx; // 更新
		refBox = boxA;
		incBox = boxB;
		isVectRight = true;
	}
	else if (abs(dotAy) == 1)
	{
		//dotMax = dotAy; // 更新
		refBox = boxA;
		incBox = boxB;
		isVectRight = false;
	}
	else if (abs(dotBx) == 1)
	{
		//dotMax = dotBx; // 更新
		refBox = boxB;
		incBox = boxA;
		isVectRight = true;
	}
	else if (abs(dotBy) == 1)
	{
		//dotMax = dotBy; // 更新
		refBox = boxB;
		incBox = boxA;
		isVectRight = false;
	}
	outReferenceBox = refBox;
	outIncidentBox = incBox;

	// 辺の算出
	Edge referenceEdge;
	XMFLOAT3 center = outReferenceBox.GetPosition(); 
	XMFLOAT3 halfLength = outReferenceBox.GetSize() / 2;
	XMFLOAT3 normalOrth = CrossProduct(normal, { 0, 0, 1 }); // 直交ベクトルの取得

	if (!isVectRight) // 反転処理
		halfLength = { halfLength.y, halfLength.x, halfLength.z };

	referenceEdge._StartPoint = {
		center.x + normal.x * halfLength.x + normalOrth.x * halfLength.y,
		center.y + normal.y * halfLength.x + normalOrth.y * halfLength.y,
		center.z
	};
	referenceEdge._EndPoint = {
		center.x + normal.x * halfLength.x - normalOrth.x * halfLength.y,
		center.y + normal.y * halfLength.x - normalOrth.y * halfLength.y,
		center.z
	};

	return referenceEdge;
}

Edge PhysicsWorld::GetIncidentEdge(const XMFLOAT3& normal, const BoxCollision& incidentBox)
{
	// 被参照エッジを探す　-normalと内積を取る
	float dotRight  = InnerProduct(incidentBox.GetVectorRight()     , normal * -1);
	float dotLeft   = InnerProduct(incidentBox.GetVectorRight() * -1, normal * -1);
	float dotTop    = InnerProduct(incidentBox.GetVectorTop()       , normal * -1);
	float dotBottom = InnerProduct(incidentBox.GetVectorTop() * -1  , normal * -1);
	float dotMax = -1;
	XMFLOAT3 vect = {0, 0, 0};

	if (dotRight > dotMax)
	{
		dotMax = dotRight;
		vect = incidentBox.GetVectorRight();
	}
	if (dotLeft > dotMax)
	{
		dotMax = dotLeft;
		vect = incidentBox.GetVectorRight() * -1;
	}
	if (dotTop > dotMax)
	{
		dotMax = dotTop;
		vect = incidentBox.GetVectorTop();
	}
	if (dotBottom > dotMax)
	{
		dotMax = dotBottom;
		vect = incidentBox.GetVectorTop() * -1;
	}

	// 辺の算出
	Edge incidentEdge;
	XMFLOAT3 center = incidentBox.GetPosition();
	XMFLOAT3 halfLength = incidentBox.GetSize() / 2;
	XMFLOAT3 vectOrth = CrossProduct(vect, { 0, 0, 1 }); // 直交ベクトルの取得

	incidentEdge._StartPoint = {
		center.x + vect.x * halfLength.x + vectOrth.x * halfLength.y,
		center.y + vect.y * halfLength.x + vectOrth.y * halfLength.y,
		center.z
	};
	incidentEdge._EndPoint = {
		center.x + vect.x * halfLength.x - vectOrth.x * halfLength.y,
		center.y + vect.y * halfLength.x - vectOrth.y * halfLength.y,
		center.z
	};

	return incidentEdge;
}

std::vector<XMFLOAT3> PhysicsWorld::ClipEdge(std::vector<XMFLOAT3> contactPoints, const XMFLOAT3& clipEdge, const XMFLOAT3& clipNormal)
{
	std::vector<XMFLOAT3> outPoints; // 接触点出力用
	if (contactPoints.size() == 0) return contactPoints; // サイズ０なので返す
	if (contactPoints.size() == 1)  // サイズ１
	{
		XMFLOAT3 P = contactPoints[0]; // 終点
		float dotP = InnerProduct(P - clipEdge, clipNormal);
		if (dotP >= 0) // P は内側にあるので格納
			outPoints.push_back(P);
		return outPoints;
	}

	XMFLOAT3 P = contactPoints[0]; 
	XMFLOAT3 Q = contactPoints[1]; 
	float dotP = InnerProduct(P - clipEdge, clipNormal);
	float dotQ = InnerProduct(Q - clipEdge, clipNormal);

	if (dotP >= 0) // P は内側にあるので格納
		outPoints.push_back(P);
	if (dotQ >= 0) // Q は内側にあるので格納
		outPoints.push_back(Q);
	
	if (dotP * dotQ < 0) // 片方が外側にある
	{
		float t = dotP / (dotP - dotQ);
		// P + t*(Q-P)
		outPoints.push_back({ P.x + (Q.x - P.x) * t, P.y + (Q.y - P.y) * t, 0 });
	}

	return outPoints; // ※出力頂点は順不同なので注意※
}

std::vector<XMFLOAT3> PhysicsWorld::GetContactPoints(const XMFLOAT3& normal, const Edge& referenceEdge, const std::vector<XMFLOAT3> contactPoints)
{
	std::vector<XMFLOAT3> out;

	for (int i = 0; i < contactPoints.size(); i++)
	{
		XMFLOAT3 vect = contactPoints[i] - referenceEdge._StartPoint;
		if (InnerProduct(vect, normal * -1) >= 0) // 内側にある
			out.push_back(contactPoints[i]);
	}

	return out;
}

void PhysicsWorld::SolveOneContact(Contact& contact, const XMFLOAT3& contactPoint, int k, int iteration)
{
	// 要素の準備 -----
	PhysicsBody* pBodyA = contact._pBodyA; // PhysicsBody
	PhysicsBody* pBodyB = contact._pBodyB; // PhysicsBody
	XMFLOAT3 normal = contact._Normal; // 衝突法線ベクトル
	float restitution = contact._Restitution; // 反発係数

	XMFLOAT3 vectAtoContact = contactPoint - pBodyA->GetOwner()->GetPosition(); // 接触点へのベクトル
	XMFLOAT3 vectBtoContact = contactPoint - pBodyB->GetOwner()->GetPosition(); // 接触点へのベクトル

	XMFLOAT3 velocityAtContactA = pBodyA->_Velocity + CrossProduct(pBodyA->_AngularVelocity, vectAtoContact); // 接触点の線形速度　＋　回転による速度
	XMFLOAT3 velocityAtContactB = pBodyB->_Velocity + CrossProduct(pBodyB->_AngularVelocity, vectBtoContact); // 接触点の線形速度　＋　回転による速度

	XMFLOAT3 relativeVelocity = velocityAtContactB - velocityAtContactA; // 接触点での相対速度（AからみたBの速度）
	float	 relativeVelocityN = InnerProduct(relativeVelocity, normal); // 相対速度の法線方向成分

	if (relativeVelocityN >= 0.0f)
	{
		//printf("normal: %f, %f, %f\n", normal.x, normal.y, normal.z);
		//printf("veloci: %f, %f, %f\n\n", relativeVelocity.x, relativeVelocity.y, relativeVelocity.z);
		return; // 離れているので反発不要
	}
	
	float rotationEffectA = InnerProduct(CrossProduct(vectAtoContact, normal), CrossProduct(vectAtoContact, normal) * pBodyA->_InertiaInverse); 
	float rotationEffectB = InnerProduct(CrossProduct(vectBtoContact, normal), CrossProduct(vectBtoContact, normal) * pBodyB->_InertiaInverse); 

	float j = -(1.0f + restitution) * relativeVelocityN / (pBodyA->_MassInverse + pBodyB->_MassInverse + rotationEffectA + rotationEffectB); // インパルスベクトルの強さ
	XMFLOAT3 impulse = normal * j; // 実際に剛体に加える力

	// 速度更新
	pBodyA->_Velocity		 = pBodyA->_Velocity - impulse * pBodyA->_MassInverse;
	pBodyA->_AngularVelocity = pBodyA->_AngularVelocity - CrossProduct(vectAtoContact, impulse).z * pBodyA->_InertiaInverse;
	pBodyB->_Velocity		 = pBodyB->_Velocity + impulse * pBodyB->_MassInverse;
	pBodyB->_AngularVelocity = pBodyB->_AngularVelocity + CrossProduct(vectBtoContact, impulse).z * pBodyB->_InertiaInverse;

}

void PhysicsWorld::CorrectPosition(const Contact& contact)
{
	// 各要素を取り出す
	PhysicsBody* pBodyA = contact._pBodyA;
	PhysicsBody* pBodyB = contact._pBodyB;
	XMFLOAT3 normal = contact._Normal; // 法線
	float penetration = contact._Penetration; // 貫入深度

	// 定数
	const float slop = 0.001f; // めり込み許容値
	const float percent = 0.2f; // 貫入の何割を補正するか

	// slop以下の微小なめり込みは許容し、それ以上を補正対象とする
	float correctionDepth = std::max(penetration - slop, 0.0f);
	float totalInverseMass = pBodyA->_MassInverse + pBodyB->_MassInverse; // 合計逆質量
	if (totalInverseMass == 0.0f) return; // どちらも静的なので無視

	float correctionMagnitude = (correctionDepth / totalInverseMass) * percent;
	XMFLOAT3 correction = normal * correctionMagnitude;

	// 位置を逆質量比で動かす
	pBodyA->GetOwner()->GetTransform()->position.x -= correction.x * pBodyA->_MassInverse;
	pBodyA->GetOwner()->GetTransform()->position.y -= correction.y * pBodyA->_MassInverse;
	pBodyA->GetOwner()->GetTransform()->position.z -= correction.z * pBodyA->_MassInverse;
	pBodyB->GetOwner()->GetTransform()->position.x += correction.x * pBodyB->_MassInverse;
	pBodyB->GetOwner()->GetTransform()->position.y += correction.y * pBodyB->_MassInverse;
	pBodyB->GetOwner()->GetTransform()->position.z += correction.z * pBodyB->_MassInverse;
}


void PhysicsWorld::RegisterGameObject(GameObject* pObject)
{
	m_pGameObjects.push_back(pObject);
}

void PhysicsWorld::DeleteGameObject(GameObject* pObject)
{
	m_pGameObjects.erase(std::remove(m_pGameObjects.begin(), m_pGameObjects.end(), pObject), m_pGameObjects.end());
}

std::vector<GameObject*> PhysicsWorld::GetGameObject(const std::string& name)
{
	std::vector<GameObject*> Objects2d;

	for (int i = 0; i < m_pGameObjects.size(); i++)
	{
		if (m_pGameObjects[i]->GetTag() == name)
			Objects2d.push_back(m_pGameObjects[i]);
	}

	return Objects2d;
}


/*
  備忘録
  なぜかうまくいかないので一度戻す
// 当たり判定 -----
			if (pBpb->_BodyType == PhysicsBody::BodyType::Dynamic && pAcol->isOverlap(*pBcol, &normal, &penetration))
			{
				// 衝突したので Solva へ送る
				printf("あたったよ");
				contact = ComputeContacts(pAcol, pBcol, normal, penetration); // Contact の計算
				m_ContactObjects.push_back(contact); // 格納
			}
			else if (pApb->_BodyType == PhysicsBody::BodyType::Dynamic && pBcol->isOverlap(*pAcol, &normal, &penetration))
			{
				// 衝突したので Solva へ送る
				printf("あたったよ");
				contact = ComputeContacts(pBcol, pAcol, normal, penetration); // Contact の計算
				m_ContactObjects.push_back(contact); // 格納
			}

*/

/*
Contact PhysicsWorld::ComputeContacts(Collision2d* pAcoll, Collision2d* pBcoll, const XMFLOAT3& pNormal, const float& pPenetration)
{
	// ０）形状を具体的なクラスにダウンキャスト
	Box& boxA = static_cast<Box&>(*pAcoll);
	Box& boxB = static_cast<Box&>(*pBcoll);

	// １）参照形状(Reference)と被参照形状(Incident)を決定
	// 衝突法線に最も近い軸を持つ box を”参照形状”にする
	XMFLOAT3 normal = pNormal;
	float dotAx = fabsf(InnerProduct(boxA.GetVectorRight(), normal));
	float dotAy = fabsf(InnerProduct(boxA.GetVectorTop(), normal));
	float dotBx = fabsf(InnerProduct(boxB.GetVectorRight(), normal));
	float dotBy = fabsf(InnerProduct(boxB.GetVectorTop(), normal));

	bool AisRef = ((dotAx > dotBx) && (dotAx > dotBy))
				|| ((dotAy > dotBx) && (dotAy > dotBy));

	Box& refB = AisRef ? boxA : boxB; // 参照形状ボックス
	Box& incB = AisRef ? boxB : boxA; // 被参照形状ボックス
	XMFLOAT3 faceNormal = AisRef ? pNormal : XMFLOAT3{ -pNormal.x,-pNormal.y,-pNormal.z };

	// ２）被参照フェイスの２頂点を列挙
	std::vector<XMFLOAT3> poly;
	{
		// incB：被参照形状、faceNormal：boxA→boxB方向の法線を反転して使う
		XMFLOAT3 faceDir = faceNormal * -1;

		// incB の中心、ローカル軸、半 extents
		XMFLOAT3 center = incB.GetPosition();
		XMFLOAT3 ux = incB.GetVectorRight();
		XMFLOAT3 uy = incB.GetVectorTop();
		float ex = incB.GetSize().x * 0.5f;
		float ey = incB.GetSize().y * 0.5f;

		float dotUX = fabsf(InnerProduct(ux, faceDir));
		float dotUY = fabsf(InnerProduct(uy, faceDir));


		// facedir に近い軸を判定し、フェイスの面を春 orth（エッジ方向）を選ぶ
		XMFLOAT3 faceAxis; // フェイス面の法線
		float faceExtent; //
		XMFLOAT3 orthAxis; // 面に直交するエッジ方向の軸
		float orthExtent; // エッジの半分の長さ
		if (dotUX > dotUY)
		{
			// faceDir が ux 方向より　⇒　フェイスは左右の面　⇒　エッジ軸は uy
			faceAxis = ux * Sign(InnerProduct(ux, faceDir));
			faceExtent = ex;
			orthAxis = uy;
			orthExtent = ey;
		}
		else
		{
			// faceDir が uy 方向寄り　⇒　フェイスは上下の面　⇒　エッジ軸は ux
			faceAxis = uy * Sign(InnerProduct(uy, faceDir));
			faceExtent = ey;
			orthAxis = ux;
			orthExtent = ex;
		}
		// ２頂点を poly にセット
		XMFLOAT3 p_1 = {
			center.x + faceAxis.x * faceExtent + orthAxis.x * orthExtent,
			center.y + faceAxis.y * faceExtent + orthAxis.y * orthExtent,
			center.z
		};
		XMFLOAT3 p_2 = {
			center.x + faceAxis.x * faceExtent - orthAxis.x * orthExtent,
			center.y + faceAxis.y * faceExtent - orthAxis.y * orthExtent,
			center.z
		};

		poly = { p_1, p_2 };
	}


	// ３）参照フェイスの２辺を定義（フェイス法線 = refaxis に沿う面を選ぶ）
	// refCorners のうち、法線方向で最も遠い２点を選びその２点を結ぶエッジがクリッピングの基準になる
	XMFLOAT3 ux = refB.GetVectorRight(), uy = refB.GetVectorTop();
	float ex = refB.GetSize().x * 0.5f, ey = refB.GetSize().y * 0.5f;
	XMFLOAT3 sideAxis;
	float sideExtent;
	if (fabsf(InnerProduct(ux, faceNormal)) > fabsf(InnerProduct(uy, faceNormal)))
	{
		sideAxis = uy;
		sideExtent = ey;
	}
	else
	{
		sideAxis = ux;
		sideExtent = ex;
	}
	XMFLOAT3 center = refB.GetPosition();
	// 平面１：sideAxis.x <= offset_1
	XMFLOAT3 normal_1 = sideAxis;
	float offset_1 = InnerProduct(normal_1, { center.x + sideAxis.x * sideExtent, center.y + sideAxis.y * sideExtent, 0 });

	// 平面２：-sideAxis.x <= offset_2
	XMFLOAT3 normal_2 = { -sideAxis.x, -sideAxis.y, 0 };
	float offset_2 = InnerProduct(normal_2, { center.x - sideAxis.x * sideExtent, center.y - sideAxis.y * sideExtent, 0 });

	// ４）クリップ処理：poly を順に平面１→平面２で切り出す
	auto clip = [&](const XMFLOAT3& normal, float offset) {
		std::vector<XMFLOAT3> out;
		int N = (int)poly.size();
		for (int i = 0; i < N; i++)
		{
			auto P = poly[i];
			auto Q = poly[(i + 1) % N];
			float dP = InnerProduct(normal, P) - offset;
			float dQ = InnerProduct(normal, Q) - offset;
			//（１）P が内側なら残す
			if (dP <= 0) out.push_back(P);
			// （２）辺が交差するなら交点を追加
			if (dP * dQ < 0)
			{
				float t = dP / (dP - dQ);
				// P + t*(Q-P)
				out.push_back({ P.x + (Q.x - P.x) * t, P.y + (Q.y - P.y) * t, 0 });
			}
		}
		poly.swap(out);
	};
	clip(normal_1, offset_1);
	clip(normal_2, offset_2);

	// ５）その他の Contact 情報と合わせて返却
	PhysicsBody* bodyA =  pAcoll->GetOwner()->GetPhysicsBody();
	PhysicsBody* bodyB =  pBcoll->GetOwner()->GetPhysicsBody();

	// ６）摩擦・反発係数を混合
	float e = std::min(bodyA->_Restitution, bodyB->_Restitution);
	float frictionS = sqrt(bodyA->_StaticFriction * bodyB->_StaticFriction);
	float frictionD = sqrt(bodyA->_DynamicFriction * bodyB->_DynamicFriction);

	// ７）Contact を構築して返却
	return Contact(
		bodyA, bodyB,
		faceNormal,
		poly,
		pPenetration,
		e, frictionS, frictionD
	);
}

*/

/*

	// 各要素を取り出す
	PhysicsBody* pBodyA = contact._pBodyA;
	PhysicsBody* pBodyB = contact._pBodyB;
	XMFLOAT3 contactNormal = contact._Normal;
	float restitution = iteration == 0 ? contact._Restitution : 0.0f; // １回目のループのときのみ適用させる
	float staticFriction = contact._StaticFriction;
	float dynamicFriction = contact._DynamicFriction;

	// １）接触点位置への各ボディ中心からのベクトル
	XMFLOAT3 vectToContactPointFromA = contactPoint - pBodyA->GetOwner()->GetPosition();
	XMFLOAT3 vectToContactPointFromB = contactPoint - pBodyB->GetOwner()->GetPosition();

	// ２）接触点での点速度
	XMFLOAT3 pointVelocityA = pBodyA->_Velocity + CrossProduct(pBodyA->_AngularVelocity, vectToContactPointFromA);
	XMFLOAT3 pointVelocityB = pBodyB->_Velocity + CrossProduct(pBodyB->_AngularVelocity, vectToContactPointFromB);

	// ３）相対速度と法線成分 -----
	XMFLOAT3 relativeVelocity = pointVelocityB - pointVelocityA;
	float velocityAlongNormal = InnerProduct(relativeVelocity, contactNormal);
	//printf("vn (before impulse) = %.6f\n", velocityAlongNormal); // DEBUG
	if (velocityAlongNormal > 0) return; // 離れている方向に動いているので衝突解決不要

	// ４）法線方向のインパルスの大きさ
	float inverseMassSum = pBodyA->_MassInverse + pBodyB->_MassInverse; // 逆質量の和
	float crossA = vectToContactPointFromA.x * contactNormal.y - vectToContactPointFromA.y * contactNormal.x; // 接触点法線と接触点から中心へのベクトルの外積
	float crossB = vectToContactPointFromB.x * contactNormal.y - vectToContactPointFromB.y * contactNormal.x; // 接触点法線と接触点から中心へのベクトルの外積
	float rotationalTermA = crossA * crossA * pBodyA->_InertiaInverse; // 外積×外積×逆慣性
	float rotationalTermB = crossB * crossB * pBodyB->_InertiaInverse; // 外積×外積×逆慣性
	float denominator = inverseMassSum + rotationalTermA + rotationalTermB; //
	float impulseMagnitude = -(1.0f + restitution) * velocityAlongNormal / denominator; //

	// ５）法線インパルスを速度に適用（累計方式）
	float oldNormalImpulse = contact._NormalImpulseAccum[k];
	float newNormalImpulse = std::max(oldNormalImpulse + impulseMagnitude, 0.0f);// 負にならないようにクランプ
	float dentaNormalImpulse = newNormalImpulse - oldNormalImpulse; // 差分
	contact._NormalImpulseAccum[k] = newNormalImpulse; // 更新
	//printf("normalImpulseAccum: %f\n", newNormalImpulse); // DEBUG

	XMFLOAT3 impulse = contactNormal * dentaNormalImpulse;
	pBodyA->ApplyImpulse(impulse * -1, vectToContactPointFromA);
	pBodyB->ApplyImpulse(impulse *  1, vectToContactPointFromB);

	/*
	XMFLOAT3 impulseVector = contactNormal * impulseMagnitude;
	pBodyA->_Velocity		   = pBodyA->_Velocity		- impulseVector * pBodyA->_MassInverse;
	pBodyA->_AngularVelocity = pBodyA->_AngularVelocity - (vectToContactPointFromA.x * impulseVector.y - vectToContactPointFromA.y * impulseVector.x) * pBodyA->_InertiaInverse;
	pBodyB->_Velocity		   = pBodyB->_Velocity		+ impulseVector * pBodyB->_MassInverse;
	pBodyB->_AngularVelocity = pBodyB->_AngularVelocity + (vectToContactPointFromB.x * impulseVector.y - vectToContactPointFromB.y * impulseVector.x) * pBodyB->_InertiaInverse;
	

	// ６）摩擦方向単位ベクトル -----
XMFLOAT3 tangent = relativeVelocity - (contactNormal * velocityAlongNormal);
if (LengthFloat3(tangent) > 0) tangent = Normalize(tangent);
else return; // 速度がピッタリ法線方向なら摩擦不要

// ７）摩擦インパルス大きさ（今回は厳密に求める。ゲームエンジン程度なら法線インパルスのdenomを流用しても問題ないらしい）
float velocityAlongTangent = InnerProduct(relativeVelocity, tangent);
float tangentCrossA = vectToContactPointFromA.x * tangent.y - vectToContactPointFromA.y * tangent.x;
float tangentCrossB = vectToContactPointFromB.x * tangent.y - vectToContactPointFromB.y * tangent.x;
float rotationalTangentA = tangentCrossA * tangentCrossA * pBodyA->_InertiaInverse;
float rotationalTangentB = tangentCrossB * tangentCrossB * pBodyB->_InertiaInverse;
float frictionDenominator = inverseMassSum + rotationalTangentA + rotationalTangentB; // 接線方向に効く有効質量（逆質量和＋回転項）
float frictionMagnitude = -velocityAlongTangent / frictionDenominator; // 

// ８）クローン摩擦のクランプ
float maxStaticFriction = contact._StaticFriction * newNormalImpulse;
float maxDynamicFriction = contact._DynamicFriction * newNormalImpulse;
float oldTangentImpulse = contact._TangentImpulseAccum[k];
float newTangentImpulse = oldTangentImpulse + frictionMagnitude;
if (abs(oldTangentImpulse + frictionMagnitude) < maxStaticFriction)
{
	// 静止摩擦力領域
	newTangentImpulse = oldTangentImpulse + frictionMagnitude;
}
else
{
	// 動摩擦力領域
	if (oldTangentImpulse + frictionMagnitude > 0.0f)
		newTangentImpulse = maxDynamicFriction;
	else
		newTangentImpulse = -maxDynamicFriction;
}
float deltaTangentImpulse = newTangentImpulse - oldTangentImpulse;
contact._TangentImpulseAccum[k] = newTangentImpulse;

float desiredTangentImpulse = frictionMagnitude;
printf("[Debug] desiredT=%.6f  oldT=%.6f  newT=%.6f  deltaT=%.6f\n",
	desiredTangentImpulse,
	oldTangentImpulse,
	newTangentImpulse,
	deltaTangentImpulse);


XMFLOAT3 impulseT = tangent * deltaTangentImpulse;
pBodyA->ApplyImpulse(impulseT * -1, vectToContactPointFromA);
pBodyB->ApplyImpulse(impulseT * 1, vectToContactPointFromB);

/*
float maxStaticFriction = impulseMagnitude * staticFriction;
XMFLOAT3 frictionImpulse;
if (abs(frictionMagnitude) < maxStaticFriction)
	frictionImpulse = tangent * frictionMagnitude;
else
	frictionImpulse = tangent * (-impulseMagnitude * dynamicFriction);

// ９）摩擦インパルス適用
pBodyA->_Velocity        = pBodyA->_Velocity        - frictionImpulse * pBodyA->_MassInverse;
pBodyA->_AngularVelocity = pBodyA->_AngularVelocity - pBodyA->_InertiaInverse * (vectToContactPointFromA.x * frictionImpulse.y - vectToContactPointFromA.y * frictionImpulse.x);
pBodyB->_Velocity        = pBodyB->_Velocity		+ frictionImpulse * pBodyB->_MassInverse;
pBodyB->_AngularVelocity = pBodyB->_AngularVelocity + pBodyB->_InertiaInverse * (vectToContactPointFromB.x * frictionImpulse.y - vectToContactPointFromB.y * frictionImpulse.x);
*/

