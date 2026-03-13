#include "common.hlsli"

float4 main(VertexOut pin) : SV_TARGET
{
    // 보간(Interpolating) 때문에 normal의 정규화가 깨질 수 있습니다.
    pin.NormalW = normalize(pin.NormalW);
    
    float3 toEyeW = normalize(gEyePosW - pin.PosW);
    
    float4 ambient = float4(0.0, 0.0, 0.0, 0.0);
    float4 diffuse = float4(0.0, 0.0, 0.0, 0.0);
    float4 spec = float4(0.0, 0.0, 0.0, 0.0);
    
    float4 A, D, S;
    
    ComputeDirectionalLight(gMaterial, gDirLight, pin.NormalW, toEyeW, A, D, S);
    ambient += A;
    diffuse += D;
    spec += S;
    
    ComputePointLight(gMaterial, gPointLight, pin.PosW, pin.NormalW, toEyeW, A, D, S);
    ambient += A;
    diffuse += D;
    spec += S;
    
    ComputeSpotLight(gMaterial, gSpotLight, pin.PosW, pin.NormalW, toEyeW, A, D, S);
    ambient += A;
    diffuse += D;
    spec += S;
    
    float4 litColor = ambient + diffuse + spec;
    
    litColor.a = gMaterial.Diffuse.a;
    
    return litColor;
}