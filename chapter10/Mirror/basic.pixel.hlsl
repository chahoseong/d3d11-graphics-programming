#include "common.hlsli"

Texture2D gDiffuseMap : register(t0);
SamplerState anisotropicSS : register(s0);

float4 main(VertexOut pin) : SV_TARGET
{
    pin.NormalW = normalize(pin.NormalW);
    
    float3 toEye = gEyePosW - pin.PosW;
    
    float distToEye = length(toEye);
    
    toEye /= distToEye;
    
    float4 texColor = float4(1, 1, 1, 1);
    if (gUseTexture)
    {
        texColor = gDiffuseMap.Sample(anisotropicSS, pin.Tex);
        
        if (gAlphaClip)
        {
            clip(texColor.a - 0.1);
        }
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
    
    if (gFogEnable)
    {
        float fogLerp = saturate((distToEye - gFogStart) / gFogRange);
        litColor = lerp(litColor, gFogColor, fogLerp);
    }

    litColor.a = gMaterial.Diffuse.a * texColor.a;
    
    return litColor;
}
