/*
	MeshFilterComponent.h
	20250813  hanaue sho
	�W�I���g���i�`��j�̕ێ���
	�h����`�����h�Ƃ����������i���_�A�C���f�b�N�X�A�g�|���W etc.�j
*/
#ifndef MESHFILTERCOMPONENT_H_
#define MESHFILTERCOMPONENT_H_
#include "Component.h"
#include <d3d11.h>

class MeshFilterComponent : public Component
{
protected:
	ID3D11Buffer* m_VertexBuffer = nullptr;
	UINT		  m_Stride		= 0;
	UINT		  m_Offset		= 0;
	UINT		  m_VertexCount = 0;

	ID3D11Buffer* m_IndexBuffer = nullptr;
	DXGI_FORMAT	  m_IndexFormat = DXGI_FORMAT_R32_UINT;
	UINT		  m_IndexCount  = 0;

	D3D11_PRIMITIVE_TOPOLOGY m_Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// ���L��
	bool m_OwnsVB = false;
	bool m_OwnsIB = false;

public:
	void SetVertexBuffer(ID3D11Buffer* vb, UINT stride, UINT vertexCount, UINT offset = 0, bool takeOwnership = false) noexcept
	{ m_VertexBuffer = vb; m_Stride = stride; m_VertexCount = vertexCount; m_Offset = offset; m_OwnsVB = takeOwnership; }

	void SetIndexBuffer(ID3D11Buffer* ib, UINT indexCount, DXGI_FORMAT format = DXGI_FORMAT_R32_UINT, bool takeOwnership = false) noexcept
	{ m_IndexBuffer = ib; m_IndexCount = indexCount; m_IndexFormat = format; m_OwnsIB = takeOwnership; }
	
	void SetTopology(D3D11_PRIMITIVE_TOPOLOGY topo) noexcept
	{ m_Topology = topo; }

	// �A�N�Z�X
	ID3D11Buffer* VertexBuffer() const noexcept  { return m_VertexBuffer; }
	ID3D11Buffer* IndexBuffer()  const noexcept  { return m_IndexBuffer; }
	UINT		  Stride()		 const noexcept  { return m_Stride; }
	UINT		  Offset()		 const noexcept  { return m_Offset; }
	UINT		  VertexCount()	 const noexcept  { return m_VertexCount; }
	UINT		  IndexCount()	 const noexcept  { return m_IndexCount; }
	DXGI_FORMAT	  IndexFormat()	 const noexcept  { return m_IndexFormat; }
	D3D11_PRIMITIVE_TOPOLOGY Topology() const noexcept { return m_Topology; }

	// �Ó���
	bool IsIndexed() const noexcept { return m_IndexBuffer != nullptr && m_IndexCount > 0; }

	bool IsReady() const noexcept
	{
		if (!m_VertexBuffer || m_Stride == 0) return false;
		if (IsIndexed())
		{
			if (m_IndexFormat != DXGI_FORMAT_R16_UINT && m_IndexFormat != DXGI_FORMAT_R32_UINT) return false;
		}
		else
		{
			if (m_VertexCount == 0) return false;
		}
		return true;
	}

	// �j���i���L���̂݁j
	void Uninit() override
	{
		if (m_OwnsVB && m_VertexBuffer) { m_VertexBuffer->Release(); m_VertexBuffer = nullptr; }
		if (m_OwnsIB && m_IndexBuffer) { m_IndexBuffer->Release(); m_IndexBuffer = nullptr; }
	}

	// �O�����������Ǘ�����ꍇ�̃��Z�b�g
	void ResetNonOwning() noexcept
	{
		m_VertexBuffer = nullptr; m_IndexBuffer = nullptr;
		m_Stride = m_Offset = m_VertexCount = m_IndexCount = 0;
		m_IndexFormat = DXGI_FORMAT_R32_UINT;
		m_Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		m_OwnsVB = m_OwnsIB = false;
	}
};

#endif