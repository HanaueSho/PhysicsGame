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


Scene::~Scene() = default; // cpp で定義しないとリンカーエラーが出る

void Scene::Init()
{
	m_pPhysicsSystem.reset(new PhysicsSystem(*this)); // make_unique は deleter には使えない
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
	// オブジェクト破棄処理
	for (auto& gameObjectList : m_GameObjects) // 参照型じゃないとバグるよ
	{
		gameObjectList.remove_if(
			[](GameObject* object)
			{
				if (object && object->IsDestroyRequested())
				{
					object->Uninit();
					delete object; // 所有者が delete
					return true; // リストから外す
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

	// ----- 物理演算を１ステップ -----
	m_pPhysicsSystem->BeginStep(fixedDt);
	m_pPhysicsSystem->Step(fixedDt);
	m_pPhysicsSystem->EndStep(fixedDt);
}

void Scene::Draw()
{
	// メインカメラ取得
	Camera* camera = GetGameObject<Camera>();
	Vector3 cameraPos{}, cameraFwd{ 0, 0, 1 };
	if (camera)
	{
		cameraPos = camera->GetComponent<TransformComponent>()->WorldPosition();
		cameraFwd = camera->GetComponent<TransformComponent>()->Forward().normalized();
	}

	// 描画するオブジェクトの構造体
	struct DrawItem
	{
		MeshRendererComponent* mr    = nullptr;
		MaterialComponent*	   mat	 = nullptr;
		float				   depth = 0.0f;
	};

	std::vector<DrawItem> opaques;      // 不透明
	std::vector<DrawItem> transparents; // 透明
	opaques.reserve(256);      // 領域確保
	transparents.reserve(256); // 領域確保

	// ----- 描画対象を収集 -----
	for (auto& gameObjectList : m_GameObjects)
	{
		for (auto* gameObject : gameObjectList)
		{
			if (!gameObject) continue;

			// 描けるものだけ拾う
			auto* mr  = gameObject->GetComponent<MeshRendererComponent>();
            auto* mat = gameObject->GetComponent<MaterialComponent>();
            auto* tf  = gameObject->GetComponent<TransformComponent>();
            if (!mr || !mat || !tf) continue;

			// 視線方向で深度を出す
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

	// ----- ソート -----
	// 不透明： front-to-back（小さいdepth＝手前から）
	std::stable_sort(opaques.begin(), opaques.end(),
		[](const DrawItem& a, const DrawItem& b) {return a.depth < b.depth; });
	// 透明：　back-to-front（大きいdepth＝奥から）
	std::stable_sort(transparents.begin(), transparents.end(),
		[](const DrawItem& a, const DrawItem& b) {return a.depth > b.depth; });

	// ----- 描画 -----
	// 不透明（depth書き込みON, ブレンドOFF）
	for (auto& it : opaques) it.mr->Draw();
	// 透明（depthj書き込みOFF推奨, ブレンドON）
	for (auto& it : transparents) it.mr->Draw();

	Renderer::SetDepthEnable(true); // 後始末
}

void PhysicsSystemDeleter::operator()(PhysicsSystem* p) noexcept
{
	delete p;
}
