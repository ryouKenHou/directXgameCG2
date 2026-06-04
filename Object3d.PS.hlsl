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
    float4 transformedUV = mul(float32_t4(input.texcoord,0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    PixelShaderOuput output;
    
    if (gMaterial.enableLighting != 0) {
        float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = saturate(pow(NdotL*0.5f+0.5f, 2.0f ));
        output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
    }
    else {
        output.color = gMaterial.color * textureColor;
    }
    
    
    
    return output;
}