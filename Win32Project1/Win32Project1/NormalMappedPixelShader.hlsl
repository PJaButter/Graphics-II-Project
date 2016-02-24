#pragma pack_matrix(row_major)

struct P_IN
{
	float4 posH : SV_POSITION;
	float4 uvsOut : TEXTPOS;
	float4 nrmOut : NORMALS;
	float4 posW : POSITION;
	float4 tanOut : TANGENTS;
	float4 biTansOut : BITANGENTS;
};

texture2D baseTexture : register(t0); // first texture
texture2D normalTexture : register(t1); // Normal Map texture

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
	float3 newNormal = normalTexture.Sample(filter, uvs);

	newNormal = (newNormal * 2.0f) - 1.0f;

	float3x3 TBNMatrix;
	TBNMatrix[0] = normalize(input.tanOut.xyz);
	TBNMatrix[1] = normalize(input.biTansOut.xyz);
	TBNMatrix[2] = normalize(input.nrmOut.xyz);

	newNormal = mul(newNormal, TBNMatrix);

	// Directional Lighting
	float4 lightDir = { 1, 0.5f, -0.5f, 0.5f };
	float3 lightDirColor = { 1, 1, 1 };
	float lightDirRatio = saturate(dot(lightDir, normalize(newNormal.xyz)));
	float3 lightDirFinalColor = lightDirRatio * lightDirColor * baseColor.xyz;

	// Point Lighting
	float3 pointLightPos = { -5, 0, 8 };
	float pointLightRadius = 20.0f;
	float3 pointLightDir = normalize(pointLightPos - input.posW.xyz);
		float pointLightDirRatio = saturate(dot(pointLightDir, normalize(newNormal.xyz)));
	float3 pointLightDirColor = { 1, 0, 0 };
	float pointLightDirAttenuation = 1.0f - saturate(length(pointLightPos - input.posW.xyz) / pointLightRadius);
	float3 pointLightDirFinalColor = pointLightDirRatio * pointLightDirColor * pointLightDirAttenuation * baseColor.xyz;
	float3 toPointLight = normalize(pointLightPos - input.posW.xyz);
	float3 toCamera = normalize(position.xyz - input.posW.xyz);
	float3 pointLightReflection = normalize(reflect(-toPointLight, normalize(newNormal.xyz)));
	float  pointLightSpecRatio = pow(dot(pointLightReflection, toCamera), 256);
	float3 pointLightSpecColor = pointLightDirFinalColor * pointLightSpecRatio * 1.0f;

	// Spotlight
	float3 spotlightPos = position.xyz;
	float3 spotlightColor = color;
	float3 spotlightDir = normalize(spotlightPos - input.posW.xyz);
	float3 coneDir = direction.xyz;
	float coneRatio = ratios.y;
	float spotlightRadius = ratios.z;
	float surfaceRatio = saturate(dot(-spotlightDir, coneDir));
	float spotFactor = (surfaceRatio > coneRatio) ? 1 : 0;
	float spotlightRatio = saturate(dot(spotlightDir, normalize(newNormal.xyz)));

	float spotlightDirAttenuation = 1.0f - saturate((ratios.x - surfaceRatio) / (ratios.x - ratios.y));
	float3 spotlightFinalColor = spotFactor * spotlightRatio * spotlightColor * spotlightDirAttenuation * baseColor.xyz;
	float3 toSpotlight = normalize(spotlightPos - input.posW.xyz);
	float3 spotlightReflection = normalize(reflect(-toSpotlight, normalize(newNormal.xyz)));
	float spotlightSpecRatio = pow(dot(spotlightReflection, toCamera), 256);
	float3 spotlightSpecColor = spotlightFinalColor * spotlightSpecRatio * 1.0f;

	float3 lightColor = lightDirFinalColor;
	lightColor += pointLightDirFinalColor + pointLightSpecColor;
	if (ratios.w == 1)
		lightColor += spotlightFinalColor + spotlightSpecColor;

	float4 returnColor = saturate(float4(lightColor, baseColor.a));

	return returnColor;
}

