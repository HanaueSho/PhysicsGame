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
	// ������Transform�̍X�V
	// Update -----
	for (int i = 0; i < m_pGameObjects.size(); i++)
	{
		if (m_pGameObjects[i]->GetPhysicsBody()->_BodyType == PhysicsBody::BodyType::Dynamic)
			IntegrationForce(*m_pGameObjects[i]); // ���x�X�V
	
		if (m_pGameObjects[i]->GetPhysicsBody()->_BodyType == PhysicsBody::BodyType::Dynamic ||
			m_pGameObjects[i]->GetPhysicsBody()->_BodyType == PhysicsBody::BodyType::Kinematic)
				IntegrationVelocity(*m_pGameObjects[i]); // ��]�A�ʒu�X�V
	}

	// Narrow Phase -----
	Contact contact; // Contact �i�[�p
	XMFLOAT3 normal; // ��
	float penetration; // �ѓ��[�x
	m_ContactObjects.clear(); // ������
	for (int i = 0; i < m_pGameObjects.size(); i++)
	{
		auto* pAcol = m_pGameObjects[i]->GetCollision();
		PhysicsBody* pApb = m_pGameObjects[i]->GetPhysicsBody();
		for (int j = i + 1; j < m_pGameObjects.size(); j++)
		{
			auto* pBcol = m_pGameObjects[j]->GetCollision();
			PhysicsBody* pBpb = m_pGameObjects[j]->GetPhysicsBody();

			// static * static �͖���
			if (pApb->_BodyType == PhysicsBody::BodyType::Static &&
				pBpb->_BodyType == PhysicsBody::BodyType::Static)
			{
				continue;
			}

			// �����蔻�� -----
			if (pAcol->isOverlap(*pBcol, &normal, &penetration))
			{
				// �Փ˂����̂� Solva �֑���
				contact = ComputeContacts(pAcol, pBcol, normal, penetration); // Contact �̌v�Z
				m_ContactObjects.push_back(contact); // �i�[
			}
		}
	}

	// Velocity �������[�v
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

	// �����@�[�[�[�[�[�@BodyType�ɂ���Ă͓����Ȃ�������
	// �ʒu�␳���[�v
	for (int i = 0; i < m_ContactObjects.size(); i++)
	{
		CorrectPosition(m_ContactObjects[i]);
	}
	
}

// --------------------------------------------------
// �t�H�[�X�A�g���N���瑬�x�X�V
// --------------------------------------------------
void PhysicsWorld::IntegrationForce(GameObject& gameObject)
{
	PhysicsBody* pBody = gameObject.GetPhysicsBody();

	// �d�͂�������
	XMFLOAT3 gravityForce = {
		m_Gravity.x * pBody->_Mass,
		m_Gravity.y * pBody->_Mass,
		m_Gravity.z * pBody->_Mass
	};
	pBody->AddForce(gravityForce);

	// ���s�����̉����x a = F * (1/m)
	XMFLOAT3 acceleration = pBody->_ForceAccum * pBody->_MassInverse;
	// �p���x a = torpue * (1/I)
	float angularAcceleration = pBody->_TorqueAccum * pBody->_InertiaInverse;

	// ���A�I�I�C���[�ϕ��iSemi-implicit Eular�j
	pBody->_Velocity = pBody->_Velocity + acceleration * Time::deltaTime;
	pBody->_AngularVelocity = pBody->_AngularVelocity + angularAcceleration * Time::deltaTime;

	// �t�H�[�X�A�g���N�~�ς��N���A -----
	pBody->_ForceAccum = { 0, 0, 0 };
	pBody->_TorqueAccum = 0;

	// ���� -----
	pBody->_Velocity.x *= 1.0f / (1.0f + pBody->_LinearDamping * Time::deltaTime);
	pBody->_Velocity.y *= 1.0f / (1.0f + pBody->_LinearDamping * Time::deltaTime);
	pBody->_AngularVelocity *= 1.0f / (1.0f + pBody->_AngularDamping * Time::deltaTime);
}

// --------------------------------------------------
// ���x����ʒu�A��]�̍X�V
// --------------------------------------------------
void PhysicsWorld::IntegrationVelocity(GameObject& gameObject)
{
	PhysicsBody* pBody = gameObject.GetPhysicsBody();

	// �ʒu�X�V
	gameObject.GetTransform()->position.x += pBody->_Velocity.x * Time::deltaTime;
	gameObject.GetTransform()->position.y += pBody->_Velocity.y * Time::deltaTime;

	// ��]�X�V
	gameObject.Rotate(pBody->_AngularVelocity * Time::deltaTime);
}

// --------------------------------------------------
// ���ʃN���b�v�iSutherland-Hodgman�j
// --------------------------------------------------
Contact PhysicsWorld::ComputeContacts(Collision* pAcoll, Collision* pBcoll, const XMFLOAT3& pNormal, const float& pPenetration)
{
	// �O�j�`�����̓I�ȃN���X�Ƀ_�E���L���X�g
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

	// �T�j���̑��� Contact ���ƍ��킹�ĕԋp
	PhysicsBody* bodyA = referenceBox.GetOwner()->GetPhysicsBody(); // A �� reference
	PhysicsBody* bodyB = incidentBox.GetOwner()->GetPhysicsBody(); // B �� incident
	
	// �U�j���C�E�����W��������
	float e = std::min(bodyA->_Restitution, bodyB->_Restitution);
	float frictionS = sqrt(bodyA->_StaticFriction * bodyB->_StaticFriction);
	float frictionD = sqrt(bodyA->_DynamicFriction * bodyB->_DynamicFriction);

	// �V�jContact ���\�z���ĕԋp
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
	//float dotMax = -1; // �ő� ������͎g��Ȃ�
	float dotAx = InnerProduct(boxA.GetVectorRight(), normal);
	float dotAy = InnerProduct(boxA.GetVectorTop()  , normal);
	float dotBx = InnerProduct(boxB.GetVectorRight(), normal);
	float dotBy = InnerProduct(boxB.GetVectorTop()  , normal);
	float dotMax = dotAx;
	bool isVectRight = false;
	BoxCollision refBox = boxA;
	BoxCollision incBox = boxB;

	// �������@����͋�`�����Ȃ̂ł��̂悤�ȏ������@������
	if (abs(dotAx) == 1)
	{
		//dotMax = dotAx; // �X�V
		refBox = boxA;
		incBox = boxB;
		isVectRight = true;
	}
	else if (abs(dotAy) == 1)
	{
		//dotMax = dotAy; // �X�V
		refBox = boxA;
		incBox = boxB;
		isVectRight = false;
	}
	else if (abs(dotBx) == 1)
	{
		//dotMax = dotBx; // �X�V
		refBox = boxB;
		incBox = boxA;
		isVectRight = true;
	}
	else if (abs(dotBy) == 1)
	{
		//dotMax = dotBy; // �X�V
		refBox = boxB;
		incBox = boxA;
		isVectRight = false;
	}
	outReferenceBox = refBox;
	outIncidentBox = incBox;

	// �ӂ̎Z�o
	Edge referenceEdge;
	XMFLOAT3 center = outReferenceBox.GetPosition(); 
	XMFLOAT3 halfLength = outReferenceBox.GetSize() / 2;
	XMFLOAT3 normalOrth = CrossProduct(normal, { 0, 0, 1 }); // �����x�N�g���̎擾

	if (!isVectRight) // ���]����
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
	// ��Q�ƃG�b�W��T���@-normal�Ɠ��ς����
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

	// �ӂ̎Z�o
	Edge incidentEdge;
	XMFLOAT3 center = incidentBox.GetPosition();
	XMFLOAT3 halfLength = incidentBox.GetSize() / 2;
	XMFLOAT3 vectOrth = CrossProduct(vect, { 0, 0, 1 }); // �����x�N�g���̎擾

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
	std::vector<XMFLOAT3> outPoints; // �ڐG�_�o�͗p
	if (contactPoints.size() == 0) return contactPoints; // �T�C�Y�O�Ȃ̂ŕԂ�
	if (contactPoints.size() == 1)  // �T�C�Y�P
	{
		XMFLOAT3 P = contactPoints[0]; // �I�_
		float dotP = InnerProduct(P - clipEdge, clipNormal);
		if (dotP >= 0) // P �͓����ɂ���̂Ŋi�[
			outPoints.push_back(P);
		return outPoints;
	}

	XMFLOAT3 P = contactPoints[0]; 
	XMFLOAT3 Q = contactPoints[1]; 
	float dotP = InnerProduct(P - clipEdge, clipNormal);
	float dotQ = InnerProduct(Q - clipEdge, clipNormal);

	if (dotP >= 0) // P �͓����ɂ���̂Ŋi�[
		outPoints.push_back(P);
	if (dotQ >= 0) // Q �͓����ɂ���̂Ŋi�[
		outPoints.push_back(Q);
	
	if (dotP * dotQ < 0) // �Е����O���ɂ���
	{
		float t = dotP / (dotP - dotQ);
		// P + t*(Q-P)
		outPoints.push_back({ P.x + (Q.x - P.x) * t, P.y + (Q.y - P.y) * t, 0 });
	}

	return outPoints; // ���o�͒��_�͏��s���Ȃ̂Œ��Ӂ�
}

std::vector<XMFLOAT3> PhysicsWorld::GetContactPoints(const XMFLOAT3& normal, const Edge& referenceEdge, const std::vector<XMFLOAT3> contactPoints)
{
	std::vector<XMFLOAT3> out;

	for (int i = 0; i < contactPoints.size(); i++)
	{
		XMFLOAT3 vect = contactPoints[i] - referenceEdge._StartPoint;
		if (InnerProduct(vect, normal * -1) >= 0) // �����ɂ���
			out.push_back(contactPoints[i]);
	}

	return out;
}

void PhysicsWorld::SolveOneContact(Contact& contact, const XMFLOAT3& contactPoint, int k, int iteration)
{
	// �v�f�̏��� -----
	PhysicsBody* pBodyA = contact._pBodyA; // PhysicsBody
	PhysicsBody* pBodyB = contact._pBodyB; // PhysicsBody
	XMFLOAT3 normal = contact._Normal; // �Փ˖@���x�N�g��
	float restitution = contact._Restitution; // �����W��

	XMFLOAT3 vectAtoContact = contactPoint - pBodyA->GetOwner()->GetPosition(); // �ڐG�_�ւ̃x�N�g��
	XMFLOAT3 vectBtoContact = contactPoint - pBodyB->GetOwner()->GetPosition(); // �ڐG�_�ւ̃x�N�g��

	XMFLOAT3 velocityAtContactA = pBodyA->_Velocity + CrossProduct(pBodyA->_AngularVelocity, vectAtoContact); // �ڐG�_�̐��`���x�@�{�@��]�ɂ�鑬�x
	XMFLOAT3 velocityAtContactB = pBodyB->_Velocity + CrossProduct(pBodyB->_AngularVelocity, vectBtoContact); // �ڐG�_�̐��`���x�@�{�@��]�ɂ�鑬�x

	XMFLOAT3 relativeVelocity = velocityAtContactB - velocityAtContactA; // �ڐG�_�ł̑��Α��x�iA����݂�B�̑��x�j
	float	 relativeVelocityN = InnerProduct(relativeVelocity, normal); // ���Α��x�̖@����������

	if (relativeVelocityN >= 0.0f)
	{
		//printf("normal: %f, %f, %f\n", normal.x, normal.y, normal.z);
		//printf("veloci: %f, %f, %f\n\n", relativeVelocity.x, relativeVelocity.y, relativeVelocity.z);
		return; // ����Ă���̂Ŕ����s�v
	}
	
	float rotationEffectA = InnerProduct(CrossProduct(vectAtoContact, normal), CrossProduct(vectAtoContact, normal) * pBodyA->_InertiaInverse); 
	float rotationEffectB = InnerProduct(CrossProduct(vectBtoContact, normal), CrossProduct(vectBtoContact, normal) * pBodyB->_InertiaInverse); 

	float j = -(1.0f + restitution) * relativeVelocityN / (pBodyA->_MassInverse + pBodyB->_MassInverse + rotationEffectA + rotationEffectB); // �C���p���X�x�N�g���̋���
	XMFLOAT3 impulse = normal * j; // ���ۂɍ��̂ɉ������

	// ���x�X�V
	pBodyA->_Velocity		 = pBodyA->_Velocity - impulse * pBodyA->_MassInverse;
	pBodyA->_AngularVelocity = pBodyA->_AngularVelocity - CrossProduct(vectAtoContact, impulse).z * pBodyA->_InertiaInverse;
	pBodyB->_Velocity		 = pBodyB->_Velocity + impulse * pBodyB->_MassInverse;
	pBodyB->_AngularVelocity = pBodyB->_AngularVelocity + CrossProduct(vectBtoContact, impulse).z * pBodyB->_InertiaInverse;

}

void PhysicsWorld::CorrectPosition(const Contact& contact)
{
	// �e�v�f�����o��
	PhysicsBody* pBodyA = contact._pBodyA;
	PhysicsBody* pBodyB = contact._pBodyB;
	XMFLOAT3 normal = contact._Normal; // �@��
	float penetration = contact._Penetration; // �ѓ��[�x

	// �萔
	const float slop = 0.001f; // �߂荞�݋��e�l
	const float percent = 0.2f; // �ѓ��̉�����␳���邩

	// slop�ȉ��̔����Ȃ߂荞�݂͋��e���A����ȏ��␳�ΏۂƂ���
	float correctionDepth = std::max(penetration - slop, 0.0f);
	float totalInverseMass = pBodyA->_MassInverse + pBodyB->_MassInverse; // ���v�t����
	if (totalInverseMass == 0.0f) return; // �ǂ�����ÓI�Ȃ̂Ŗ���

	float correctionMagnitude = (correctionDepth / totalInverseMass) * percent;
	XMFLOAT3 correction = normal * correctionMagnitude;

	// �ʒu���t���ʔ�œ�����
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
  ���Y�^
  �Ȃ������܂������Ȃ��̂ň�x�߂�
// �����蔻�� -----
			if (pBpb->_BodyType == PhysicsBody::BodyType::Dynamic && pAcol->isOverlap(*pBcol, &normal, &penetration))
			{
				// �Փ˂����̂� Solva �֑���
				printf("����������");
				contact = ComputeContacts(pAcol, pBcol, normal, penetration); // Contact �̌v�Z
				m_ContactObjects.push_back(contact); // �i�[
			}
			else if (pApb->_BodyType == PhysicsBody::BodyType::Dynamic && pBcol->isOverlap(*pAcol, &normal, &penetration))
			{
				// �Փ˂����̂� Solva �֑���
				printf("����������");
				contact = ComputeContacts(pBcol, pAcol, normal, penetration); // Contact �̌v�Z
				m_ContactObjects.push_back(contact); // �i�[
			}

*/

/*
Contact PhysicsWorld::ComputeContacts(Collision2d* pAcoll, Collision2d* pBcoll, const XMFLOAT3& pNormal, const float& pPenetration)
{
	// �O�j�`�����̓I�ȃN���X�Ƀ_�E���L���X�g
	Box& boxA = static_cast<Box&>(*pAcoll);
	Box& boxB = static_cast<Box&>(*pBcoll);

	// �P�j�Q�ƌ`��(Reference)�Ɣ�Q�ƌ`��(Incident)������
	// �Փ˖@���ɍł��߂��������� box ���h�Q�ƌ`��h�ɂ���
	XMFLOAT3 normal = pNormal;
	float dotAx = fabsf(InnerProduct(boxA.GetVectorRight(), normal));
	float dotAy = fabsf(InnerProduct(boxA.GetVectorTop(), normal));
	float dotBx = fabsf(InnerProduct(boxB.GetVectorRight(), normal));
	float dotBy = fabsf(InnerProduct(boxB.GetVectorTop(), normal));

	bool AisRef = ((dotAx > dotBx) && (dotAx > dotBy))
				|| ((dotAy > dotBx) && (dotAy > dotBy));

	Box& refB = AisRef ? boxA : boxB; // �Q�ƌ`��{�b�N�X
	Box& incB = AisRef ? boxB : boxA; // ��Q�ƌ`��{�b�N�X
	XMFLOAT3 faceNormal = AisRef ? pNormal : XMFLOAT3{ -pNormal.x,-pNormal.y,-pNormal.z };

	// �Q�j��Q�ƃt�F�C�X�̂Q���_���
	std::vector<XMFLOAT3> poly;
	{
		// incB�F��Q�ƌ`��AfaceNormal�FboxA��boxB�����̖@���𔽓]���Ďg��
		XMFLOAT3 faceDir = faceNormal * -1;

		// incB �̒��S�A���[�J�����A�� extents
		XMFLOAT3 center = incB.GetPosition();
		XMFLOAT3 ux = incB.GetVectorRight();
		XMFLOAT3 uy = incB.GetVectorTop();
		float ex = incB.GetSize().x * 0.5f;
		float ey = incB.GetSize().y * 0.5f;

		float dotUX = fabsf(InnerProduct(ux, faceDir));
		float dotUY = fabsf(InnerProduct(uy, faceDir));


		// facedir �ɋ߂����𔻒肵�A�t�F�C�X�̖ʂ��t orth�i�G�b�W�����j��I��
		XMFLOAT3 faceAxis; // �t�F�C�X�ʂ̖@��
		float faceExtent; //
		XMFLOAT3 orthAxis; // �ʂɒ�������G�b�W�����̎�
		float orthExtent; // �G�b�W�̔����̒���
		if (dotUX > dotUY)
		{
			// faceDir �� ux �������@�ˁ@�t�F�C�X�͍��E�̖ʁ@�ˁ@�G�b�W���� uy
			faceAxis = ux * Sign(InnerProduct(ux, faceDir));
			faceExtent = ex;
			orthAxis = uy;
			orthExtent = ey;
		}
		else
		{
			// faceDir �� uy �������@�ˁ@�t�F�C�X�͏㉺�̖ʁ@�ˁ@�G�b�W���� ux
			faceAxis = uy * Sign(InnerProduct(uy, faceDir));
			faceExtent = ey;
			orthAxis = ux;
			orthExtent = ex;
		}
		// �Q���_�� poly �ɃZ�b�g
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


	// �R�j�Q�ƃt�F�C�X�̂Q�ӂ��`�i�t�F�C�X�@�� = refaxis �ɉ����ʂ�I�ԁj
	// refCorners �̂����A�@�������ōł������Q�_��I�т��̂Q�_�����ԃG�b�W���N���b�s���O�̊�ɂȂ�
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
	// ���ʂP�FsideAxis.x <= offset_1
	XMFLOAT3 normal_1 = sideAxis;
	float offset_1 = InnerProduct(normal_1, { center.x + sideAxis.x * sideExtent, center.y + sideAxis.y * sideExtent, 0 });

	// ���ʂQ�F-sideAxis.x <= offset_2
	XMFLOAT3 normal_2 = { -sideAxis.x, -sideAxis.y, 0 };
	float offset_2 = InnerProduct(normal_2, { center.x - sideAxis.x * sideExtent, center.y - sideAxis.y * sideExtent, 0 });

	// �S�j�N���b�v�����Fpoly �����ɕ��ʂP�����ʂQ�Ő؂�o��
	auto clip = [&](const XMFLOAT3& normal, float offset) {
		std::vector<XMFLOAT3> out;
		int N = (int)poly.size();
		for (int i = 0; i < N; i++)
		{
			auto P = poly[i];
			auto Q = poly[(i + 1) % N];
			float dP = InnerProduct(normal, P) - offset;
			float dQ = InnerProduct(normal, Q) - offset;
			//�i�P�jP �������Ȃ�c��
			if (dP <= 0) out.push_back(P);
			// �i�Q�j�ӂ���������Ȃ��_��ǉ�
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

	// �T�j���̑��� Contact ���ƍ��킹�ĕԋp
	PhysicsBody* bodyA =  pAcoll->GetOwner()->GetPhysicsBody();
	PhysicsBody* bodyB =  pBcoll->GetOwner()->GetPhysicsBody();

	// �U�j���C�E�����W��������
	float e = std::min(bodyA->_Restitution, bodyB->_Restitution);
	float frictionS = sqrt(bodyA->_StaticFriction * bodyB->_StaticFriction);
	float frictionD = sqrt(bodyA->_DynamicFriction * bodyB->_DynamicFriction);

	// �V�jContact ���\�z���ĕԋp
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

	// �e�v�f�����o��
	PhysicsBody* pBodyA = contact._pBodyA;
	PhysicsBody* pBodyB = contact._pBodyB;
	XMFLOAT3 contactNormal = contact._Normal;
	float restitution = iteration == 0 ? contact._Restitution : 0.0f; // �P��ڂ̃��[�v�̂Ƃ��̂ݓK�p������
	float staticFriction = contact._StaticFriction;
	float dynamicFriction = contact._DynamicFriction;

	// �P�j�ڐG�_�ʒu�ւ̊e�{�f�B���S����̃x�N�g��
	XMFLOAT3 vectToContactPointFromA = contactPoint - pBodyA->GetOwner()->GetPosition();
	XMFLOAT3 vectToContactPointFromB = contactPoint - pBodyB->GetOwner()->GetPosition();

	// �Q�j�ڐG�_�ł̓_���x
	XMFLOAT3 pointVelocityA = pBodyA->_Velocity + CrossProduct(pBodyA->_AngularVelocity, vectToContactPointFromA);
	XMFLOAT3 pointVelocityB = pBodyB->_Velocity + CrossProduct(pBodyB->_AngularVelocity, vectToContactPointFromB);

	// �R�j���Α��x�Ɩ@������ -----
	XMFLOAT3 relativeVelocity = pointVelocityB - pointVelocityA;
	float velocityAlongNormal = InnerProduct(relativeVelocity, contactNormal);
	//printf("vn (before impulse) = %.6f\n", velocityAlongNormal); // DEBUG
	if (velocityAlongNormal > 0) return; // ����Ă�������ɓ����Ă���̂ŏՓˉ����s�v

	// �S�j�@�������̃C���p���X�̑傫��
	float inverseMassSum = pBodyA->_MassInverse + pBodyB->_MassInverse; // �t���ʂ̘a
	float crossA = vectToContactPointFromA.x * contactNormal.y - vectToContactPointFromA.y * contactNormal.x; // �ڐG�_�@���ƐڐG�_���璆�S�ւ̃x�N�g���̊O��
	float crossB = vectToContactPointFromB.x * contactNormal.y - vectToContactPointFromB.y * contactNormal.x; // �ڐG�_�@���ƐڐG�_���璆�S�ւ̃x�N�g���̊O��
	float rotationalTermA = crossA * crossA * pBodyA->_InertiaInverse; // �O�ρ~�O�ρ~�t����
	float rotationalTermB = crossB * crossB * pBodyB->_InertiaInverse; // �O�ρ~�O�ρ~�t����
	float denominator = inverseMassSum + rotationalTermA + rotationalTermB; //
	float impulseMagnitude = -(1.0f + restitution) * velocityAlongNormal / denominator; //

	// �T�j�@���C���p���X�𑬓x�ɓK�p�i�݌v�����j
	float oldNormalImpulse = contact._NormalImpulseAccum[k];
	float newNormalImpulse = std::max(oldNormalImpulse + impulseMagnitude, 0.0f);// ���ɂȂ�Ȃ��悤�ɃN�����v
	float dentaNormalImpulse = newNormalImpulse - oldNormalImpulse; // ����
	contact._NormalImpulseAccum[k] = newNormalImpulse; // �X�V
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
	

	// �U�j���C�����P�ʃx�N�g�� -----
XMFLOAT3 tangent = relativeVelocity - (contactNormal * velocityAlongNormal);
if (LengthFloat3(tangent) > 0) tangent = Normalize(tangent);
else return; // ���x���s�b�^���@�������Ȃ疀�C�s�v

// �V�j���C�C���p���X�傫���i����͌����ɋ��߂�B�Q�[���G���W�����x�Ȃ�@���C���p���X��denom�𗬗p���Ă����Ȃ��炵���j
float velocityAlongTangent = InnerProduct(relativeVelocity, tangent);
float tangentCrossA = vectToContactPointFromA.x * tangent.y - vectToContactPointFromA.y * tangent.x;
float tangentCrossB = vectToContactPointFromB.x * tangent.y - vectToContactPointFromB.y * tangent.x;
float rotationalTangentA = tangentCrossA * tangentCrossA * pBodyA->_InertiaInverse;
float rotationalTangentB = tangentCrossB * tangentCrossB * pBodyB->_InertiaInverse;
float frictionDenominator = inverseMassSum + rotationalTangentA + rotationalTangentB; // �ڐ������Ɍ����L�����ʁi�t���ʘa�{��]���j
float frictionMagnitude = -velocityAlongTangent / frictionDenominator; // 

// �W�j�N���[�����C�̃N�����v
float maxStaticFriction = contact._StaticFriction * newNormalImpulse;
float maxDynamicFriction = contact._DynamicFriction * newNormalImpulse;
float oldTangentImpulse = contact._TangentImpulseAccum[k];
float newTangentImpulse = oldTangentImpulse + frictionMagnitude;
if (abs(oldTangentImpulse + frictionMagnitude) < maxStaticFriction)
{
	// �Î~���C�͗̈�
	newTangentImpulse = oldTangentImpulse + frictionMagnitude;
}
else
{
	// �����C�͗̈�
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

// �X�j���C�C���p���X�K�p
pBodyA->_Velocity        = pBodyA->_Velocity        - frictionImpulse * pBodyA->_MassInverse;
pBodyA->_AngularVelocity = pBodyA->_AngularVelocity - pBodyA->_InertiaInverse * (vectToContactPointFromA.x * frictionImpulse.y - vectToContactPointFromA.y * frictionImpulse.x);
pBodyB->_Velocity        = pBodyB->_Velocity		+ frictionImpulse * pBodyB->_MassInverse;
pBodyB->_AngularVelocity = pBodyB->_AngularVelocity + pBodyB->_InertiaInverse * (vectToContactPointFromB.x * frictionImpulse.y - vectToContactPointFromB.y * frictionImpulse.x);
*/

