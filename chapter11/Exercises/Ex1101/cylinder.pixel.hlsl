#include "cylinder.shared.hlsli"

float4 main(GeoOut pin) : SV_TARGET
{
    pin.Normal = normalize(pin.Normal);
    
    float3 toEye = gEyePosW - pin.PosW;
    
    float distToEye = length(toEye);
    
    toEye /= distToEye;
    
    float4 litColor = float4(0.0, 0.0, 0.0, 1.0);
    
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
            ComputeDirectionalLight(gMaterial, gDirLights[i], pin.Normal, toEye, A, D, S);
     
            ambient += A;
            diffuse += D;
            spec += S;
        }
    
        litColor = ambient + diffuse + spec;
    }
    
    //float fogLerp = saturate((distToEye - gFogStart) / gFogRange);
    //litColor = lerp(litColor, gFogColor, fogLerp);

    litColor.a = gMaterial.Diffuse.a;
    
    return litColor;
}
