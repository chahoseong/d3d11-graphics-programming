cbuffer cbPerFrame : register(b0)
{
    float gTime;
    float3 padding; // 16바이트 정렬을 맞추기 위해 추가
};

cbuffer cbPerObject : register(b1)
{
    float4x4 gWorldViewProj;
};

struct VertexIn
{
    float3 PosL : POSITION;
    float4 Color : COLOR;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float4 Color : COLOR;
};

VertexOut main(VertexIn vin)
{
    VertexOut vout;
    
    vin.PosL.xy += 0.5f * sin(vin.PosL.x) * sin(3.0f * gTime);
    vin.PosL.z *= 0.6f + 0.4f * sin(2.0f * gTime);
    
    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
    
    vout.Color = vin.Color;
    
    return vout;
}
