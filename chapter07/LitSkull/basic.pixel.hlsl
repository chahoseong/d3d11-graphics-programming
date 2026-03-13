#include "common.hlsli"

float4 main(VertexOut pin) : SV_TARGET
{
    pin.NormalW = normalize(pin.NormalW);
    
    float3 toEye = gEyePosW - pin.PosW;
    
    float distToEye = length(toEye);
    
    toEye /= distToEye;
    
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
    
    float4 litColor = ambient + diffuse + spec;

    litColor.a = gMaterial.Diffuse.a;
    
    return litColor;
}
