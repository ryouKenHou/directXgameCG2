struct PixelShaderOuput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOuput main()
{
    PixelShaderOuput output;
    output.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    return output;
}