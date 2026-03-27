cbuffer cbPerFrame : register(b0)
{
    float4 gColor;
    float4x4 gProjection;
};

struct VertexIn
{
    float3 PosW : POSITION;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
};

VertexOut main(VertexIn vin)
{
    VertexOut vout;
    vout.PosH = float4(vin.PosW, 1.0);
    return vout;
}