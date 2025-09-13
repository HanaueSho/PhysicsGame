/*
	Camera.cpp
	20250818 hanaue sho
*/
#include	"Camera.h"
#include	"CameraMove.h"
#include	"mouse.h"


// --------------------------------------------------
// �J�����i���N���X�j
// --------------------------------------------------
void Camera::Init()
{
	auto* tf = GetComponent<TransformComponent>();
	auto* camera = AddComponent<CameraComponent>();
	auto* cameraMove = AddComponent<CameraMove>();

	// 2D�ݒ�
	camera->SetMode(CameraComponent::Mode::Ortho2D);
	camera->SetOrthoOffCenter(0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f, 0.0f, 0.1f);
	camera->UseTransformIn2D(false); // �Œ� 2D �J����

	// 3D�ݒ�
	// cam->SetMode(CameraComponent::Mode::Perspective);
	// cam->SetPerspective(DirectX::XMConvertToRadians(60.0f),
	//                     (float)SCREEN_WIDTH / SCREEN_HEIGHT,
	//                     0.1f, 1000.0f);
	// tf->SetPosition({0, 2, -5});

}



/*
// --------------------------------------------------
// �X�N���[����̍��W��Ԃ�
// �����F���[���h���W
// --------------------------------------------------
XMFLOAT2 Camera::GetPositionOnScreen(const XMFLOAT3& position)
{
	XMMATRIX worldMatrix = XMMatrixTranslation(position.x, position.y, position.z); // ���f�����W���烏�[���h���W�ւ̕ϊ�
	XMMATRIX viewMatrix = m_mtxView; // ���[���h���W����r���[���W�ւ̕ϊ�
	XMMATRIX projectionMatrix = m_mtxPerspective; // �r���[���W����v���W�F�N�V�������W�ւ̕ϊ�

	XMVECTOR worldPos = { position.x, position.y, position.z, 1.0f }; // �I�u�W�F�N�g�̍��W�i���[���h���W�j
	XMVECTOR viewPos = XMVector3TransformCoord(worldPos, viewMatrix); // �r���[��Ԃ֕ϊ�
	XMVECTOR projPos = XMVector3TransformCoord(viewPos, projectionMatrix); // �v���W�F�N�V������Ԃ֕ϊ�

	// �v���W�F�N�V�����s��̌��ʂƂ��ē�������W��[-1, 1]�͈̔�
	XMFLOAT2 positionOnScreen;
	positionOnScreen.x = (projPos.m128_f32[0] + 1.0f) * 0.5f * SCREEN_WIDTH;
	positionOnScreen.y = (1.0f - projPos.m128_f32[1]) * 0.5f * SCREEN_HEIGHT;

	/*
		���Y�^
		����̈ʒu�֌W�ɂ���Ă̓J�����̔w�ʂɂ�����̂���ʓ��̍��W���w���Ă��܂����Ƃ�����B
		�Ή���Ƃ��Ă̓J�����Ƃ̈ʒu�֌W���m�F���Ĕw�ʂ̂Ƃ��͕`�悵�Ȃ��悤�ɂ���ȂǁB
	

	return positionOnScreen;
}
*/
/*
// --------------------------------------------------
// �t���[�J����
// ���R�ɓ�����J����
// �}�E�X�z�C�[�� : �^�[�Q�b�g�܂ł̋�������
// �E�N���b�N : ����
// --------------------------------------------------
void FreeCamera::Update()
{
	// �}�E�X�ɂ��J�����ړ� ----------
	if (Mouse_IsWheelScroll()) m_Distance += 0.5f * Mouse_GetWheelScroll(); // ��������
	m_Distance = Clamp(m_Distance, -70, -0.5f);

	// �N���b�N���Ă���ԂɃ}�E�X�̈ړ��ʂ𔽉f -----
	static XMFLOAT2 mousePos = {};
	static XMFLOAT2 mousePosOld = {};
	// �E�N���b�N�ŉ�] -----
	if (Mouse_IsClick(MS_CLICK_RIGHT))
	{
		// �擾
		if (Mouse_IsClickTrigger(MS_CLICK_RIGHT))
		{
			mousePosOld.x = Mouse_GetPositionX();
			mousePosOld.y = Mouse_GetPositionY();
		}
		mousePos.x = Mouse_GetPositionX();
		mousePos.y = Mouse_GetPositionY();

		// ���[�J���x�N�g������]������
		{
			XMMATRIX rotation = XMMatrixRotationAxis(m_VectorTop, -(mousePosOld.x - mousePos.x) * 0.005f);
			m_VectorFront = XMVector3TransformNormal(m_VectorFront, rotation);
			m_VectorRight = XMVector3Cross(m_VectorTop, m_VectorFront); // �O��

			m_VectorFront = XMVector3Normalize(m_VectorFront); // ���K��
			m_VectorRight = XMVector3Normalize(m_VectorRight); // ���K��
		}
		{
			XMMATRIX rotation = XMMatrixRotationAxis(m_VectorRight, -(mousePosOld.y - mousePos.y) * 0.005f);
			m_VectorFront = XMVector3TransformNormal(m_VectorFront, rotation);
			m_VectorTop = XMVector3Cross(m_VectorFront, m_VectorRight); // �O��

			m_VectorFront = XMVector3Normalize(m_VectorFront); // ���K��
			m_VectorTop = XMVector3Normalize(m_VectorTop); // ���K��
		}

		// �L�^
		mousePosOld.x = mousePos.x;
		mousePosOld.y = mousePos.y;
	}
	// ���N���b�N�ňړ� -----
	if (Mouse_IsClick(MS_CLICK_MIDDLE))
	{
		if (Mouse_IsClickTrigger(MS_CLICK_MIDDLE))
		{
			mousePosOld.x = Mouse_GetPositionX();
			mousePosOld.y = Mouse_GetPositionY();
		}
		mousePos.x = Mouse_GetPositionX();
		mousePos.y = Mouse_GetPositionY();

		// �}�E�X�̈ړ��ʂňړ� -----
		XMFLOAT3 targetPosition = m_TargetPosition;
		XMFLOAT3 cameraPosition = {};
		XMStoreFloat3(&cameraPosition, m_Position);
		XMFLOAT3 vecTop = {};
		XMStoreFloat3(&vecTop, m_VectorTop);
		XMFLOAT3 vecRight = {};
		XMStoreFloat3(&vecRight, m_VectorRight);

		XMFLOAT2 diff = (mousePosOld - mousePos) * 0.01f;
		diff.y *= -1;

		targetPosition.x += vecTop.x * diff.y + vecRight.x * diff.x;
		targetPosition.y += vecTop.y * diff.y + vecRight.y * diff.x;
		targetPosition.z += vecTop.z * diff.y + vecRight.z * diff.x;
		cameraPosition.x += vecTop.x * diff.y + vecRight.x * diff.x;
		cameraPosition.y += vecTop.y * diff.y + vecRight.y * diff.x;
		cameraPosition.z += vecTop.z * diff.y + vecRight.z * diff.x;

		m_TargetPosition = targetPosition; // �X�V
		m_Position = XMLoadFloat3(&cameraPosition); // �X�V

		// �L�^
		mousePosOld.x = mousePos.x;
		mousePosOld.y = mousePos.y;

		//printf("m_Position: %f, %f, %f\n", cameraPosition.x, cameraPosition.y, cameraPosition.z);
		//printf("m_Position: %f, %f, %f\n", m_TargetPosition.x, m_TargetPosition.y, m_TargetPosition.z);
	}


	// �J�������W�v�Z ----------
	XMMATRIX rotation =
	{
		{m_VectorRight},
		{m_VectorTop},
		{m_VectorFront},
		{0.0f,   0.0f,   0.0f,   1.0f}
	}; // ���[�J���x�N�g���ɂ���]�s��
	XMMATRIX translation = XMMatrixTranslationFromVector(XMLoadFloat3(&m_TargetPosition)); // �Ώۂ� position
	XMMATRIX translation1 = XMMatrixTranslation(0.0f, 0.0f, m_Distance); // ���a���̈ړ�����

	XMMATRIX world = translation1 * rotation * translation; // �ϊ��s��

	XMVECTOR position = XMVector3TransformCoord({ 0.0f, 0.0f, 0.0f }, world); // �ϊ���� position

	XMVECTOR vect = position - m_Position; // �����x�N�g��
	vect *= 0.05f;
	m_Position += vect;

	// �J�����̌����̌v�Z ----------
	XMVECTOR targetPos = XMLoadFloat3(&m_TargetPosition);
	vect = targetPos - m_FocusPosition; // �����x�N�g��
	vect *= 0.05f * 2;
	m_FocusPosition += vect;

	//                               cameraPosition          camera�̌���        camera�̏�x�N�g��
	XMMATRIX view = XMMatrixLookAtLH(m_Position, m_FocusPosition, m_VectorTop); // LH...LeftHand(������W�n)
	//                                              camera�̉�p  �A�X�y�N�g��  nearZ  farZ
	XMMATRIX perspective = XMMatrixPerspectiveFovLH(XM_PI / 3.0f, (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.01f, 1000.0f); // �p�[�X�y�N�e�B�u���W�ϊ� // XMConvertToRadians(60) �ł���

	SetMatrixView(view);
	SetMatrixPerspective(perspective);


}

void FreeCamera::Draw()
{
	//�v���W�F�N�V�����s����쐬
	XMMATRIX	ProjectionMatrix = GetMatrixPerspective();
	//�v���W�F�N�V�����s����Z�b�g
	Renderer::SetProjectionMatrix(ProjectionMatrix);

	//�J�����s����쐬
	XMVECTOR	eyev = m_FocusPosition;
	XMVECTOR	pos = m_Position;
	XMVECTOR	up = m_VectorTop;
	XMMATRIX	ViewMatrix = XMMatrixLookAtLH(pos, eyev, up);

	//�J�����s����Z�b�g
	Renderer::SetViewMatrix(GetMatrixView());

	Renderer::SetDepthEnable(true);

	////�J�������W�Z�b�g
	//XMFLOAT3 position;
	//XMStoreFloat3(&position, GetPosition());
	//Renderer::SetCameraPosition(position);
}

// --------------------------------------------------
// �t�H���[�J����
// �^�[�Q�b�g�ɒǏ]����J����
// �}�E�X�z�C�[�� : �^�[�Q�b�g�܂ł̋�������
// �E�N���b�N : ����
// --------------------------------------------------
void FollowCamera::Update()
{
	// �}�E�X�ɂ��J�����ړ� ----------
	if (Mouse_IsWheelScroll()) m_Distance += 1.5f * Mouse_GetWheelScroll(); // ��������
	m_Distance = Clamp(m_Distance, -70, -3);

	// �N���b�N���Ă���ԂɃ}�E�X�̈ړ��ʂ𔽉f
	static XMFLOAT2 mousePos = {};
	static XMFLOAT2 mousePosOld = {};
	if (Mouse_IsClick(MS_CLICK_RIGHT))
	{
		// �擾
		if (Mouse_IsClickTrigger(MS_CLICK_RIGHT))
		{
			mousePosOld.x = Mouse_GetPositionX();
			mousePosOld.y = Mouse_GetPositionY();
		}
		mousePos.x = Mouse_GetPositionX();
		mousePos.y = Mouse_GetPositionY();

		// ���[�J���x�N�g������]������
		{
			XMMATRIX rotation = XMMatrixRotationAxis(m_VectorTop, -(mousePosOld.x - mousePos.x) * 0.005f);
			m_VectorFront = XMVector3TransformNormal(m_VectorFront, rotation);
			m_VectorRight = XMVector3Cross(m_VectorTop, m_VectorFront); // �O��

			m_VectorFront = XMVector3Normalize(m_VectorFront); // ���K��
			m_VectorRight = XMVector3Normalize(m_VectorRight); // ���K��
		}
		{
			XMMATRIX rotation = XMMatrixRotationAxis(m_VectorRight, -(mousePosOld.y - mousePos.y) * 0.005f);
			m_VectorFront = XMVector3TransformNormal(m_VectorFront, rotation);
			m_VectorTop = XMVector3Cross(m_VectorFront, m_VectorRight); // �O��

			m_VectorFront = XMVector3Normalize(m_VectorFront); // ���K��
			m_VectorTop = XMVector3Normalize(m_VectorTop); // ���K��
		}

		// �L�^
		mousePosOld.x = mousePos.x;
		mousePosOld.y = mousePos.y;
	}

	// �J�������W�v�Z ----------
	XMMATRIX rotation =
	{
		{m_VectorRight},
		{m_VectorTop},
		{m_VectorFront},
		{0.0f,   0.0f,   0.0f,   1.0f}
	}; // ���[�J���x�N�g���ɂ���]�s��

	XMFLOAT3 positionTarget = { m_pTargetObject->GetPosition().x, m_pTargetObject->GetPosition().y, m_pTargetObject->GetPosition().z };
	XMMATRIX translation = XMMatrixTranslationFromVector(XMLoadFloat3(&positionTarget)); // �Ώۂ� position
	XMMATRIX translation1 = XMMatrixTranslation(0.0f, 0.0f, m_Distance); // ���a���̈ړ�����

	XMMATRIX world = translation1 * rotation * translation; // �ϊ��s��

	XMVECTOR position = XMVector3TransformCoord({ 0.0f, 0.0f, 0.0f }, world); // �ϊ���� position

	XMVECTOR vect = position - m_Position; // �����x�N�g��
	vect *= 0.05f;
	m_Position += vect;

	// �J�����̌����̌v�Z ----------
	XMVECTOR targetPos = XMLoadFloat3(&positionTarget);
	vect = targetPos - m_FocusPosition; // �����x�N�g��
	vect *= 0.05f;
	m_FocusPosition += vect;

	//                               cameraPosition          camera�̌���        camera�̏�x�N�g��
	XMMATRIX view = XMMatrixLookAtLH(m_Position, m_FocusPosition, m_VectorTop); // LH...LeftHand(������W�n)
	//                                              camera�̉�p  �A�X�y�N�g��  nearZ  farZ
	XMMATRIX perspective = XMMatrixPerspectiveFovLH(XM_PI / 3.0f, (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.01f, 1000.0f); // �p�[�X�y�N�e�B�u���W�ϊ� // XMConvertToRadians(60) �ł���

	SetMatrixView(view);
	SetMatrixPerspective(perspective);

	// �킯�����Ăނ������iZ�\�[�g�֌W�j
	XMFLOAT3 pos;
	Vector3 posVec;
	XMStoreFloat3(&pos, m_Position);
	posVec.x = pos.x;
	posVec.y = pos.y;
	posVec.z = pos.z;
	SetPosition(posVec);
}

void FollowCamera::Draw()
{
	//�v���W�F�N�V�����s����쐬
	XMMATRIX	ProjectionMatrix = GetMatrixPerspective();
	//�v���W�F�N�V�����s����Z�b�g
	Renderer::SetProjectionMatrix(ProjectionMatrix);

	//�J�����s����쐬
	XMVECTOR	eyev = m_FocusPosition;
	XMVECTOR	pos = m_Position;
	XMVECTOR	up = m_VectorTop;
	XMMATRIX	ViewMatrix = XMMatrixLookAtLH(pos, eyev, up);

	//�J�����s����Z�b�g
	Renderer::SetViewMatrix(GetMatrixView());


	Renderer::SetDepthEnable(true);
}
*/
