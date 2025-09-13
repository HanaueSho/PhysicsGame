/*
	collision.cpp
	hanaue sho 20241119
*/
#include <algorithm>
#include <vector>
#include "Collision.h"
#include "Transform.h"
#include "Matrix4x4.h"
#include "Quaternion.h"
#include "ContactManifold.h"

// ==================================================
// ヘルパ関数
// ==================================================
namespace
{
	// ワールドOBB
	struct OBBW
	{
		Vector3 center;  // center
		Vector3 axis[3]; // world acis
		Vector3 extent;	 // world scale 適用後
	};
	// ワールドOBB の構築
	OBBW MakeOBB(const Transform& tr, const BoxCollision& box)
	{
		OBBW o;
		o.center = tr.position;
		o.axis[0] = tr.GetRight().normalized();
		o.axis[1] = tr.GetUp().normalized();
		o.axis[2] = tr.GetForward().normalized();
		o.extent = { fabsf(tr.scale.x) * box.HalfSize().x,
					 fabsf(tr.scale.y) * box.HalfSize().y,
					 fabsf(tr.scale.z) * box.HalfSize().z };
		return o;
	}

	float AbsDot(const Vector3& a, const Vector3& b) { return fabsf(Vector3::Dot(a, b)); }
	// 最大要素のインデックスを返す
	int ArgMax3(float a, float b, float c) { return (a > b ? (a > c ? 0 : 2) : (b > c ? 1 : 2)); } // {max, return} = {a, 0}, {b, 1}, {c, 2} 

	// 点群を平面でクリップ（Sutherland-Hodgman）
	std::vector<Vector3> ClipPolygonAgainstPlane(const std::vector<Vector3>& poly, const Vector3& n, float d)
	{
		std::vector<Vector3> out;
		if (poly.empty()) return out;
		const int N = (int)poly.size();
		for (int i = 0; i < N; i++)
		{
			const Vector3& A = poly[i]; // 基準頂点（始点）
			const Vector3& B = poly[(i + 1) % N]; // 隣の頂点 （終点）
			const float da = Vector3::Dot(n, A) - d;
			const float db = Vector3::Dot(n, B) - d;
			const bool ina = (da <= 0.0f); // 今回はn方向を“外側”としている
			const bool inb = (db <= 0.0f);
			if (ina && inb)
				out.push_back(B);
			else if (ina && !inb)
			{
				float t = da / (da - db + 1e-20f);
				out.push_back(A + (B - A) * t);
			}
			else if (!ina && inb)
			{
				float t = da / (da - db + 1e-20f);
				out.push_back(A + (B - A) * t);
				out.push_back(B);
			}
		}
		return out;
	}

	// SAT -----
	enum class AxisKind { FaceA, FaceB, EdgeEdge};
	struct AxisChoice 
	{ 
		AxisKind kind;
		int ia; // faceA:0..2 / Edge : A側軸
		int ib; // faceB:0..2 / Edge : B側軸
		float depth; // 最小貫入量
		Vector3 n; // A → B
	}; 
	float SupportRadius(const OBBW& O, const Vector3& n)
	{
		return O.extent.x * AbsDot(O.axis[0], n) + O.extent.y * AbsDot(O.axis[1], n) + O.extent.z * AbsDot(O.axis[2], n);
	}
	bool SAT_AllAxes(const OBBW& A, const OBBW& B, AxisChoice& out, float epsLen2 = 1e-12f)
	{
		const Vector3 vectAtoB = B.center - A.center;
		float best = 1e9f; // 貫入深度
		AxisKind bestKind = AxisKind::FaceA; // 種類
		int bestI = 0, bestJ = 0;  // 
		Vector3 bestN = A.axis[0]; // 

		auto testAxis = [&](const Vector3& nRaw, AxisKind kind, int ia, int ib) -> bool
			{
				float nLen2 = nRaw.lengthSq();
				if (nLen2 < epsLen2) return true; // 無効軸（ほぼ平行）
				Vector3 n = nRaw / sqrtf(nLen2); // 投影軸（分離軸）（正規化）
				float dist = fabsf(Vector3::Dot(vectAtoB, n)); // 投影
				float rA = SupportRadius(A, n); // 半長の投影
				float rB = SupportRadius(B, n); // 半長の投影
				float overlap = rA + rB - dist; // 軸上で比較
				if (overlap < 0.0f) return false; // 分離
				if (overlap < best) { best = overlap; bestKind = kind; bestI = ia; bestJ = ib; bestN = n; }
				return true; // 触れている
			};

		// Face Axis (A, B)
		for (int i = 0; i < 3; i++) if (!testAxis(A.axis[i], AxisKind::FaceA, i, -1)) return false;
		for (int j = 0; j < 3; j++) if (!testAxis(B.axis[j], AxisKind::FaceB, -1, j)) return false;

		// Edge-Edge Axis
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
			{
				Vector3 c = Vector3::Cross(A.axis[i], B.axis[j]);
				if (!testAxis(c, AxisKind::EdgeEdge, i, j)) return false;
			}

		// 法線は A → B に向ける
		if (Vector3::Dot(bestN, vectAtoB) < 0) bestN = -bestN; 

		// AxisChoice の生成
		out.kind = bestKind;
		out.ia = bestI;
		out.ib = bestJ;
		out.depth = best;
		out.n = bestN;

		return true; // 接触
	}

	// ref OBB の refFace を選ぶ
	void BuildRefFaceQuad(const OBBW& R, int refAxis, std::vector<Vector3>& outQuad) 
	{
		static const int others[3][2] = { {1,2}, {0,2}, {0,1} };
		const int u = others[refAxis][0];
		const int v = others[refAxis][1];
		Vector3 n = R.axis[refAxis];
		Vector3 Ru = R.axis[u];
		Vector3 Rv = R.axis[v];
		float eu = R.extent[u];
		float ev = R.extent[v];
		Vector3 center = R.center + n * R.extent[refAxis]; // 表側面
		outQuad.clear();
		outQuad.push_back(center + Ru*eu + Rv*ev); // 右上
		outQuad.push_back(center - Ru*eu + Rv*ev); // 左上
		outQuad.push_back(center - Ru*eu - Rv*ev); // 左下
		outQuad.push_back(center + Ru*eu - Rv*ev); // 右下
	}

	// incidence face : normal に最も反平行な B の面（index を返す）
	int PickIncidenceFace(const OBBW& I, const Vector3& n) 
	{
		float d0 = Vector3::Dot(I.axis[0], n);
		float d1 = Vector3::Dot(I.axis[1], n);
		float d2 = Vector3::Dot(I.axis[2], n);
		return ArgMax3(fabsf(d0), fabsf(d1), fabsf(d2));
	}

	// ２本の線分（中心±dir*Len）最近接点
	void ClosestPointSegmentSegment(
		const Vector3& P0, const Vector3& u, float lu,
		const Vector3& Q0, const Vector3& v, float lv,
		Vector3& outP, Vector3& outQ) 
	{
		Vector3 w0 = P0 - Q0;
		float a = Vector3::Dot(u, u);
		float b = Vector3::Dot(u, v);
		float c = Vector3::Dot(v, v);
		float d = Vector3::Dot(u, w0);
		float e = Vector3::Dot(v, w0);
		float denom = a*c - b*b;
		float s, t;
		if (denom < 1e-12f) // ほぼ平行
		{ 
			s = 0.0f; 
			t = (c > 1e-12f) ? (e / c) : 0.0f; 
		}
		else 
		{ 
			s = (b*e - c*d)/denom; 
			t = (a*e - b*d)/denom; 
		}

		// clamp
		if (s > lu) s = lu; else if (s < -lu) s = -lu;
		if (t > lv) t = lv; else if (t < -lv) t = -lv;
		outP = P0 + u*s;
		outQ = Q0 + v*t;
	}
}



// ==================================================
// BoxCollision
// ==================================================
AABB BoxCollision::ComputeWorldAABB(const Transform& trans) const
{
	Vector3 center = trans.position;
	Matrix4x4 rotate = trans.rotation.ToMatrix();
	Matrix4x4 scale = Matrix4x4::CreateScale(trans.scale.x, trans.scale.y, trans.scale.z);
	Matrix4x4 mat = scale * rotate; // 回転とスケールを持った行列
	Vector3 e; // 各要素の列の絶対値と m_HalfSize の内積
	e.x = fabsf(mat.m[0][0]) * m_HalfSize.x + fabsf(mat.m[1][0]) * m_HalfSize.y + fabsf(mat.m[2][0]) * m_HalfSize.z;
	e.y = fabsf(mat.m[0][1]) * m_HalfSize.x + fabsf(mat.m[1][1]) * m_HalfSize.y + fabsf(mat.m[2][1]) * m_HalfSize.z;
	e.z = fabsf(mat.m[0][2]) * m_HalfSize.x + fabsf(mat.m[1][2]) * m_HalfSize.y + fabsf(mat.m[2][2]) * m_HalfSize.z;

	return { center - e, center + e }; // min: center - e, max: center + e
}

bool BoxCollision::isOverlap(const Transform& myTrans, const Collision& collisionB, const Transform& transB, ContactManifold& out, float slop) const
{
	return collisionB.isOverlapWithBox(transB, *this, myTrans, out, slop);
}

// --------------------------------------------------
// BOX × BOX （SAT方式）
// --------------------------------------------------
bool BoxCollision::isOverlapWithBox(const Transform& myTrans, const BoxCollision& box, const Transform& transBox, ContactManifold& out, float slop) const
{
	// ----- OBB 構築 -----　（ダブルディスパッチの弊害でここはAとB逆に生成する）
	OBBW B = MakeOBB(myTrans, *this);
	OBBW A = MakeOBB(transBox, box);

	// ----- SAT（最小貫入(depth)とその勝ち軸を得る） -----
	AxisChoice ch;
	if (!SAT_AllAxes(A, B, ch)) { out.touching = false; out.count = 0; return false; } // 当たっていなかったら終わり

	// ------ スロープを考慮して接触を判定 ------
	// depth >= -slop なら接触扱い
	const bool touching = (ch.depth >= -slop);
	out.touching = touching;
	if (!touching) { out.count = 0; return false; }

	// 接触法線（SAT で選ばれた軸）
	out.normal = ch.n; // A → B

	// ----- manifold の構築 -----
	out.count = 0;

	if (ch.kind == AxisKind::FaceA || ch.kind == AxisKind::FaceB)
	{
		// face-face
		const bool refIsA = (ch.kind == AxisKind::FaceA); // ReferenceBox の判定
		const OBBW& Ref = refIsA ? A : B; // ReferenceBox の設定
		const OBBW& Inc = refIsA ? B : A; // IncidenceBox の設定
		const int refAxis = refIsA ? ch.ia : ch.ib;

		// 参照面法線は「参照→相手」へ向ける必要がある
		// out.normal は A→B。参照がBのときは反転して B→A にする
		//const Vector3 refN = refIsA ? out.normal : -out.normal;
		const Vector3 refN = refIsA ? out.normal : -out.normal; 

		// 参照面の四角形（今は使ってないので放置）（デバッグ描画に使えるみたい）
		//std::vector<Vector3> refQuad;
		//BuildRefFaceQuad(Ref, refAxis, refQuad);

		// ----- 入射OBB側から、参照法線に最も反平行な面を選ぶ（その４頂点）-----
		const int incFace = PickIncidenceFace(Inc, refN); // incFace の候補
		const int sign = (Vector3::Dot(Inc.axis[incFace], refN) > 0.0f) ? -1.0f : 1.0f; // refNに反平行になるような符号

		// 入射面の４済を作って多角形にする
		std::vector<Vector3> poly; poly.reserve(4);
		{
			// Incidence face の４隅を作る
			const int u = (incFace == 0 ? 1 : 0); // 入射面の２本のエッジ方向（incFace 軸以外の２軸）
			const int v = (incFace == 2 ? 1 : 2);
			
			// n と反平行な「裏側の面」の平面上の中心点 
			const Vector3 c  = Inc.center + Inc.axis[incFace] * Inc.extent[incFace] * sign; // 反対側面（n に反平行）
			const Vector3 U  = Inc.axis[u], V = Inc.axis[v];
			const float   eu = Inc.extent[u], ev = Inc.extent[v];
			poly.push_back(c + U * eu + V * ev); // 右上
			poly.push_back(c - U * eu + V * ev); // 左上
			poly.push_back(c - U * eu - V * ev); // 左下
			poly.push_back(c + U * eu - V * ev); // 右下
		}

		// ----- 参照面の４つの“側面平面”で入射面ポリゴンをクリップ -----
		std::vector<Vector3> poly1 = poly;
		auto clipAgainstSidePlane = [&](const Vector3& pn, float pd) // pn : 平面の法線ベクトル, pd : 原点からのオフセット（平面上の任意の点）
			{
				poly1 = ClipPolygonAgainstPlane(poly1, pn, pd);
			};

		// 参照面上の中心点（表側）
		const int u = (refAxis == 0 ? 1 : 0); // refAxis 以外の２軸
		const int v = (refAxis == 2 ? 1 : 2);
		const Vector3 p0 = Ref.center + refN * Ref.extent[refAxis]; // 参照面上の中心点

		// 左右								// 側面がワールド上のどこにあるか求めている
		clipAgainstSidePlane( Ref.axis[u],  Vector3::Dot(Ref.axis[u], p0) + Ref.extent[u]);
		clipAgainstSidePlane(-Ref.axis[u], -Vector3::Dot(Ref.axis[u], p0) + Ref.extent[u]);
		// 上下
		clipAgainstSidePlane( Ref.axis[v],  Vector3::Dot(Ref.axis[v], p0) + Ref.extent[v]);
		clipAgainstSidePlane(-Ref.axis[v], -Vector3::Dot(Ref.axis[v], p0) + Ref.extent[v]);

		// 残った頂点（接触点候補）を manifold へ
		for (const Vector3& q : poly1)
		{
			if (out.count >= ContactManifold::MAX_POINTS) break; // 上限４つまで（現在は速いもの順）

			// 参照面への有向距離：dot(refN, q - p0)（表側が＋）
			// penetration は参照面への signed 距離の -値を使う（>=0 が重なり）
			const float signedDist = Vector3::Dot(q - p0, refN); // 参照面表側が＋なので <= 0 ならめり込んでいる
			const float pen = -signedDist;					// pen > 0 ならめり込んでいる
			if (pen < -slop) continue; // slop よりも小さい = めり込んでいない

			// A/B それぞれの“自分の面上”の点を作る
			if (ch.kind == AxisKind::FaceA)
			{
				out.points[out.count].pointOnB = q;
				out.points[out.count].pointOnA = q - refN * signedDist; // A 参照面上
			}
			else
			{
				out.points[out.count].pointOnA = q;
				out.points[out.count].pointOnB = q + refN * signedDist; // B 参照面上 
			}
			out.points[out.count].penetration = pen;
			out.count++;
		}

		// クリップで全滅した場合の保険（数値誤差対策）
		if (out.count == 0)
		{
			out.count = 1;
			out.points[0].pointOnA = p0;
			out.points[0].pointOnB = p0;
			out.points[0].penetration = std::max(0.0f, ch.depth);
		}
	} // if (ch.kind == AxisKind::FaceA || ch.kind == AxisKind::FaceB)
	else
	{
		// Edge-Edge（厳密ではなく軽めのもの）
		const int ia = ch.ia, ib = ch.ib;
		// 代表エッジの中心（各OBBの該当軸以外の投影は 0 の中心線でOK）
		const Vector3 Pa0 = A.center; // 代表線分中心
		const Vector3 Qa0 = B.center;
		const Vector3 ua = A.axis[ia]; const float la = A.extent[ia]; // 方向＆半長
		const Vector3 ub = B.axis[ib]; const float lb = B.extent[ib];

		Vector3 pA, pB;
		ClosestPointSegmentSegment(Pa0, ua, la, Qa0, ub, lb, pA, pB);

		out.count = 1;
		out.points[0].pointOnA = pA;
		out.points[0].pointOnB = pB;
		out.points[0].penetration = ch.depth; // SAT の最小貫入をそのままつかう
	}

	return true; // 全ての判定を通ったので true
}

// --------------------------------------------------
// BOX × SPHERE （最近接点方式）
// --------------------------------------------------
bool BoxCollision::isOverlapWithSphere(const Transform& myTrans, const SphereCollision& sphere, const Transform& transSph, ContactManifold& out, float slop) const
{
	// ----- OBBの世界軸を取得 -----
	const Vector3 aX = myTrans.GetRight();
	const Vector3 aY = myTrans.GetUp();
	const Vector3 aZ = myTrans.GetForward();

	// ----- 半エクステント（スケール反映） -----
	Vector3 extent = {
		fabsf(myTrans.scale.x) * m_HalfSize.x,
		fabsf(myTrans.scale.y) * m_HalfSize.y,
		fabsf(myTrans.scale.z) * m_HalfSize.z,
	};

	// ----- 球の中心と半径取得 -----
	const Vector3 centerSph = transSph.position;
	const Vector3 centerBox = myTrans.position;
	const float   radius = sphere.Radius() * fabsf(transSph.scale.x); // 一旦 scale.x でスケーリング

	// ----- 箱中心→球中心へのベクトルを箱の軸に投影 -----
	const Vector3 vect = centerSph - centerBox;
	const Vector3 dist = {
		Vector3::Dot(vect, aX),
		Vector3::Dot(vect, aY),
		Vector3::Dot(vect, aZ)
	};

	// ----- 最近接点のローカル座標（Clamp）-----
	auto Clamp = [](float a, float min, float max)
		{
			float result = a;
			result = std::min(a, max);
			result = std::max(result, min);
			return result;
		};
	float qx = Clamp(dist.x, -extent.x, extent.x);
	float qy = Clamp(dist.y, -extent.y, extent.y);
	float qz = Clamp(dist.z, -extent.z, extent.z);

	// ----- ワールド最近接点 -----
	Vector3 qWorld = centerBox + aX * qx + aY * qy + aZ * qz; // 接触点の計算

	// ----- 球中心　→　最近接点ベクトル ------
	const Vector3 vectSphToPoint = centerSph - qWorld;
	const float distSphToPointSq = vectSphToPoint.lengthSq(); 

	// 球が箱の内部にある特殊ケース対策
	Vector3 n;
	float pen;
	if (distSphToPointSq > 1e-12f) // 球が箱の外にいる
	{
		const float dist = sqrtf(distSphToPointSq);
		n = vectSphToPoint / dist; // 接触点→球　方向（Box → Sphere）
		pen = radius - dist; // 正なら重なっている
	}
	else // 球が箱の内部
	{
		// 最近平面を選ぶ
		const float dx = extent.x - fabsf(dist.x);
		const float dy = extent.y - fabsf(dist.y);
		const float dz = extent.z - fabsf(dist.z);
		if		(dx <= dy && dx <= dz) { n = (dist.x >= 0 ? aX : -aX); qx = (dist.x >= 0 ? extent.x : -extent.x); } // 一番近くの面の方向を normal にする
		else if (dy <= dz)			   { n = (dist.y >= 0 ? aY : -aY); qy = (dist.y >= 0 ? extent.y : -extent.y); }
		else						   { n = (dist.z >= 0 ? aZ : -aZ); qz = (dist.z >= 0 ? extent.z : -extent.z); }

		qWorld = centerBox + aX * qx + aY * qy + aZ * qz; // 接触点の再計算
		pen = radius + std::min(dx, std::min(dy, dz)); // 正で重なっている
	}
	// 接触しているかの判定
	const bool touching = (pen >= -slop);

	// ----- ContactManifold の出力 -----
	out.touching = touching;
	out.normal = -n; // Sphere → Box
	out.count = 1;
	out.points[0].penetration = pen;
	out.points[0].pointOnA = centerSph + out.normal * radius;
	out.points[0].pointOnB = qWorld;

	return touching;
}

bool BoxCollision::isOverlapWithCapsule(const Transform& myTrans, const CapsuleCollision& capsule, const Transform& transCap, ContactManifold& out, float slop) const
{
	return false;
}


// ==================================================
// SphereCollision
// ==================================================
AABB SphereCollision::ComputeWorldAABB(const Transform& trans) const
{
	Vector3 center = trans.position;
	float scaleMax = std::max({ fabsf(trans.scale.x), fabsf(trans.scale.y), fabsf(trans.scale.z) });
	float radius   = m_Radius * scaleMax;
	Vector3 d{ radius, radius, radius };
	return { center - d, center + d }; // min: center - radius, max: center + radius
}

bool SphereCollision::isOverlap(const Transform& myTrans, const Collision& collisionB, const Transform& transB, ContactManifold& out, float slop) const
{
	return collisionB.isOverlapWithSphere(transB, *this, myTrans, out, slop);
}

bool SphereCollision::isOverlapWithBox(const Transform& myTrans, const BoxCollision& box, const Transform& transBox, ContactManifold& out, float slop) const
{
	// ----- OBBの世界軸を取得 -----
	const Vector3 aX = transBox.GetRight();
	const Vector3 aY = transBox.GetUp();
	const Vector3 aZ = transBox.GetForward();

	// ----- 半エクステント（スケール反映） -----
	Vector3 extent = {
		fabsf(transBox.scale.x) * box.HalfSize().x,
		fabsf(transBox.scale.y) * box.HalfSize().y,
		fabsf(transBox.scale.z) * box.HalfSize().z,
	};

	// ----- 球の中心と半径取得 -----
	const Vector3 centerSph = myTrans.position;
	const Vector3 centerBox = transBox.position;
	const float   radius = this->Radius() * fabsf(myTrans.scale.x); // 一旦 scale.x でスケーリング

	// ----- 箱中心→球中心へのベクトルを箱の軸に投影 -----
	const Vector3 vect = centerSph - centerBox;
	const Vector3 dist = {
		Vector3::Dot(vect, aX),
		Vector3::Dot(vect, aY),
		Vector3::Dot(vect, aZ)
	};

	// ----- 最近接点のローカル座標（Clamp）-----
	auto Clamp = [](float a, float min, float max)
		{
			float result = a;
			result = std::min(a, max);
			result = std::max(result, min);
			return result;
		};
	float qx = Clamp(dist.x, -extent.x, extent.x);
	float qy = Clamp(dist.y, -extent.y, extent.y);
	float qz = Clamp(dist.z, -extent.z, extent.z);

	// ----- ワールド最近接点 -----
	Vector3 qWorld = centerBox + aX * qx + aY * qy + aZ * qz; // 接触点の計算

	// ----- 球中心　→　最近接点ベクトル ------
	const Vector3 vectSphToPoint = centerSph - qWorld;
	const float distSphToPointSq = vectSphToPoint.lengthSq();

	// 球が箱の内部にある特殊ケース対策
	Vector3 n;
	float pen;
	if (distSphToPointSq > 1e-12f) // 球が箱の外にいる
	{
		const float dist = sqrtf(distSphToPointSq);
		n = vectSphToPoint / dist; // 接触点→球　方向（Box → Sphere）
		pen = radius - dist; // 正なら重なっている
	}
	else // 球が箱の内部
	{
		// 最近平面を選ぶ
		const float dx = extent.x - fabsf(dist.x);
		const float dy = extent.y - fabsf(dist.y);
		const float dz = extent.z - fabsf(dist.z);
		if (dx <= dy && dx <= dz) { n = (dist.x >= 0 ? aX : -aX); qx = (dist.x >= 0 ? extent.x : -extent.x); } // 一番近くの面の方向を normal にする
		else if (dy <= dz) { n = (dist.y >= 0 ? aY : -aY); qy = (dist.y >= 0 ? extent.y : -extent.y); }
		else { n = (dist.z >= 0 ? aZ : -aZ); qz = (dist.z >= 0 ? extent.z : -extent.z); }

		qWorld = centerBox + aX * qx + aY * qy + aZ * qz; // 接触点の再計算
		pen = radius + std::min(dx, std::min(dy, dz)); // 正で重なっている
	}
	// 接触しているかの判定
	const bool touching = (pen >= -slop);

	// ----- ContactManifold の出力 -----
	out.touching = touching;
	out.normal = n; // Sphere → Box
	out.count = 1;
	out.points[0].penetration = pen;
	out.points[0].pointOnA = centerSph + out.normal * radius;
	out.points[0].pointOnB = qWorld;

	return touching;
}

// --------------------------------------------------
// Sphere × Sphere
// --------------------------------------------------
bool SphereCollision::isOverlapWithSphere(const Transform& myTrans, const SphereCollision& sphere, const Transform& transSph, ContactManifold& out, float slop) const
{
	// 中心
	Vector3 centerB = myTrans.position;
	Vector3 centerA = transSph.position;

	// 距離（２乗）
	Vector3 vect = centerB - centerA; // A → B
	float distSq = vect.lengthSq();

	// 半径（スケール反映）
	float radiusA = m_Radius * myTrans.scale.x;			// とりあえずｘの値を使う
	float radiusB = sphere.Radius() * transSph.scale.x; // とりあえずｘの値を使う
	float sum = radiusA + radiusB;

	// 接触判定
	if (distSq > 1e-12f) // ２点間の距離が0以上か確認
	{
		const float d = sqrtf(distSq);
		out.normal = vect / d; // A → B
		out.points[0].penetration = sum - d; // 貫入深度計算
	}
	else
	{
		out.normal = { 1, 0, 0 }; // 退避向き
		out.points[0].penetration = sum;
	}
	out.touching = (out.points[0].penetration >= -slop); // 貫入深度が正 → sum > d → 接触している

	out.points[0].pointOnA = centerA + out.normal * radiusA;
	out.points[0].pointOnB = centerB + out.normal * radiusB;
	out.count = 1;

	return out.touching;
}

bool SphereCollision::isOverlapWithCapsule(const Transform& myTrans, const CapsuleCollision& capsule, const Transform& transCap, ContactManifold& out, float slop) const
{
	return false;
}


// ==================================================
// CapsuleCollision
// ==================================================
AABB CapsuleCollision::ComputeWorldAABB(const Transform& trans) const
{
	return AABB();
}

bool CapsuleCollision::isOverlap(const Transform& myTrans, const Collision& collisionB, const Transform& transB, ContactManifold& out, float slop) const
{
	return collisionB.isOverlapWithCapsule(transB, *this, myTrans, out, slop);
}

bool CapsuleCollision::isOverlapWithBox(const Transform& myTrans, const BoxCollision& box, const Transform& transBox, ContactManifold& out, float slop) const
{
	return false;
}

bool CapsuleCollision::isOverlapWithSphere(const Transform& myTrans, const SphereCollision& sphere, const Transform& transSph, ContactManifold& out, float slop) const
{
	return false;
}

bool CapsuleCollision::isOverlapWithCapsule(const Transform& myTrans, const CapsuleCollision& capsule, const Transform& transCap, ContactManifold& out, float slop) const
{
	return false;
}




// 備忘録
/*
// --------------------------------------------------
// BOX × BOX （SAT方式）
// --------------------------------------------------
bool BoxCollision::isOverlapWithBox(const Transform& myTrans, const BoxCollision& box, const Transform& transBox, ContactManifold& out, float slop) const
{
	// 数値安定用
	constexpr float EPS_LEN2 = 1e-10f; // 軸の長さ＾２がこれ未満なら平行とみなしスキップ
	constexpr float MARGIN	 = 1e-5f;	// 比較の緩衝

	// ----- BoxA, B の軸を１度だけ取得しておく -----
	const Vector3 aX = myTrans.GetRight();
	const Vector3 aY = myTrans.GetUp();
	const Vector3 aZ = myTrans.GetForward();
	const Vector3 bX = transBox.GetRight();
	const Vector3 bY = transBox.GetUp();
	const Vector3 bZ = transBox.GetForward();

	// ----- 軸を集める -----
	Vector3 listAxis[15];
	// BoxAの３軸
	listAxis[0] = aX;
	listAxis[1] = aY;
	listAxis[2] = aZ;
	// BoxBの３軸
	listAxis[3] = bX;
	listAxis[4] = bY;
	listAxis[5] = bZ;
	// 上の軸のクロス９軸
	int index = 6; // ６から入れるので
	for (int ia = 0; ia < 3; ia++)
	{
		for (int ib = 3; ib < 6; ib++)
		{
			Vector3 cross = Vector3::Cross(listAxis[ia], listAxis[ib]);
			if (cross.lengthSq() < EPS_LEN2) continue; // 外積が０に限りなく近い　→　ほぼ平行　→　分離軸として無効
			cross.normalize(); // 正規化
			listAxis[index++] = cross; // 正規化後に格納
		}
	}

	// ----- 各軸の判定（SAT） -----
	// Box 間のベクトル
	Vector3 vect = transBox.position - myTrans.position;
	// 各 Box の半エクステント（スケール適用）
	Vector3 extentA = {
		fabsf(myTrans.scale.x) * m_HalfSize.x,
		fabsf(myTrans.scale.y) * m_HalfSize.y,
		fabsf(myTrans.scale.z) * m_HalfSize.z,
	};
	Vector3 extentB = {
		fabsf(transBox.scale.x) * box.HalfSize().x,
		fabsf(transBox.scale.y) * box.HalfSize().y,
		fabsf(transBox.scale.z) * box.HalfSize().z,
	};

	// 各軸のチェック
	for (int i = 0; i < index; i++)
	{
		// 中心間距離を軸上に投影
		float dist = fabsf(Vector3::Dot(vect, listAxis[i]));

		// 投影半径
		float rA = extentA.x * fabsf(Vector3::Dot(aX, listAxis[i]))
				 + extentA.y * fabsf(Vector3::Dot(aY, listAxis[i]))
				 + extentA.z * fabsf(Vector3::Dot(aZ, listAxis[i]));
		float rB = extentB.x * fabsf(Vector3::Dot(bX, listAxis[i]))
				 + extentB.y * fabsf(Vector3::Dot(bY, listAxis[i]))
				 + extentB.z * fabsf(Vector3::Dot(bZ, listAxis[i]));

		if (dist > rA + rB + MARGIN) return false; // 分離しているので false
	}

	return true; // 全ての判定を通ったので true
}
*/

