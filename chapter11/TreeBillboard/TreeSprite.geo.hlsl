#include "TreeSprite.hlsli"

static const float2 gTexC[4] =
{
    float2(0.0f, 1.0f),
    float2(0.0f, 0.0f),
    float2(1.0f, 1.0f),
    float2(1.0f, 0.0f), 
};

[maxvertexcount(4)]
void main(point VertexOut gin[1],
            uint primID : SV_PrimitiveID,
            inout TriangleStream<GeoOut> triStream)
{
    float3 up = float3(0.0, 1.0, 0.0);
    float3 look = gEyePosW - gin[0].CenterW;
    look.y = 0.0;
    look = normalize(look);
    float3 right = cross(up, look);
    
    float halfWidth = 0.5 * gin[0].SizeW.x;
    float halfHeight = 0.5 * gin[0].SizeW.y;
    
    float4 v[4];
    v[0] = float4(gin[0].CenterW + halfWidth * right - halfHeight * up, 1.0);
    v[1] = float4(gin[0].CenterW + halfWidth * right + halfHeight * up, 1.0);
    v[2] = float4(gin[0].CenterW - halfWidth * right - halfHeight * up, 1.0);
    v[3] = float4(gin[0].CenterW - halfWidth * right + halfHeight * up, 1.0);
    
    GeoOut gout;
    
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        gout.PosH = mul(v[i], gViewProj);
        gout.PosW = v[i].xyz;
        gout.NormalW = look;
        gout.Tex = gTexC[i];
        gout.PrimID = primID;
        triStream.Append(gout);
    }
}