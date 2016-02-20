struct P_IN
{
	float4 posH : SV_POSITION;
	float4 uvsOut : TEXTPOS;
	float4 nrmOut : NORMALS;
	float4 posW : POSITION;
};

texture2D baseTexture : register(t0); // first texture

SamplerState filter : register(s0); // filter 0 using CLAMP, filter 1 using WRAP

cbuffer LIGHT : register(b0)
{
	float4 position;
	float4 direction;
	float4 ratios; // Inner, outer, radius, on or off
	float4 color;
}

// Pixel shader performing multi-texturing with a detail texture on a second UV channel
// A simple optimization would be to pack both UV sets into a single register
float4 main(P_IN input) : SV_TARGET
{
	// Directional Lighting
	float4 lightDir = { 1, 0.5f, -0.5f, 0.5f };
	float4 lightDirColor = { 1, 1, 1, 0.5f };
	float lightDirRatio = saturate(dot(lightDir, normalize(input.nrmOut)));
	float4 lightDirFinalColor = lightDirRatio * lightDirColor;

	// Point Lighting
	float3 pointLightPos = { -1, 0, 0 };
	float pointLightRadius = 20.0f;
	float3 pointLightDir = normalize(pointLightPos - input.posW.xyz);
	float pointLightDirRatio = saturate(dot(pointLightDir, normalize(input.nrmOut.xyz)));
	float4 pointLightDirColor = { 1, 0, 0, 0.5f };
	float pointLightDirAttenuation = 1.0f - saturate(length(pointLightPos - input.posW.xyz) / pointLightRadius);
	float4 pointLightDirFinalColor = pointLightDirRatio * pointLightDirColor * pointLightDirAttenuation;

	// Spotlight
	float3 spotlightPos = position.xyz;
	float4 spotlightColor = color;
	float3 spotlightDir = normalize(spotlightPos - input.posW.xyz);
	float3 coneDir = direction.xyz;
	float coneRatio = ratios.y;
	float spotlightRadius = ratios.z;
	float surfaceRatio = saturate(dot(-spotlightDir, coneDir));
	float spotFactor = (surfaceRatio > coneRatio) ? 1 : 0;
	float spotlightRatio = saturate(dot(spotlightDir, normalize(input.nrmOut.xyz)));
	float spotlightDirAttenuation = 1.0f - saturate((ratios.x - surfaceRatio) / (ratios.x - ratios.y));
	float4 spotlightFinalColor = spotFactor * spotlightRatio * spotlightColor * spotlightDirAttenuation;

	float2 uvs = float2(input.uvsOut.x, input.uvsOut.y);
	float4 baseColor = baseTexture.Sample(filter, uvs); // get base color

	float4 lightColor = lightDirFinalColor;
	lightColor += pointLightDirFinalColor;
	if (ratios.w == 1)
		lightColor += spotlightFinalColor;

	float4 returnColor = baseColor * lightColor;

	return returnColor;
}
