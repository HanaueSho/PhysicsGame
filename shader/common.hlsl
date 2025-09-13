/*
    common.hlsl
20250423 hanaue sho
*/
// このファイルは他のシェーダーファイルへインクルードされる
// 各種マトリクスやベクトルを受け取る変数を用意

cbuffer WorldBuffer : register(b0) // 定数バッファ０番 行列を受け取る定数バッファ
{
    matrix World; // matrix 型の World へ受け取る
}

cbuffer ViewBuffer : register(b1) // 定数バッファ１番
{
    matrix View;
}

cbuffer ProjectionBuffer : register(b2) // 定数バッファ２番
{
    matrix Projection;
}
struct MATERIAL
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emission;
    float Shininess;
    bool TextureEnable;
    float2 Dummy;
};
    
// 頂点シェーダ―へ入力されるデータを構造体の形で表現
struct VS_IN
{
    float4 Position : POSITION0;
    float4 Normal : NORMAL0;
    float4 Diffuse : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

// ピクシェルシェーダーへ入力されるデータを構造大の形で表現
struct PS_IN
{
    float4 Position : SV_Position;
    float4 WorldPosition : POSITION0;
    float4 Normal : NORMAL0;
    float4 Diffuse : COLOR0;
    float2 TexCoord : TEXCOORD0;
    MATERIAL Material : MATERIAL;
};

struct LIGHT
{
    bool Enable; // bool 型とは言うが内部的には float 型
    bool3 Dummy; // 配置アドレスを既定の倍数にするためのパディング
    float4 Direction; // 実はC言語でも同じで VisualStudio がやってくれている（何を？）
    float4 Diffuse;
    float4 Ambient;
    
    float4 Position; // 光の位置
    float4 PointLightParam; // 光の届く距離
};

cbuffer LightBuffer : register(b4) // コンスタントバッファ４番とする
{
    LIGHT Light; // LIGHT 構造体
}

cbuffer CameraBuffer : register(b5) // バッファの５番とする
{
    float4 CameraPosition; // カメラの座標を受け取る変数
}

cbuffer ParameterBuffer : register(b6)
{
    float4 Parameter; // シェーダー内で使う変数名
}
