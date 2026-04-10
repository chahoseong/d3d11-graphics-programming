#include "cylinder.shared.hlsli"

VertexOut main(VertexIn vin)
{
    VertexOut vout;
    
    vout.CenterW = vin.PosL;
    vout.SizeW = vin.SizeW;
    
    return vout;
}