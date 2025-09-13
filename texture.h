#pragma once

#include <unordered_map>
#include <string>
#include "main.h"

class Texture
{
private:
	static std::unordered_map<std::string, ID3D11ShaderResourceView*> m_TexturePool;

public:
	static ID3D11ShaderResourceView* Load(const char* fileName);
};