/*
	MeshRendererComponent.h
	20250813  hanaue sho
	”どう描くか”という情報を持つ
*/
#ifndef MESHRENDERERCOMPONENT_H_
#define MESHRENDERERCOMPONENT_H_
#include "Component.h"
#include "TransformComponent.h"
#include "MeshFilterComponent.h"
#include "MaterialComponent.h"
#include "Renderer.h"
#include "GameObject.h"
#include "MathConversion.h" // ToXMMATRIX(const Matrix4x4&)

class MeshRendererComponent : public Component
{
protected:
	TransformComponent*	 m_pTransform = nullptr;
	MeshFilterComponent* m_pMeshFilter = nullptr;
	MaterialComponent*	 m_pMaterial = nullptr;

	bool m_IsDraw = true;
public:
	void SetDraw(bool b) { m_IsDraw = b; }

	void OnAdded() override
	{
		m_pTransform  = Owner()->GetComponent<TransformComponent>();
		m_pMeshFilter = Owner()->GetComponent<MeshFilterComponent>();
		m_pMaterial   = Owner()->GetComponent<MaterialComponent>();
	}

	void Draw() override
	{
		if (!m_IsDraw) return;
		if (!m_pTransform || !m_pMeshFilter || !m_pMaterial) return;
		if (!m_pMaterial->IsReady() || !m_pMeshFilter->IsReady()) return;

		// マテリアル（シェーダ、IL、テクスチャ、定数バッファのMATERIAL）をバインド
		m_pMaterial->Bind();

		// world 行列を Renderer に
		const Matrix4x4& worldMatrix = m_pTransform->WorldMatrix();
		Renderer::SetWorldMatrix(ToXMMATRIX(worldMatrix));

		// Meshをセットして Draw
		auto* ctx = Renderer::GetDeviceContext();
		if (!ctx) return;

		UINT stride = m_pMeshFilter->Stride();
		UINT offset = m_pMeshFilter->Offset();
		ID3D11Buffer* vb = m_pMeshFilter->VertexBuffer();

		ctx->IASetPrimitiveTopology(m_pMeshFilter->Topology());
		ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

		if (m_pMeshFilter->IsIndexed())
		{
			ctx->IASetIndexBuffer(m_pMeshFilter->IndexBuffer(), m_pMeshFilter->IndexFormat(), 0);
			ctx->DrawIndexed(m_pMeshFilter->IndexCount(), 0, 0);
		}
		else // 非インデックス描画（頂点描画）
		{
			ctx->Draw(m_pMeshFilter->VertexCount(), 0);
		}
	}
};

#endif