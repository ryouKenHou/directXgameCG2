struct Material
{
    float32_t4 color;
};

ConstantBuffer<Material> gMaterial : register(b0);

struct PixelShaderOuput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOuput main()
{
    PixelShaderOuput output;
    output.color = gMaterial.color;
    return output;
}