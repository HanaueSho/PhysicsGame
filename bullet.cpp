/*
	bullet.cpp
	20250514 hanaue sho
*/
#include "bullet.h"
#include "main.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "keyboard.h"
#include "camera.h"
#include "manager.h"
#include "enemy.h"
#include "CALCULATION_HELPER.h"
#include "explosion.h"
#include "scene.h"
#include "score.h"
#include "MathConversion.h"

void Bullet::Init()
{
	m_pModelRenderer = new ModelRenderer();
	m_pModelRenderer->Load("assets\\model\\bullet.obj");
	
	Renderer::CreateVertexShader(&m_pVertexShader, &m_pVertexLayout, "shader\\unlitTextureVS.cso");

	Renderer::CreatePixelShader(&m_pPixelShader, "shader\\unlitTexturePS.cso");

	// テクスチャ読み込み
	TexMetadata metadata;
	ScratchImage image;
	LoadFromWICFile(L"assets\\texture\\kirby.png", WIC_FLAGS_NONE, &metadata, image);
	//LoadFromWICFile(L"assets\\texture\\HumanTube.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(Renderer::GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &m_pTexture);
	assert(m_pTexture);
}

void Bullet::Uninit()
{
	delete m_pModelRenderer;
	m_pModelRenderer = nullptr;

	m_pVertexShader->Release();
	m_pPixelShader->Release();
	m_pVertexLayout->Release();
}

void Bullet::Update()
{
	/*
	float dt = 0.1f;

	m_Transform.position = m_Transform.position + GetForward() * dt;

	if (m_Transform.position.length() > 10.0f)
	{
		SetDestroy();
	}

	std::vector<Enemy*> pListEnemy = Manager::GetScene()->GetGameObjects<Enemy>();
	for(Enemy * enemy : pListEnemy)
	{
		Vector3 vect = enemy->GetPosition() + Vector3{ 0, 1, 0 } - GetPosition();
		float dist = vect.length();

		if (dist < 0.5f + 0.1f) // エネミーの半径＋弾の半径
		{
			Explosion* clone = Manager::GetScene()->AddGameObject<Explosion>(1); // エフェクト生成
			clone->Init();
			clone->SetPosition(enemy->GetPosition() + Vector3{ 0, 1, 0 });

			enemy->SetDestroy();
			SetDestroy(); // 自身も破棄

			// スコア増加
			Manager::GetScene()->GetGameObject<Score>()->Add(1);

		}
	}

	// エフェクト生成
	Explosion* clone = Manager::GetScene()->AddGameObject<Explosion>(1); // エフェクト生成
	clone->Init();
	clone->SetPosition(GetPosition());
	*/
}

void Bullet::Draw()
{
	/*
	// 入力レイアウト
	Renderer::GetDeviceContext()->IASetInputLayout(m_pVertexLayout);

	// シェーダー設定
	Renderer::GetDeviceContext()->VSSetShader(m_pVertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_pPixelShader, NULL, 0);
	
	// テクスチャ設定
	Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_pTexture);

	// マトリクス設定
	XMMATRIX world;

	world = ToXMMATRIX(m_Transform.GetWorldMatrix());
	
	Renderer::SetWorldMatrix(world);

	m_pModelRenderer->Draw();
	*/
}

float Bullet::RadianByVector3(Vector3 vect_1, Vector3 vect_2)
{
	if (vect_1.length() == 0 || vect_2.length() == 0) return 0;

	float dot = vect_1.x * vect_2.x + vect_1.y * vect_2.y + vect_1.z * vect_2.z;
	float cos = dot / (vect_1.length() * vect_2.length());
	Clamp(cos, -1.0f, 1.0f);
	return acos(cos);
}