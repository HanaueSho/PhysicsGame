/*
	MathConversion.h
	20250810  hanaue sho
    Matrix4x4��XMMATRIX�̕ϊ��p�w�b�_�[
*/
#ifndef MATHCONVERSION_H_
#define MATHCONVERSION_H_
#include <DirectXMath.h>
#include "Matrix4x4.h"

// Matrix4x4 �� XMMATRIX �֕ϊ�
inline DirectX::XMMATRIX ToXMMATRIX(const Matrix4x4& mat)
{
    DirectX::XMFLOAT4X4 temp; 
    Matrix4x4 rowMajor = mat; // �����œ]�u����
    static_assert(sizeof(temp) == sizeof(rowMajor), "Matrix size mismatch");
    memcpy(&temp, &rowMajor, sizeof(temp));
    return DirectX::XMLoadFloat4x4(&temp);
}




#endif