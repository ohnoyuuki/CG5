
struct VertexShaderOutput
{
    float position : SV_POSITION;
};

struct VertexShaderInput
{
    float4 position : POSITION0;
};

VertexShaderInput main( VertexShaderInput input )
{
    VertexShaderInput output;
    output.position = input.position;
    return output;
}

//float main( float4 pos : POSITION ) : SV_POSITION
//{
//	return pos;
//}