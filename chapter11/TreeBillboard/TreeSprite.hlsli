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
    float4x4 gViewProj;
    Material gMaterial;
};

cbuffer cbPerOptions : register(b2)
{
    int gUseTexture;
    int gFogEnable;
    int gAlphaClip;
};

struct VertexIn
{
    float3 PosW : POSITION;
    float2 SizeW : SIZE;
};

struct VertexOut
{
    float3 CenterW : POSITION;
    float2 SizeW : SIZE;
};

struct GeoOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 Tex : TEXCOORD;
    uint PrimID : SV_PrimitiveID;
};
