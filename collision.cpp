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
// �w���p�֐�
// ==================================================
namespace
{
	// ���[���hOBB
	struct OBBW
	{
		Vector3 center;  // center
		Vector3 axis[3]; // world acis
		Vector3 extent;	 // world scale �K�p��
	};
	// ���[���hOBB �̍\�z
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
	// �ő�v�f�̃C���f�b�N�X��Ԃ�
	int ArgMax3(float a, float b, float c) { return (a > b ? (a > c ? 0 : 2) : (b > c ? 1 : 2)); } // {max, return} = {a, 0}, {b, 1}, {c, 2} 

	// �_�Q�𕽖ʂŃN���b�v�iSutherland-Hodgman�j
	std::vector<Vector3> ClipPolygonAgainstPlane(const std::vector<Vector3>& poly, const Vector3& n, float d)
	{
		std::vector<Vector3> out;
		if (poly.empty()) return out;
		const int N = (int)poly.size();
		for (int i = 0; i < N; i++)
		{
			const Vector3& A = poly[i]; // ����_�i�n�_�j
			const Vector3& B = poly[(i + 1) % N]; // �ׂ̒��_ �i�I�_�j
			const float da = Vector3::Dot(n, A) - d;
			const float db = Vector3::Dot(n, B) - d;
			const bool ina = (da <= 0.0f); // �����n�������g�O���h�Ƃ��Ă���
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
		int ia; // faceA:0..2 / Edge : A����
		int ib; // faceB:0..2 / Edge : B����
		float depth; // �ŏ��ѓ���
		Vector3 n; // A �� B
	}; 
	float SupportRadius(const OBBW& O, const Vector3& n)
	{
		return O.extent.x * AbsDot(O.axis[0], n) + O.extent.y * AbsDot(O.axis[1], n) + O.extent.z * AbsDot(O.axis[2], n);
	}
	bool SAT_AllAxes(const OBBW& A, const OBBW& B, AxisChoice& out, float epsLen2 = 1e-12f)
	{
		const Vector3 vectAtoB = B.center - A.center;
		float best = 1e9f; // �ѓ��[�x
		AxisKind bestKind = AxisKind::FaceA; // ���
		int bestI = 0, bestJ = 0;  // 
		Vector3 bestN = A.axis[0]; // 

		auto testAxis = [&](const Vector3& nRaw, AxisKind kind, int ia, int ib) -> bool
			{
				float nLen2 = nRaw.lengthSq();
				if (nLen2 < epsLen2) return true; // �������i�قڕ��s�j
				Vector3 n = nRaw / sqrtf(nLen2); // ���e���i�������j�i���K���j
				float dist = fabsf(Vector3::Dot(vectAtoB, n)); // ���e
				float rA = SupportRadius(A, n); // �����̓��e
				float rB = SupportRadius(B, n); // �����̓��e
				float overlap = rA + rB - dist; // ����Ŕ�r
				if (overlap < 0.0f) return false; // ����
				if (overlap < best) { best = overlap; bestKind = kind; bestI = ia; bestJ = ib; bestN = n; }
				return true; // �G��Ă���
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

		// �@���� A �� B �Ɍ�����
		if (Vector3::Dot(bestN, vectAtoB) < 0) bestN = -bestN; 

		// AxisChoice �̐���
		out.kind = bestKind;
		out.ia = bestI;
		out.ib = bestJ;
		out.depth = best;
		out.n = bestN;

		return true; // �ڐG
	}

	// ref OBB �� refFace ��I��
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
		Vector3 center = R.center + n * R.extent[refAxis]; // �\����
		outQuad.clear();
		outQuad.push_back(center + Ru*eu + Rv*ev); // �E��
		outQuad.push_back(center - Ru*eu + Rv*ev); // ����
		outQuad.push_back(center - Ru*eu - Rv*ev); // ����
		outQuad.push_back(center + Ru*eu - Rv*ev); // �E��
	}

	// incidence face : normal �ɍł������s�� B �̖ʁiindex ��Ԃ��j
	int PickIncidenceFace(const OBBW& I, const Vector3& n) 
	{
		float d0 = Vector3::Dot(I.axis[0], n);
		float d1 = Vector3::Dot(I.axis[1], n);
		float d2 = Vector3::Dot(I.axis[2], n);
		return ArgMax3(fabsf(d0), fabsf(d1), fabsf(d2));
	}

	// �Q�{�̐����i���S�}dir*Len�j�ŋߐړ_
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
		if (denom < 1e-12f) // �قڕ��s
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
	Matrix4x4 mat = scale * rotate; // ��]�ƃX�P�[�����������s��
	Vector3 e; // �e�v�f�̗�̐�Βl�� m_HalfSize �̓���
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
// BOX �~ BOX �iSAT�����j
// --------------------------------------------------
bool BoxCollision::isOverlapWithBox(const Transform& myTrans, const BoxCollision& box, const Transform& transBox, ContactManifold& out, float slop) const
{
	// ----- OBB �\�z -----�@�i�_�u���f�B�X�p�b�`�̕��Q�ł�����A��B�t�ɐ�������j
	OBBW B = MakeOBB(myTrans, *this);
	OBBW A = MakeOBB(transBox, box);

	// ----- SAT�i�ŏ��ѓ�(depth)�Ƃ��̏������𓾂�j -----
	AxisChoice ch;
	if (!SAT_AllAxes(A, B, ch)) { out.touching = false; out.count = 0; return false; } // �������Ă��Ȃ�������I���

	// ------ �X���[�v���l�����ĐڐG�𔻒� ------
	// depth >= -slop �Ȃ�ڐG����
	const bool touching = (ch.depth >= -slop);
	out.touching = touching;
	if (!touching) { out.count = 0; return false; }

	// �ڐG�@���iSAT �őI�΂ꂽ���j
	out.normal = ch.n; // A �� B

	// ----- manifold �̍\�z -----
	out.count = 0;

	if (ch.kind == AxisKind::FaceA || ch.kind == AxisKind::FaceB)
	{
		// face-face
		const bool refIsA = (ch.kind == AxisKind::FaceA); // ReferenceBox �̔���
		const OBBW& Ref = refIsA ? A : B; // ReferenceBox �̐ݒ�
		const OBBW& Inc = refIsA ? B : A; // IncidenceBox �̐ݒ�
		const int refAxis = refIsA ? ch.ia : ch.ib;

		// �Q�Ɩʖ@���́u�Q�Ɓ�����v�֌�����K�v������
		// out.normal �� A��B�B�Q�Ƃ�B�̂Ƃ��͔��]���� B��A �ɂ���
		//const Vector3 refN = refIsA ? out.normal : -out.normal;
		const Vector3 refN = refIsA ? out.normal : -out.normal; 

		// �Q�Ɩʂ̎l�p�`�i���͎g���ĂȂ��̂ŕ��u�j�i�f�o�b�O�`��Ɏg����݂����j
		//std::vector<Vector3> refQuad;
		//BuildRefFaceQuad(Ref, refAxis, refQuad);

		// ----- ����OBB������A�Q�Ɩ@���ɍł������s�Ȗʂ�I�ԁi���̂S���_�j-----
		const int incFace = PickIncidenceFace(Inc, refN); // incFace �̌��
		const int sign = (Vector3::Dot(Inc.axis[incFace], refN) > 0.0f) ? -1.0f : 1.0f; // refN�ɔ����s�ɂȂ�悤�ȕ���

		// ���˖ʂ̂S�ς�����đ��p�`�ɂ���
		std::vector<Vector3> poly; poly.reserve(4);
		{
			// Incidence face �̂S�������
			const int u = (incFace == 0 ? 1 : 0); // ���˖ʂ̂Q�{�̃G�b�W�����iincFace ���ȊO�̂Q���j
			const int v = (incFace == 2 ? 1 : 2);
			
			// n �Ɣ����s�ȁu�����̖ʁv�̕��ʏ�̒��S�_ 
			const Vector3 c  = Inc.center + Inc.axis[incFace] * Inc.extent[incFace] * sign; // ���Α��ʁin �ɔ����s�j
			const Vector3 U  = Inc.axis[u], V = Inc.axis[v];
			const float   eu = Inc.extent[u], ev = Inc.extent[v];
			poly.push_back(c + U * eu + V * ev); // �E��
			poly.push_back(c - U * eu + V * ev); // ����
			poly.push_back(c - U * eu - V * ev); // ����
			poly.push_back(c + U * eu - V * ev); // �E��
		}

		// ----- �Q�Ɩʂ̂S�́g���ʕ��ʁh�œ��˖ʃ|���S�����N���b�v -----
		std::vector<Vector3> poly1 = poly;
		auto clipAgainstSidePlane = [&](const Vector3& pn, float pd) // pn : ���ʂ̖@���x�N�g��, pd : ���_����̃I�t�Z�b�g�i���ʏ�̔C�ӂ̓_�j
			{
				poly1 = ClipPolygonAgainstPlane(poly1, pn, pd);
			};

		// �Q�Ɩʏ�̒��S�_�i�\���j
		const int u = (refAxis == 0 ? 1 : 0); // refAxis �ȊO�̂Q��
		const int v = (refAxis == 2 ? 1 : 2);
		const Vector3 p0 = Ref.center + refN * Ref.extent[refAxis]; // �Q�Ɩʏ�̒��S�_

		// ���E								// ���ʂ����[���h��̂ǂ��ɂ��邩���߂Ă���
		clipAgainstSidePlane( Ref.axis[u],  Vector3::Dot(Ref.axis[u], p0) + Ref.extent[u]);
		clipAgainstSidePlane(-Ref.axis[u], -Vector3::Dot(Ref.axis[u], p0) + Ref.extent[u]);
		// �㉺
		clipAgainstSidePlane( Ref.axis[v],  Vector3::Dot(Ref.axis[v], p0) + Ref.extent[v]);
		clipAgainstSidePlane(-Ref.axis[v], -Vector3::Dot(Ref.axis[v], p0) + Ref.extent[v]);

		// �c�������_�i�ڐG�_���j�� manifold ��
		for (const Vector3& q : poly1)
		{
			if (out.count >= ContactManifold::MAX_POINTS) break; // ����S�܂Łi���݂͑������̏��j

			// �Q�Ɩʂւ̗L�������Fdot(refN, q - p0)�i�\�����{�j
			// penetration �͎Q�Ɩʂւ� signed ������ -�l���g���i>=0 ���d�Ȃ�j
			const float signedDist = Vector3::Dot(q - p0, refN); // �Q�Ɩʕ\�����{�Ȃ̂� <= 0 �Ȃ�߂荞��ł���
			const float pen = -signedDist;					// pen > 0 �Ȃ�߂荞��ł���
			if (pen < -slop) continue; // slop ���������� = �߂荞��ł��Ȃ�

			// A/B ���ꂼ��́g�����̖ʏ�h�̓_�����
			if (ch.kind == AxisKind::FaceA)
			{
				out.points[out.count].pointOnB = q;
				out.points[out.count].pointOnA = q - refN * signedDist; // A �Q�Ɩʏ�
			}
			else
			{
				out.points[out.count].pointOnA = q;
				out.points[out.count].pointOnB = q + refN * signedDist; // B �Q�Ɩʏ� 
			}
			out.points[out.count].penetration = pen;
			out.count++;
		}

		// �N���b�v�őS�ł����ꍇ�̕ی��i���l�덷�΍�j
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
		// Edge-Edge�i�����ł͂Ȃ��y�߂̂��́j
		const int ia = ch.ia, ib = ch.ib;
		// ��\�G�b�W�̒��S�i�eOBB�̊Y�����ȊO�̓��e�� 0 �̒��S����OK�j
		const Vector3 Pa0 = A.center; // ��\�������S
		const Vector3 Qa0 = B.center;
		const Vector3 ua = A.axis[ia]; const float la = A.extent[ia]; // ����������
		const Vector3 ub = B.axis[ib]; const float lb = B.extent[ib];

		Vector3 pA, pB;
		ClosestPointSegmentSegment(Pa0, ua, la, Qa0, ub, lb, pA, pB);

		out.count = 1;
		out.points[0].pointOnA = pA;
		out.points[0].pointOnB = pB;
		out.points[0].penetration = ch.depth; // SAT �̍ŏ��ѓ������̂܂܂���
	}

	return true; // �S�Ă̔����ʂ����̂� true
}

// --------------------------------------------------
// BOX �~ SPHERE �i�ŋߐړ_�����j
// --------------------------------------------------
bool BoxCollision::isOverlapWithSphere(const Transform& myTrans, const SphereCollision& sphere, const Transform& transSph, ContactManifold& out, float slop) const
{
	// ----- OBB�̐��E�����擾 -----
	const Vector3 aX = myTrans.GetRight();
	const Vector3 aY = myTrans.GetUp();
	const Vector3 aZ = myTrans.GetForward();

	// ----- ���G�N�X�e���g�i�X�P�[�����f�j -----
	Vector3 extent = {
		fabsf(myTrans.scale.x) * m_HalfSize.x,
		fabsf(myTrans.scale.y) * m_HalfSize.y,
		fabsf(myTrans.scale.z) * m_HalfSize.z,
	};

	// ----- ���̒��S�Ɣ��a�擾 -----
	const Vector3 centerSph = transSph.position;
	const Vector3 centerBox = myTrans.position;
	const float   radius = sphere.Radius() * fabsf(transSph.scale.x); // ��U scale.x �ŃX�P�[�����O

	// ----- �����S�������S�ւ̃x�N�g���𔠂̎��ɓ��e -----
	const Vector3 vect = centerSph - centerBox;
	const Vector3 dist = {
		Vector3::Dot(vect, aX),
		Vector3::Dot(vect, aY),
		Vector3::Dot(vect, aZ)
	};

	// ----- �ŋߐړ_�̃��[�J�����W�iClamp�j-----
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

	// ----- ���[���h�ŋߐړ_ -----
	Vector3 qWorld = centerBox + aX * qx + aY * qy + aZ * qz; // �ڐG�_�̌v�Z

	// ----- �����S�@���@�ŋߐړ_�x�N�g�� ------
	const Vector3 vectSphToPoint = centerSph - qWorld;
	const float distSphToPointSq = vectSphToPoint.lengthSq(); 

	// �������̓����ɂ������P�[�X�΍�
	Vector3 n;
	float pen;
	if (distSphToPointSq > 1e-12f) // �������̊O�ɂ���
	{
		const float dist = sqrtf(distSphToPointSq);
		n = vectSphToPoint / dist; // �ڐG�_�����@�����iBox �� Sphere�j
		pen = radius - dist; // ���Ȃ�d�Ȃ��Ă���
	}
	else // �������̓���
	{
		// �ŋߕ��ʂ�I��
		const float dx = extent.x - fabsf(dist.x);
		const float dy = extent.y - fabsf(dist.y);
		const float dz = extent.z - fabsf(dist.z);
		if		(dx <= dy && dx <= dz) { n = (dist.x >= 0 ? aX : -aX); qx = (dist.x >= 0 ? extent.x : -extent.x); } // ��ԋ߂��̖ʂ̕����� normal �ɂ���
		else if (dy <= dz)			   { n = (dist.y >= 0 ? aY : -aY); qy = (dist.y >= 0 ? extent.y : -extent.y); }
		else						   { n = (dist.z >= 0 ? aZ : -aZ); qz = (dist.z >= 0 ? extent.z : -extent.z); }

		qWorld = centerBox + aX * qx + aY * qy + aZ * qz; // �ڐG�_�̍Čv�Z
		pen = radius + std::min(dx, std::min(dy, dz)); // ���ŏd�Ȃ��Ă���
	}
	// �ڐG���Ă��邩�̔���
	const bool touching = (pen >= -slop);

	// ----- ContactManifold �̏o�� -----
	out.touching = touching;
	out.normal = -n; // Sphere �� Box
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
	// ----- OBB�̐��E�����擾 -----
	const Vector3 aX = transBox.GetRight();
	const Vector3 aY = transBox.GetUp();
	const Vector3 aZ = transBox.GetForward();

	// ----- ���G�N�X�e���g�i�X�P�[�����f�j -----
	Vector3 extent = {
		fabsf(transBox.scale.x) * box.HalfSize().x,
		fabsf(transBox.scale.y) * box.HalfSize().y,
		fabsf(transBox.scale.z) * box.HalfSize().z,
	};

	// ----- ���̒��S�Ɣ��a�擾 -----
	const Vector3 centerSph = myTrans.position;
	const Vector3 centerBox = transBox.position;
	const float   radius = this->Radius() * fabsf(myTrans.scale.x); // ��U scale.x �ŃX�P�[�����O

	// ----- �����S�������S�ւ̃x�N�g���𔠂̎��ɓ��e -----
	const Vector3 vect = centerSph - centerBox;
	const Vector3 dist = {
		Vector3::Dot(vect, aX),
		Vector3::Dot(vect, aY),
		Vector3::Dot(vect, aZ)
	};

	// ----- �ŋߐړ_�̃��[�J�����W�iClamp�j-----
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

	// ----- ���[���h�ŋߐړ_ -----
	Vector3 qWorld = centerBox + aX * qx + aY * qy + aZ * qz; // �ڐG�_�̌v�Z

	// ----- �����S�@���@�ŋߐړ_�x�N�g�� ------
	const Vector3 vectSphToPoint = centerSph - qWorld;
	const float distSphToPointSq = vectSphToPoint.lengthSq();

	// �������̓����ɂ������P�[�X�΍�
	Vector3 n;
	float pen;
	if (distSphToPointSq > 1e-12f) // �������̊O�ɂ���
	{
		const float dist = sqrtf(distSphToPointSq);
		n = vectSphToPoint / dist; // �ڐG�_�����@�����iBox �� Sphere�j
		pen = radius - dist; // ���Ȃ�d�Ȃ��Ă���
	}
	else // �������̓���
	{
		// �ŋߕ��ʂ�I��
		const float dx = extent.x - fabsf(dist.x);
		const float dy = extent.y - fabsf(dist.y);
		const float dz = extent.z - fabsf(dist.z);
		if (dx <= dy && dx <= dz) { n = (dist.x >= 0 ? aX : -aX); qx = (dist.x >= 0 ? extent.x : -extent.x); } // ��ԋ߂��̖ʂ̕����� normal �ɂ���
		else if (dy <= dz) { n = (dist.y >= 0 ? aY : -aY); qy = (dist.y >= 0 ? extent.y : -extent.y); }
		else { n = (dist.z >= 0 ? aZ : -aZ); qz = (dist.z >= 0 ? extent.z : -extent.z); }

		qWorld = centerBox + aX * qx + aY * qy + aZ * qz; // �ڐG�_�̍Čv�Z
		pen = radius + std::min(dx, std::min(dy, dz)); // ���ŏd�Ȃ��Ă���
	}
	// �ڐG���Ă��邩�̔���
	const bool touching = (pen >= -slop);

	// ----- ContactManifold �̏o�� -----
	out.touching = touching;
	out.normal = n; // Sphere �� Box
	out.count = 1;
	out.points[0].penetration = pen;
	out.points[0].pointOnA = centerSph + out.normal * radius;
	out.points[0].pointOnB = qWorld;

	return touching;
}

// --------------------------------------------------
// Sphere �~ Sphere
// --------------------------------------------------
bool SphereCollision::isOverlapWithSphere(const Transform& myTrans, const SphereCollision& sphere, const Transform& transSph, ContactManifold& out, float slop) const
{
	// ���S
	Vector3 centerB = myTrans.position;
	Vector3 centerA = transSph.position;

	// �����i�Q��j
	Vector3 vect = centerB - centerA; // A �� B
	float distSq = vect.lengthSq();

	// ���a�i�X�P�[�����f�j
	float radiusA = m_Radius * myTrans.scale.x;			// �Ƃ肠�������̒l���g��
	float radiusB = sphere.Radius() * transSph.scale.x; // �Ƃ肠�������̒l���g��
	float sum = radiusA + radiusB;

	// �ڐG����
	if (distSq > 1e-12f) // �Q�_�Ԃ̋�����0�ȏォ�m�F
	{
		const float d = sqrtf(distSq);
		out.normal = vect / d; // A �� B
		out.points[0].penetration = sum - d; // �ѓ��[�x�v�Z
	}
	else
	{
		out.normal = { 1, 0, 0 }; // �ޔ�����
		out.points[0].penetration = sum;
	}
	out.touching = (out.points[0].penetration >= -slop); // �ѓ��[�x���� �� sum > d �� �ڐG���Ă���

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




// ���Y�^
/*
// --------------------------------------------------
// BOX �~ BOX �iSAT�����j
// --------------------------------------------------
bool BoxCollision::isOverlapWithBox(const Transform& myTrans, const BoxCollision& box, const Transform& transBox, ContactManifold& out, float slop) const
{
	// ���l����p
	constexpr float EPS_LEN2 = 1e-10f; // ���̒����O�Q�����ꖢ���Ȃ畽�s�Ƃ݂Ȃ��X�L�b�v
	constexpr float MARGIN	 = 1e-5f;	// ��r�̊ɏ�

	// ----- BoxA, B �̎����P�x�����擾���Ă��� -----
	const Vector3 aX = myTrans.GetRight();
	const Vector3 aY = myTrans.GetUp();
	const Vector3 aZ = myTrans.GetForward();
	const Vector3 bX = transBox.GetRight();
	const Vector3 bY = transBox.GetUp();
	const Vector3 bZ = transBox.GetForward();

	// ----- �����W�߂� -----
	Vector3 listAxis[15];
	// BoxA�̂R��
	listAxis[0] = aX;
	listAxis[1] = aY;
	listAxis[2] = aZ;
	// BoxB�̂R��
	listAxis[3] = bX;
	listAxis[4] = bY;
	listAxis[5] = bZ;
	// ��̎��̃N���X�X��
	int index = 6; // �U��������̂�
	for (int ia = 0; ia < 3; ia++)
	{
		for (int ib = 3; ib < 6; ib++)
		{
			Vector3 cross = Vector3::Cross(listAxis[ia], listAxis[ib]);
			if (cross.lengthSq() < EPS_LEN2) continue; // �O�ς��O�Ɍ���Ȃ��߂��@���@�قڕ��s�@���@�������Ƃ��Ė���
			cross.normalize(); // ���K��
			listAxis[index++] = cross; // ���K����Ɋi�[
		}
	}

	// ----- �e���̔���iSAT�j -----
	// Box �Ԃ̃x�N�g��
	Vector3 vect = transBox.position - myTrans.position;
	// �e Box �̔��G�N�X�e���g�i�X�P�[���K�p�j
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

	// �e���̃`�F�b�N
	for (int i = 0; i < index; i++)
	{
		// ���S�ԋ���������ɓ��e
		float dist = fabsf(Vector3::Dot(vect, listAxis[i]));

		// ���e���a
		float rA = extentA.x * fabsf(Vector3::Dot(aX, listAxis[i]))
				 + extentA.y * fabsf(Vector3::Dot(aY, listAxis[i]))
				 + extentA.z * fabsf(Vector3::Dot(aZ, listAxis[i]));
		float rB = extentB.x * fabsf(Vector3::Dot(bX, listAxis[i]))
				 + extentB.y * fabsf(Vector3::Dot(bY, listAxis[i]))
				 + extentB.z * fabsf(Vector3::Dot(bZ, listAxis[i]));

		if (dist > rA + rB + MARGIN) return false; // �������Ă���̂� false
	}

	return true; // �S�Ă̔����ʂ����̂� true
}
*/

