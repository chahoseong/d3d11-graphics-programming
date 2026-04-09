#include "TreeSprite.hlsli"

VertexOut main(VertexIn vin)
{
    VertexOut vout;
    
    vout.CenterW = vin.PosW;
    vout.SizeW = vin.SizeW;
    
    return vout;
}
