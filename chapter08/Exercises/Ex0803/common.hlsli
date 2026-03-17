struct DirectionalLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float3 Direction;
    float pad;
};

struct PointLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    
    float3 Position;
    float Range;
    
    float3 Att;
    float pad;
};

struct SpotLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    
    float3 Position;
    float Range;
    
    float3 Direction;
    float Spot;
    
    float3 Att;
    float pad;
};

struct Material
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular; // w = SpecPower
    float4 Reflect;
};

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
    int gUseTexture;
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

void ComputeDirectionalLight(Material mat,
                            DirectionalLight L,
                            float3 normal, float3 toEye,
                            out float4 ambient,
                            out float4 diffuse,
                            out float4 spec)
{
    ambient = float4(0.0, 0.0, 0.0, 0.0);
    diffuse = float4(0.0, 0.0, 0.0, 0.0);
    spec = float4(0.0, 0.0, 0.0, 0.0);
    
    float3 lightVec = -L.Direction;
    
    ambient = mat.Ambient * L.Ambient;
    
    float diffuseFactor = dot(lightVec, normal);
    
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0), mat.Specular.w);
        
        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        spec = specFactor * mat.Specular * L.Specular;
    }
}

void ComputePointLight(Material mat,
                       PointLight L,
                       float3 pos,
                       float3 normal,
                       float3 toEye,
                       out float4 ambient,
                       out float4 diffuse,
                       out float4 spec)
{
    ambient = float4(0.0, 0.0, 0.0, 0.0);
    diffuse = float4(0.0, 0.0, 0.0, 0.0);
    spec = float4(0.0, 0.0, 0.0, 0.0);
    
    float3 lightVec = L.Position - pos;
    
    float d = length(lightVec);
    
    if (d > L.Range)
    {
        return;
    }
    
    lightVec /= d;
    
    ambient = mat.Ambient * L.Ambient;
    
    float diffuseFactor = dot(lightVec, normal);
    
    [flatten]
    if (diffuseFactor > 0.0)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0), mat.Specular.w);
        
        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        spec = specFactor * mat.Specular * L.Specular;
    }
    
    float att = 1.0 / dot(L.Att, float3(1.0, d, d * d));
    
    diffuse *= att;
    spec *= att;
}

void ComputeSpotLight(Material mat,
                    SpotLight L,
                    float3 pos,
                    float3 normal,
                    float3 toEye,
                    out float4 ambient,
                    out float4 diffuse,
                    out float4 spec)
{
    ambient = float4(0.0, 0.0, 0.0, 0.0);
    diffuse = float4(0.0, 0.0, 0.0, 0.0);
    spec = float4(0.0, 0.0, 0.0, 0.0);
    
    float3 lightVec = L.Position - pos;
    
    float d = length(lightVec);
    
    if (d > L.Range)
    {
        return;
    }
    
    lightVec /= d;
    
    ambient = mat.Ambient * L.Ambient;
    
    float diffuseFactor = dot(lightVec, normal);
    
    [flatten]
    if (diffuseFactor > 0.0)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0), mat.Specular.w);
        
        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        spec = specFactor * mat.Specular * L.Specular;
    }
    
    float spot = pow(max(dot(-lightVec, L.Direction), 0.0), L.Spot);
    
    float att = spot / dot(L.Att, float3(1.0, d, d * d));
    
    ambient *= spot;
    diffuse *= att;
    spec *= att;
}