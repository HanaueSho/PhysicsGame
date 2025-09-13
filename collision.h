/*
	collision.h
	hanaue sho 20241119
*/
#ifndef COLLISION_H_
#define COLLISION_H_
#include "Vector3.h"
#pragma message("Collision.h included from: " __FILE__)

// �\����
struct AABB
{
	Vector3 min{}, max{};
	bool isOverlap(const AABB& o) const
	{
		return !(max.x < o.min.x || min.x > o.max.x
			  || max.y < o.min.y || min.y > o.max.y
			  || max.z < o.min.z || min.z > o.max.z);
	}
};

// �O���錾
class GameObject;
struct Transform; // �R�C�c�̓N���X����Ȃ��̂Œ���
struct ContactManifold; // �R�C�c�̓N���X����Ȃ��̂Œ���
class BoxCollision; 
class SphereCollision; 
class CapsuleCollision;

// --------------------------------------------------
// �RD�R���W�����C���^�t�F�[�X
// --------------------------------------------------
class Collision
{
public:
	virtual ~Collision() = default;

	// �u���[�h�p
	virtual AABB ComputeWorldAABB(const Transform& ownerTrans) const = 0;
	
	// �_�u���f�B�X�p�b�`����
	virtual bool isOverlap			 (const Transform& myTrans, const Collision& collisionB,	 const Transform& transB  , ContactManifold& out, float slop) const = 0;
	virtual bool isOverlapWithBox	 (const Transform& myTrans, const BoxCollision& box,		 const Transform& transBox, ContactManifold& out, float slop) const = 0;
	virtual bool isOverlapWithSphere (const Transform& myTrans, const SphereCollision& sphere,	 const Transform& transSph, ContactManifold& out, float slop) const = 0;
	virtual bool isOverlapWithCapsule(const Transform& myTrans, const CapsuleCollision& capsule, const Transform& transCap, ContactManifold& out, float slop) const = 0;
};

// --------------------------------------------------
// Box
// --------------------------------------------------
class BoxCollision : public Collision
{
private:
	Vector3 m_HalfSize; // �����̃T�C�Y

public:
	BoxCollision(const Vector3& halfSize) : m_HalfSize(halfSize) {}

	const Vector3& HalfSize() const { return m_HalfSize; }

	AABB ComputeWorldAABB(const Transform& ownerTrans) const override;

	bool isOverlap			 (const Transform& myTrans, const Collision& collisionB,	 const Transform& transB  , ContactManifold& out, float slop) const override;
	bool isOverlapWithBox	 (const Transform& myTrans, const BoxCollision& box,		 const Transform& transBox, ContactManifold& out, float slop) const override;
	bool isOverlapWithSphere (const Transform& myTrans, const SphereCollision& sphere,	 const Transform& transSph, ContactManifold& out, float slop) const override;
	bool isOverlapWithCapsule(const Transform& myTrans, const CapsuleCollision& capsule, const Transform& transCap, ContactManifold& out, float slop) const override;
};

// --------------------------------------------------
// Sphere
// --------------------------------------------------
class SphereCollision : public Collision
{
private:
	float m_Radius; // ���a

public:
	SphereCollision(float radius) : m_Radius(radius) {}

	float Radius() const { return m_Radius; }

	AABB ComputeWorldAABB(const Transform& ownerTrans) const override;

	bool isOverlap			 (const Transform& myTrans, const Collision& collisionB,	 const Transform& transB  , ContactManifold& out, float slop) const override;
	bool isOverlapWithBox	 (const Transform& myTrans, const BoxCollision& box,		 const Transform& transBox, ContactManifold& out, float slop) const override;
	bool isOverlapWithSphere (const Transform& myTrans, const SphereCollision& sphere,	 const Transform& transSph, ContactManifold& out, float slop) const override;
	bool isOverlapWithCapsule(const Transform& myTrans, const CapsuleCollision& capsule, const Transform& transCap, ContactManifold& out, float slop) const override;
};

// --------------------------------------------------
// Capsule
// --------------------------------------------------
class CapsuleCollision : public Collision
{
private:
	float m_Radius; // ���a
	float m_Height; // ����

public:
	CapsuleCollision(float radius, float height) : m_Radius(radius), m_Height(height) {}

	float Radius() const { return m_Radius; }
	float Height() const { return m_Height; }

	AABB ComputeWorldAABB(const Transform& ownerTrans) const override;
	
	bool isOverlap			 (const Transform& myTrans, const Collision& collisionB,	 const Transform& transB  , ContactManifold& out, float slop) const override;
	bool isOverlapWithBox	 (const Transform& myTrans, const BoxCollision& box,		 const Transform& transBox, ContactManifold& out, float slop) const override;
	bool isOverlapWithSphere (const Transform& myTrans, const SphereCollision& sphere,	 const Transform& transSph, ContactManifold& out, float slop) const override;
	bool isOverlapWithCapsule(const Transform& myTrans, const CapsuleCollision& capsule, const Transform& transCap, ContactManifold& out, float slop) const override;
};

#endif