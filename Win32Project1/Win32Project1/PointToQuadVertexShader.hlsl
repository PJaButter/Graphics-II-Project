struct V_IN
{
	float4 posL : SV_POSITION;
	float4 colorIn : COLOR;
};

struct V_OUT
{
	float4 posH : SV_POSITION;
	float4 colorOut : COLOR;
};

V_OUT main( V_IN input )
{
	V_OUT output = (V_OUT)0;
	output.posH = input.posL;
	output.colorOut = input.colorIn;
	return output;
}