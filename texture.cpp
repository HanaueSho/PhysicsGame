#include "texture.h"
#include "main.h"
#include "renderer.h"
#include "manager.h"

std::unordered_map<std::string, ID3D11ShaderResourceView*> Texture::m_TexturePool;

ID3D11ShaderResourceView* Texture::Load(const char* fileName)
{
	if (m_TexturePool.count(fileName) > 0)
	{
		return m_TexturePool[fileName];
	}

	wchar_t wFileName[512];
	mbstowcs(wFileName, fileName, strlen(fileName) + 1);

	TexMetadata metadata;
	ScratchImage image;
	ID3D11ShaderResourceView* pTexture;
	LoadFromWICFile(wFileName, WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(
		Renderer::GetDevice(), 
		image.GetImages(), 
		image.GetImageCount(), 
		metadata, 
		&pTexture);
	
	assert(pTexture);

	m_TexturePool[fileName] = pTexture;

	return pTexture;
}