#include"resources/shaders/Test.hlsli"

Texture2D<float32_t4> gTexture : register(t0); //SRV register => t
SamplerState gSampler : register(s0); //Sampler register => s

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    float32_t2 uv = input.texcoord;
    float32_t4 textureColor = gTexture.Sample(gSampler, uv);
    
    ////位置セット(x y z w)かカラーセット(r g b a)でアクセスできる
    output.color = textureColor;//non effect
    //output.color.a = 1.0f; // Set alpha to 1.0 (fully opaque)
    return output;
}


//float4 main() : SV_TARGET
//{
//	return float4(1.0f, 1.0f, 1.0f, 1.0f);
//}