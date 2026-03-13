#include "common.hlsli"

VertexOut main(VertexIn vin)
{
    VertexOut vout;
    
    vout.PosW = mul(float4(vin.PosL, 1.0), gWorld);
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorldInvTranspose);
    
    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
    
	return vout;
}