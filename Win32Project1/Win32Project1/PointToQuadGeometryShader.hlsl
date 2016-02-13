#pragma pack_matrix(row_major)

struct GSInput
{
	float4 posH : SV_POSITION;
	float4 colorIn : COLOR;
};

struct GSOutput
{
	float4 pos : SV_POSITION;
	float4 colorOut: COLOR;
};

cbuffer OBJECT : register(b0)
{
	float4x4 worldMatrix;
}

cbuffer SCENE : register(b1)
{
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
}

[maxvertexcount(6)]
void main(
	point GSInput input[1], 
	inout TriangleStream< GSOutput > output
)
{
	GSOutput verts[4];

	verts[0].pos.x = input[0].posH.x - 1;
	verts[0].pos.y = input[0].posH.y + 1;
	verts[0].pos.z = input[0].posH.z;
	verts[0].pos.w = input[0].posH.w;
	verts[0].colorOut = input[0].colorIn;

	verts[1].pos.x = input[0].posH.x + 1;
	verts[1].pos.y = input[0].posH.y + 1;
	verts[1].pos.z = input[0].posH.z;
	verts[1].pos.w = input[0].posH.w;
	verts[1].colorOut = input[0].colorIn;

	verts[2].pos.x = input[0].posH.x - 1;
	verts[2].pos.y = input[0].posH.y - 1;
	verts[2].pos.z = input[0].posH.z;
	verts[2].pos.w = input[0].posH.w;
	verts[2].colorOut = input[0].colorIn;

	verts[3].pos.x = input[0].posH.x + 1;
	verts[3].pos.y = input[0].posH.y - 1;
	verts[3].pos.z = input[0].posH.z;
	verts[3].pos.w = input[0].posH.w;
	verts[3].colorOut = input[0].colorIn;

	for (uint i = 0; i < 4; i++)
	{
		float4 localH = float4(verts[i].pos);
		// move local space vertex from vertex buffer into world space.
		localH = mul(localH, worldMatrix);

		// Move into view space, then projection space
		localH = mul(localH, viewMatrix);
		localH = mul(localH, projectionMatrix);
		verts[i].pos = localH;
	}
	output.Append(verts[0]);
	output.Append(verts[1]);
	output.Append(verts[3]);
	output.RestartStrip();
	output.Append(verts[0]);
	output.Append(verts[3]);
	output.Append(verts[2]);
	output.RestartStrip();
}