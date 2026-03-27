cbuffer cbPerFrame : register(b0)
{
    float4 gColor;
    float4x4 gProjection;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
};

float4 main(VertexOut pin) : SV_TARGET
{
    return gColor;
}
