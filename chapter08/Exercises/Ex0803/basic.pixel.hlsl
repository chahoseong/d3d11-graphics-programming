#include "common.hlsli"

Texture2D gDiffuseMap : register(t0);
Texture2D gFlareDiffuseMap : register(t1);
Texture2D gFlareAlphaMap : register(t2);
SamplerState mainSampler : register(s0);

float4 main(VertexOut pin) : SV_TARGET
{
    pin.NormalW = normalize(pin.NormalW);
    
    float3 toEye = gEyePosW - pin.PosW;
    
    float distToEye = length(toEye);
    
    toEye /= distToEye;
    
    float4 texColor = float4(1, 1, 1, 1);
    if (gUseTexture)
    {
        float4 t0 = gDiffuseMap.Sample(mainSampler, pin.Tex);
        float4 t1 = gFlareDiffuseMap.Sample(mainSampler, pin.Tex);
        float4 t2 = gFlareAlphaMap.Sample(mainSampler, pin.Tex);
        texColor = t0 * t1 * t2;
        texColor = saturate(texColor);
    }
    
    float4 litColor = texColor;
    
    if (gLightCount > 0)
    {
        // Lighting
        float ambient = float4(0.0, 0.0, 0.0, 0.0);
        float diffuse = float4(0.0, 0.0, 0.0, 0.0);
        float spec = float4(0.0, 0.0, 0.0, 0.0);
    
        [unroll]
        for (int i = 0; i < gLightCount; ++i)
        {
            float4 A, D, S;
            ComputeDirectionalLight(gMaterial, gDirLights[i], pin.NormalW, toEye, A, D, S);
     
            ambient += A;
            diffuse += D;
            spec += S;
        }
    
        litColor = texColor * (ambient + diffuse) + spec;
    }

    litColor.a = gMaterial.Diffuse.a * texColor.a;
    
    return litColor;
}
