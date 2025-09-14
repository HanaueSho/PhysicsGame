/*
    pixelLightingPS
    20250514 hanaue sho
*/
#include "common.hlsl"

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
    // ピクセルの法線を正規化
    float4 normal = normalize(In.Normal);
    float light = -dot(Light.Direction.xyz, normal.xyz);
    light = saturate(light);
    
    // テクスチャのピクセルの色を取得
    outDiffuse = g_Texture.Sample(g_SamplerState, In.TexCoord);
    outDiffuse.rgb *= In.Diffuse.rgb * light;
    outDiffuse.a *= In.Diffuse.a; // αに明るさは関係ないので別計算
    
    // 視線ベクトルの作成
    float3 eyev = In.WorldPosition.xyz - CameraPosition.xyz;
    eyev = normalize(eyev);
    
    // 光の反射ベクトルを計算
    float3 refv = reflect(Light.Direction.xyz, normal.xyz);
    refv = normalize(refv);
    
    // 反射ベクトルと視線ベクトルの成す角を考える
    float specular = -dot(eyev, refv); // 鏡面反射の計算
    specular = saturate(specular); // 値をサチュレート
    specular = pow(specular, 10); // 鏡面反射の値を飽和計算する
    
    // ランバート反射の結果へ加算する
    outDiffuse.rgb += specular; // スペキュラ値をデフューズとして足しこむ
}
