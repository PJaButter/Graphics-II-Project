struct P_IN
{
	float4 posH : SV_POSITION;
	float4 colorIn : COLOR;
};

float4 main(P_IN input) : SV_TARGET
{
	return input.colorIn;
}