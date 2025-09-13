/*
	MeshFactory.h
	20250818  hanaue sho
	���상�b�V�������
	������炻�̂܂� MeshFilter �ɃZ�b�g����i���L���� MeshFilter �j
*/
#ifndef MESHFACTORY_H_
#define MESHFACTORY_H_
#include <d3d11.h>
#include <cassert>
#include "MeshFilterComponent.h"
#include "Renderer.h"
#include "Vector3.h"


// ���ʁiXY�j
struct QuadParams
{
	float width = 200.0f;
	float height = 200.0f;
	bool originCenter = true; // true = ���S���_, false = ���㌴�_
};

// ���ʁiXZ�j
struct PlaneParams
{
	float width = 10.0f;
	float depth = 10.0f;
	bool originCenter = true; // true = ���S���_, false = ���㌴�_
};

// ����
struct SphereParams
{
	float radius = 1.0f;
	int slices = 6; // �������i�o�x�j
	int stacks = 6; // �c�����i�ܓx�j
	bool insideOut = false; // true: �X�J�C�h�[���p
};

// ������
struct CubeParams
{
	Vector3 size = Vector3(1, 1, 1);
	bool originCenter = true;   // true = ���S���_, false = ���㌴�_
	bool insideOut = false;		// true: ����
};

// �J�v�Z��
struct CapsuleParams
{
	float radius = 1.0f;		 // ���a
	float cylinderHeight = 1.0f; // ���̒���
	int slices = 24;			 // ������
	int stacksBody = 1;			 // ���ʂ̏c����
	int stacksCap  = 4;			 // �����̏c����
	bool insideOut = false;		 // ���]
};

// �~��
struct CylinderParams
{
	float radius = 1.0f;
	float height = 2.0f;
	int slices = 24;		// ������
	int stacks = 1;			// ���ʂ̏c����
	bool insideOut = false;	// ���]
};



class MeshFactory
{
public:
	static void CreateQuad	  (MeshFilterComponent* filter, const QuadParams& p = {});
	static void CreatePlane   (MeshFilterComponent* filter, const PlaneParams& p = {});
	static void CreateSphere  (MeshFilterComponent* filter, const SphereParams& p = {});
	static void CreateCube	  (MeshFilterComponent* filter, const CubeParams& p = {});
	static void CreateCylinder(MeshFilterComponent* filter, const CylinderParams& p = {});
	static void CreateCapsule (MeshFilterComponent* filter, const CapsuleParams& p = {});
};



#endif