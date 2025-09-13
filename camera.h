/*
	Camera.h
	20250818 hanaue sho
*/
#ifndef CAMERA_H_
#define CAMERA_H_
#include "GameObject.h"
#include "TransformComponent.h"
#include "CameraComponent.h"

// ----- MyCameraClass -----
// --------------------------------------------------
// �J�������N���X
// --------------------------------------------------
class Camera : public GameObject
{
public:
	void Init() override;
	//void Uninit() override {}
	//void Update(float dt) {}
	//void Draw() override {}
};


/*
// --------------------------------------------------
// ���R�J����
// �^�[�Q�b�g�ɒǏ]����J����
// --------------------------------------------------
class FreeCamera : public Camera
{
private:
	XMVECTOR m_Position = {}; // ���W
	XMFLOAT3 m_TargetPosition = {};
	float m_Distance = -5.0f; // �Ώۂ܂ł̐�������

	XMVECTOR m_VectorRight = { 1.0f, 0.0f, 0.0f }; // �E�����x�N�g��
	XMVECTOR m_VectorTop = { 0.0f, 1.0f, 0.0f }; // ������x�N�g��
	XMVECTOR m_VectorFront = { 0.0f, 0.0f, 1.0f }; // �O�����x�N�g��

	XMVECTOR m_FocusPosition = {}; // �����_

public:
	FreeCamera() = delete;
	FreeCamera(const XMFLOAT3& targetPosition, float dist) : m_TargetPosition(targetPosition), m_Distance(dist) {}

	void Init() override {}
	void Uninit() override {}
	void Update() override;
	void Draw() override;


	XMVECTOR GetPosition() const { return m_Position; }
	XMVECTOR GetVectorRight() const { return m_VectorRight; }
	XMVECTOR GetVectorTop() const { return m_VectorTop; }
	XMVECTOR GetVectorFront() const { return m_VectorFront; }
};

// --------------------------------------------------
// �t�H���[�J����
// �^�[�Q�b�g�ɒǏ]����J����
// --------------------------------------------------
class FollowCamera : public Camera
{
private:
	XMVECTOR m_Position = {0, 0, 0}; // ���W
	GameObject* m_pTargetObject = nullptr;
	float m_Distance = -5.0f; // �Ώۂ܂ł̐�������

	XMVECTOR m_VectorRight = { 1.0f, 0.0f, 0.0f }; // �E�����x�N�g��
	XMVECTOR m_VectorTop = { 0.0f, 1.0f, 0.0f }; // ������x�N�g��
	XMVECTOR m_VectorFront = { 0.0f, 0.0f, 1.0f }; // �O�����x�N�g��

	XMVECTOR m_FocusPosition = {0, 0, 1}; // �����_

public:
	FollowCamera() = default;
	FollowCamera(GameObject* pTarget, float dist) : m_pTargetObject(pTarget), m_Distance(dist) {}

	void Init() override {}
	void Uninit() override {}
	void Update();
	void Draw() override;

	XMVECTOR GetPosition() const { return m_Position; }
	XMVECTOR GetVectorRight() const { return m_VectorRight; }
	XMVECTOR GetVectorTop() const { return m_VectorTop; }
	XMVECTOR GetVectorFront() const { return m_VectorFront; }

	void SetTarget(GameObject* pObject) { m_pTargetObject = pObject; }
};
*/

#endif