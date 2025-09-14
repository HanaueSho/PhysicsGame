/*
    pixelLightingPS
    20250514 hanaue sho
*/
#include "common.hlsl"

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
    // �s�N�Z���̖@���𐳋K��
    float4 normal = normalize(In.Normal);
    float light = -dot(Light.Direction.xyz, normal.xyz);
    light = saturate(light);
    
    // �e�N�X�`���̃s�N�Z���̐F���擾
    outDiffuse = g_Texture.Sample(g_SamplerState, In.TexCoord);
    outDiffuse.rgb *= In.Diffuse.rgb * light;
    outDiffuse.a *= In.Diffuse.a; // ���ɖ��邳�͊֌W�Ȃ��̂ŕʌv�Z
    
    // �����x�N�g���̍쐬
    float3 eyev = In.WorldPosition.xyz - CameraPosition.xyz;
    eyev = normalize(eyev);
    
    // ���̔��˃x�N�g�����v�Z
    float3 refv = reflect(Light.Direction.xyz, normal.xyz);
    refv = normalize(refv);
    
    // ���˃x�N�g���Ǝ����x�N�g���̐����p���l����
    float specular = -dot(eyev, refv); // ���ʔ��˂̌v�Z
    specular = saturate(specular); // �l���T�`�����[�g
    specular = pow(specular, 10); // ���ʔ��˂̒l��O�a�v�Z����
    
    // �����o�[�g���˂̌��ʂ։��Z����
    outDiffuse.rgb += specular; // �X�y�L�����l���f�t���[�Y�Ƃ��đ�������
}
