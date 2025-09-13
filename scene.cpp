#include <algorithm> // std::stable_sort
#include "scene.h"
#include "manager.h"
#include "renderer.h"

#include "camera.h"
#include "TransformComponent.h"
#include "MeshRendererComponent.h"
#include "MeshFilterComponent.h"
#include "MaterialComponent.h"

#include "PhysicsSystem.h"


Scene::~Scene() = default; // cpp �Œ�`���Ȃ��ƃ����J�[�G���[���o��

void Scene::Init()
{
	m_pPhysicsSystem.reset(new PhysicsSystem(*this)); // make_unique �� deleter �ɂ͎g���Ȃ�
	m_pPhysicsSystem->Init();
}

void Scene::Uninit()
{
	for (auto& gameObjectList : m_GameObjects)
	{
		for (auto* gameObject : gameObjectList)
		{
			if (!gameObject) continue;

			gameObject->Uninit();
			delete gameObject;
		}
		gameObjectList.clear();
	}

	if (m_pPhysicsSystem) { m_pPhysicsSystem->Shutdown(); m_pPhysicsSystem.reset(); }
}

void Scene::Update(float dt)
{
	for (auto& gameObjectList : m_GameObjects)
	{
		for (auto* gameObject : gameObjectList)
		{
			if (gameObject)
				gameObject->Update(dt);
		}
	}
	// �I�u�W�F�N�g�j������
	for (auto& gameObjectList : m_GameObjects) // �Q�ƌ^����Ȃ��ƃo�O���
	{
		gameObjectList.remove_if(
			[](GameObject* object)
			{
				if (object && object->IsDestroyRequested())
				{
					object->Uninit();
					delete object; // ���L�҂� delete
					return true; // ���X�g����O��
				}
				return false;
			});
	}
}

void Scene::FixedUpdate(float fixedDt)
{
	for (auto& gameObjectList : m_GameObjects)
	{
		for (auto* gameObject : gameObjectList)
		{
			if (gameObject)
				gameObject->FixedUpdate(fixedDt);
		}
	}

	// ----- �������Z���P�X�e�b�v -----
	m_pPhysicsSystem->BeginStep(fixedDt);
	m_pPhysicsSystem->Step(fixedDt);
	m_pPhysicsSystem->EndStep(fixedDt);
}

void Scene::Draw()
{
	// ���C���J�����擾
	Camera* camera = GetGameObject<Camera>();
	Vector3 cameraPos{}, cameraFwd{ 0, 0, 1 };
	if (camera)
	{
		cameraPos = camera->GetComponent<TransformComponent>()->WorldPosition();
		cameraFwd = camera->GetComponent<TransformComponent>()->Forward().normalized();
	}

	// �`�悷��I�u�W�F�N�g�̍\����
	struct DrawItem
	{
		MeshRendererComponent* mr    = nullptr;
		MaterialComponent*	   mat	 = nullptr;
		float				   depth = 0.0f;
	};

	std::vector<DrawItem> opaques;      // �s����
	std::vector<DrawItem> transparents; // ����
	opaques.reserve(256);      // �̈�m��
	transparents.reserve(256); // �̈�m��

	// ----- �`��Ώۂ����W -----
	for (auto& gameObjectList : m_GameObjects)
	{
		for (auto* gameObject : gameObjectList)
		{
			if (!gameObject) continue;

			// �`������̂����E��
			auto* mr  = gameObject->GetComponent<MeshRendererComponent>();
            auto* mat = gameObject->GetComponent<MaterialComponent>();
            auto* tf  = gameObject->GetComponent<TransformComponent>();
            if (!mr || !mat || !tf) continue;

			// ���������Ő[�x���o��
			float d = 0.0f;
			if (camera)
			{
				const Vector3 objectPos = tf->WorldPosition(); 
				d = Vector3::Dot(objectPos - cameraPos, cameraFwd);
			}

			DrawItem item{ mr, mat, d };
			if (mat->IsTransparent()) transparents.push_back(item);
			else					  opaques.push_back(item);
		}
	}

	// ----- �\�[�g -----
	// �s�����F front-to-back�i������depth����O����j
	std::stable_sort(opaques.begin(), opaques.end(),
		[](const DrawItem& a, const DrawItem& b) {return a.depth < b.depth; });
	// �����F�@back-to-front�i�傫��depth��������j
	std::stable_sort(transparents.begin(), transparents.end(),
		[](const DrawItem& a, const DrawItem& b) {return a.depth > b.depth; });

	// ----- �`�� -----
	// �s�����idepth��������ON, �u�����hOFF�j
	for (auto& it : opaques) it.mr->Draw();
	// �����idepthj��������OFF����, �u�����hON�j
	for (auto& it : transparents) it.mr->Draw();

	Renderer::SetDepthEnable(true); // ��n��
}

void PhysicsSystemDeleter::operator()(PhysicsSystem* p) noexcept
{
	delete p;
}
