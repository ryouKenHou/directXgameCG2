#include "Object3d.hlsli"

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOuput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOuput main(VertexShaderOutput input)
{
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
    PixelShaderOuput output;
    
    if (gMaterial.enableLighting != 0) {
        float cos = saturate(dot(input.normal, -gDirectionalLight.direction));
        output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
    }
    else {
        output.color = gMaterial.color * textureColor;
    }
    
    
    
    return output;
}