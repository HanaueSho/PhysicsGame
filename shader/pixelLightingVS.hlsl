/*
    pixelLightingVS
    20250514 hanaue sho
*/
#include "common.hlsl"

void main(in VS_IN In, out PS_IN Out)
{
    // ���_�ϊ������i�K�{����j
    matrix wvp;
    wvp = mul(World, View);
    wvp = mul(wvp, Projection);
    Out.Position = mul(In.Position, wvp);
    
    // ���_�@�������[���h�s��ŉ�]������i���_�Ɠ�����]��������j
    float4 worldNormal, normal;
    normal = float4(In.Normal.xyz, 0.0f);
    worldNormal = mul(normal, World);
    worldNormal = normalize(worldNormal);
    Out.Normal = worldNormal;
    
    Out.Diffuse = In.Diffuse; // ���_�̕������̂܂܏o��
    Out.TexCoord = In.TexCoord; // ���_�̕������̂܂܏o��
    
    // ���[���h�ϊ��������_���W���o��
    Out.WorldPosition = mul(In.Position, World);
    
}