#include "cylinder.shared.hlsli"

static const int sliceCount = 8;
static const int stackCount = 8;
static const float pi = 3.141592;

void BuildCylinderTopCap(float height,
                        float radius,
                        uint primID,
                        inout TriangleStream<GeoOut> outStream)
{
    float y = 0.5 * height;
    float theta = 2.0 * pi / sliceCount;
    
    for (int i = 0; i <= sliceCount; ++i)
    {
        float x = radius * cos(i * theta);
        float z = radius * sin(i * theta);
        
        float3 pos = float3(x, y, z);
        
        GeoOut gout;
        gout.PosH = mul(float4(pos, 1.0), gWorldViewProj);
        gout.PosW = mul(float4(pos, 1.0), gWorld);
        gout.Normal = mul(float3(0.0, 1.0, 0.0), (float3x3) gWorldInvTranspose);
        gout.PrimID = primID;
        
        outStream.Append(gout);
    }

    outStream.RestartStrip();

}

void BuildCylinderBottomCap(float height,
                        float radius,
                        uint primID,
                        inout TriangleStream<GeoOut> outStream)
{
    float y = -0.5 * height;
    float theta = 2.0 * pi / sliceCount;
    
    for (int i = 0; i <= sliceCount; ++i)
    {
        float x = radius * cos(i * theta);
        float z = radius * sin(i * theta);
        
        float3 pos = float3(x, y, z);
        
        GeoOut gout;
        gout.PosH = mul(float4(pos, 1.0), gWorldViewProj);
        gout.PosW = mul(float4(pos, 1.0), gWorld);
        gout.Normal = mul(float3(0.0, -1.0, 0.0), (float3x3) gWorldInvTranspose);
        gout.PrimID = primID;
        
        outStream.Append(gout);
    }
    
    outStream.RestartStrip();
}


[maxvertexcount(24)]
void main(point VertexOut gin[1],
            uint primID : SV_PrimitiveID,
            inout TriangleStream<GeoOut> outStream)
{
    if (primID < stackCount)
    {
        float stackHeight = gin[0].SizeW.y / stackCount;
        int ringCount = stackCount + 1;
    }
    else if (primID == stackCount)
    {
        
    }
    else if (primID == stackCount + 1)
    {
        
    }
    
        for (int i = 0; i < ringCount; ++i)
        {
            float y = -0.5f * gin[0].SizeW.y + i * stackHeight;
            float r = gin[0].SizeW.x;
            float theta = 2.0 * pi / sliceCount;
        
            for (int j = 0; j <= sliceCount; ++j)
            {
                GeoOut gout;
                float c = cos(j * theta);
                float s = sin(j * theta);
            
                float3 pos = float3(r * c, y, r * s);
                gout.PosH = mul(float4(pos, 1.0), gWorldViewProj);
                gout.PosW = mul(float4(pos, 1.0), gWorld);
            
                float3 tangentU = float3(-s, 0.0, c);
                float3 bitangent = float3(0.0, -gin[0].SizeW.y, 0.0);
                float3 normal = cross(tangentU, bitangent);
                gout.Normal = mul(normalize(normal), (float3x3) gWorldInvTranspose);
            
                gout.PrimID = primID;

                outStream.Append(gout);
            }
        
            outStream.RestartStrip();
        }

    BuildCylinderTopCap(gin[0].SizeW.y, gin[0].SizeW.x, primID, outStream);
    BuildCylinderBottomCap(gin[0].SizeW.y, gin[0].SizeW.x, primID, outStream);
}
