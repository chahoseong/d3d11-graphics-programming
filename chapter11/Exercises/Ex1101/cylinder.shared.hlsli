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
    Material gMaterial;
};

struct VertexIn
{
    float3 PosL : POSITION;
    float2 SizeW : SIZE;
};

struct VertexOut
{
    float3 CenterW : POSITION;
    float2 SizeW : SIZE;
};

struct GeoOut
{
    float4 PosH : SV_Position;
    float3 PosW : POSITION;
    float3 Normal : NORMAL;
    uint PrimID : SV_PrimitiveID;
};