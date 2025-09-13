/*
	ContactManifold.h
	20250901  hanaue sho
	�ڐG�_�̏��̍\����
*/
#ifndef CONTACTMANIFOLD_H_
#define CONTACTMANIFOLD_H_
#include"Vector3.h"

struct ContactPoint
{
	Vector3 pointOnA{}; // A�̐ڐG�_
	Vector3 pointOnB{}; // B�̐ڐG�_
	float penetration = 0.0f; // �ѓ��[�x
	float accumN = 0.0f; // �@���̗ݐϐ��K�C���p���X�i�P���Ȃ�X�J���[�ŗǂ��j
	Vector3 accumImpulseT = Vector3(); // �ڐ��̗ݐϐ��K�C���p���X�i�P���Ȃ�X�J���[�ŗǂ��j
};

struct ContactManifold
{
	static constexpr int MAX_POINTS = 4; // OBB*OBB �Ȃ̂łS��
	bool touching = false; // �G��Ă��邩����
	Vector3 normal{};		// A �� B �ւ̒P�ʃx�N�g��
	int count = 0; // �L���ȐڐG�_�̐�
	ContactPoint points[4]; // �ő�S�_
};

#endif