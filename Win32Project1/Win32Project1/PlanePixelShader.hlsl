struct P_IN
{
	float4 posH : SV_POSITION;
	float4 uvsOut : TEXTPOS;
	float4 nrmOut : NORMALS;
};

texture2D baseTexture : register(t0); // first texture

SamplerState filter : register(s0); // filter 0 using CLAMP, filter 1 using WRAP

// Pixel shader performing multi-texturing with a detail texture on a second UV channel
// A simple optimization would be to pack both UV sets into a single register
float4 main(P_IN input) : SV_TARGET
{
	float2 uvs = float2(input.uvsOut.x, input.uvsOut.y);
	float4 baseColor = baseTexture.Sample(filter, uvs); // get base color

	float4 returnColor;
	returnColor.a = baseColor.a;
	returnColor.r = baseColor.r;
	returnColor.g = baseColor.g;
	returnColor.b = baseColor.b;

	return returnColor;
}
