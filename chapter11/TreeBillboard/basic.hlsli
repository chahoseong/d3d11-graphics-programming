#include "LightHelper.hlsli"

cbuffer cbPerFrame : register(b0)
{
    DirectionalLight gDirLights[3];
    int gLightCount;
    float3 gEyePosW;
    
    float gFogStart;
    float gFogRange;
    float4 gFogColor;
};

cbuffer cbPerObject : register(b1)
{
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
    float4x4 gWorldViewProj;
    float4x4 gTexTransform;
    Material gMaterial;
};

cbuffer cbPerOption : register(b2)
{
    int gUseTexture;
    int gFogEnable;
    int gAlphaClip;
};

struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 Tex : TEXCOORD;
};