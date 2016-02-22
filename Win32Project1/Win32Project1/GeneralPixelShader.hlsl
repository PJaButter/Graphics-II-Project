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
	float3 color;
	float padding;
}

// Pixel shader performing multi-texturing with a detail texture on a second UV channel
// A simple optimization would be to pack both UV sets into a single register
float4 main(P_IN input) : SV_TARGET
{
	float2 uvs = float2(input.uvsOut.x, input.uvsOut.y);
	float4 baseColor = baseTexture.Sample(filter, uvs); // get base color

	// Directional Lighting
	float4 lightDir = { 1, 0.5f, -0.5f, 0.5f };
	float3 lightDirColor = { 1, 1, 1 };
	float lightDirRatio = saturate(dot(lightDir, normalize(input.nrmOut)));
	float3 lightDirFinalColor = lightDirRatio * lightDirColor * baseColor.xyz;

	// Point Lighting
	float3 pointLightPos = { -1, 0, 0 };
	float pointLightRadius = 20.0f;
	float3 pointLightDir = normalize(pointLightPos - input.posW.xyz);
	float pointLightDirRatio = saturate(dot(pointLightDir, normalize(input.nrmOut.xyz)));
	float3 pointLightDirColor = { 1, 0, 0 };
	float pointLightDirAttenuation = 1.0f - saturate(length(pointLightPos - input.posW.xyz) / pointLightRadius);
	float3 pointLightDirFinalColor = pointLightDirRatio * pointLightDirColor * pointLightDirAttenuation * baseColor.xyz;

	// Spotlight
	float3 spotlightPos = position.xyz;
	float3 spotlightColor = color;
	float3 spotlightDir = normalize(spotlightPos - input.posW.xyz);
	float3 coneDir = direction.xyz;
	float coneRatio = ratios.y;
	float spotlightRadius = ratios.z;
	float surfaceRatio = saturate(dot(-spotlightDir, coneDir));
	float spotFactor = (surfaceRatio > coneRatio) ? 1 : 0;
	float spotlightRatio = saturate(dot(spotlightDir, normalize(input.nrmOut.xyz)));
	
	float3 spotlightViewDir = normalize(position.xyz - input.posW.xyz);
	float3 spotlightHalfVector = normalize((spotlightDir) + spotlightViewDir);
	float spotlightIntensity = max(pow(saturate(dot(input.nrmOut, normalize(spotlightHalfVector))), 32), 0);
	
	float spotlightDirAttenuation = 1.0f - saturate((ratios.x - surfaceRatio) / (ratios.x - ratios.y));
	float3 spotlightFinalColor = spotFactor * spotlightRatio * spotlightColor * spotlightDirAttenuation * baseColor.xyz;
	
	spotlightFinalColor = spotlightFinalColor * 1.0f * spotlightIntensity;

	float3 lightColor = lightDirFinalColor;
	lightColor += pointLightDirFinalColor;
	if (ratios.w == 1)
		lightColor += spotlightFinalColor;

	float4 returnColor = saturate(float4(lightColor, baseColor.a));

	return returnColor;
}
