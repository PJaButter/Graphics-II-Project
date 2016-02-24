#include "NormalMappedLoadedModel3D.h"
#include "NormalMappedVertexShader.csh"
#include "NormalMappedPixelShader.csh"
#include "SkyBoxPixelShader.csh"
#include "DDSTextureLoader.h"

#define SAFE_RELEASE(p) { if(p) {p->Release(); p = nullptr;}}

NormalMappedLoadedModel3D::NormalMappedLoadedModel3D()
{
	worldMatrix = XMMatrixIdentity();
}


NormalMappedLoadedModel3D::~NormalMappedLoadedModel3D()
{
	SAFE_RELEASE(buffer);
	SAFE_RELEASE(vertexShader);
	SAFE_RELEASE(pixelShader);
	SAFE_RELEASE(layout);
	SAFE_RELEASE(indexBuffer);
	for (int i = 0; i < NUM_SHADER_RESOURCE_VIEWS; ++i)
		SAFE_RELEASE(shaderResourceViews[i]);
	SAFE_RELEASE(sampler);
	SAFE_RELEASE(blendState);
	for (int i = 0; i < NUM_RASTER_STATES; i++)
		SAFE_RELEASE(rasterizerStates[i]);
	delete[] verticies;
	delete[] indicies;
}

void NormalMappedLoadedModel3D::Initialize(ID3D11Device* device, float initX, float initY, float initZ, const wchar_t* textureFilename, const wchar_t* normalMapFilename, const char* modelFilename)
{
	worldMatrix = XMMatrixIdentity();
	worldMatrix = XMMatrixTranslation(initX, initY, initZ);

	HRESULT result = CreateDDSTextureFromFile(device, textureFilename, nullptr, &shaderResourceViews[0]);
	result = CreateDDSTextureFromFile(device, normalMapFilename, nullptr, &shaderResourceViews[1]);

	loadOBJ(modelFilename);

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(Vertex)* numVerticies;

	D3D11_SUBRESOURCE_DATA subresourceDesc;
	subresourceDesc.pSysMem = verticies;

	result = device->CreateBuffer(&bufferDesc, &subresourceDesc, &buffer);

	result = device->CreateVertexShader(NormalMappedVertexShader, sizeof(NormalMappedVertexShader), NULL, &vertexShader);
	result = device->CreatePixelShader(NormalMappedPixelShader, sizeof(NormalMappedPixelShader), NULL, &pixelShader);

	D3D11_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXTPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMALS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	result = device->CreateSamplerState(&samplerDesc, &sampler);

	result = device->CreateInputLayout(inputLayout, 4, NormalMappedVertexShader, sizeof(NormalMappedVertexShader), &layout);

	toObject.worldMatrix = worldMatrix;

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.ByteWidth = sizeof(unsigned int) * numIndicies;

	D3D11_SUBRESOURCE_DATA indexInitData;
	indexInitData.pSysMem = indicies;

	result = device->CreateBuffer(&indexBufferDesc, &indexInitData, &indexBuffer);

	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = true;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	result = device->CreateBlendState(&blendDesc, &blendState);

	D3D11_RASTERIZER_DESC rasterDesc1 = {};
	rasterDesc1.AntialiasedLineEnable = false;
	rasterDesc1.FillMode = D3D11_FILL_SOLID;
	rasterDesc1.CullMode = D3D11_CULL_BACK;
	result = device->CreateRasterizerState(&rasterDesc1, &rasterizerStates[0]);

	rasterDesc1.CullMode = D3D11_CULL_FRONT;
	result = device->CreateRasterizerState(&rasterDesc1, &rasterizerStates[1]);
}

void NormalMappedLoadedModel3D::Run(ID3D11DeviceContext* deviceContext)
{
	deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	unsigned int vertexSize = sizeof(Vertex);
	unsigned int offset = 0;


	deviceContext->IASetVertexBuffers(0, 1, &buffer, &vertexSize, &offset);
	deviceContext->VSSetShader(vertexShader, NULL, 0);
	deviceContext->PSSetShader(pixelShader, NULL, 0);
	deviceContext->IASetInputLayout(layout);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->PSSetShaderResources(0, ARRAYSIZE(shaderResourceViews), shaderResourceViews);
	deviceContext->PSSetSamplers(0, 1, &sampler);
	deviceContext->GSSetShader(nullptr, nullptr, 0);
	deviceContext->OMSetBlendState(blendState, NULL, 0xffffffff);
	deviceContext->RSSetState(rasterizerStates[1]);
	deviceContext->DrawIndexed(numIndicies, 0, 0);
	deviceContext->RSSetState(rasterizerStates[0]);
	deviceContext->DrawIndexed(numIndicies, 0, 0);
	deviceContext->RSSetState(nullptr);
	deviceContext->OMSetBlendState(NULL, NULL, 0xffffffff);
}

void NormalMappedLoadedModel3D::Translate(float offsetX, float offsetY, float offsetZ)
{
	worldMatrix = XMMatrixTranslation(offsetX, offsetY, offsetZ);
}

XMMATRIX NormalMappedLoadedModel3D::GetWorldMatrix()
{
	return worldMatrix;
}

ID3D11Buffer* NormalMappedLoadedModel3D::GetBuffer() const
{
	return buffer;
}

ID3D11Buffer* NormalMappedLoadedModel3D::GetIndexBuffer() const
{
	return indexBuffer;
}

unsigned int NormalMappedLoadedModel3D::GetNumIndicies() const
{
	return numIndicies;
}

ID3D11VertexShader* NormalMappedLoadedModel3D::GetVertexShader() const
{
	return vertexShader;
}

ID3D11PixelShader* NormalMappedLoadedModel3D::GetPixelShader() const
{
	return pixelShader;
}

ID3D11InputLayout* NormalMappedLoadedModel3D::GetLayout() const
{
	return layout;
}

ID3D11SamplerState* NormalMappedLoadedModel3D::GetSampler() const
{
	return sampler;
}

void NormalMappedLoadedModel3D::SetWorldMatrix(const XMMATRIX* matrix)
{
	worldMatrix = *matrix;
}

bool NormalMappedLoadedModel3D::loadOBJ(const char * filename)
{
	vector<XMFLOAT3> pos;
	vector<XMFLOAT2> uvs;
	vector<XMFLOAT3> nrms;

	vector<unsigned int> pos_ind;
	vector<unsigned int> uv_ind;
	vector<unsigned int> nrm_ind;

	ifstream infile;
	char input, input2;
	ofstream fout;

	// Open the file.
	infile.open(filename);

	// Check if it was successful in opening the file.
	if (!infile.is_open())
		return false;

	// Read in the vertices, texture coordinates, and normals into the data structures.
	// Important: Also convert to left hand coordinate system since Maya uses right hand coordinate system.
	infile.get(input);
	while (!infile.eof())
	{
		if (input == 'v')
		{
			infile.get(input);

			// Read in the vertices.
			if (input == ' ')
			{
				XMFLOAT3 temp;
				infile >> temp.x >> temp.y >> temp.z;

				// Invert the Z vertex to change to left hand system.
				temp.z *= -1.0f;
				pos.push_back(temp);
			}

			// Read in the texture uv coordinates.
			if (input == 't')
			{
				XMFLOAT2 temp;
				infile >> temp.x >> temp.y;

				// Invert the V texture coordinates to left hand system.
				temp.y = 1.0f - temp.y;
				uvs.push_back(temp);
			}

			// Read in the normals.
			if (input == 'n')
			{
				XMFLOAT3 temp;
				infile >> temp.x >> temp.y >> temp.z;

				// Invert the Z normal to change to left hand system.
				temp.z *= -1.0f;
				nrms.push_back(temp);
			}
		}

		// Read in the faces.
		if (input == 'f')
		{
			infile.get(input);
			if (input == ' ')
			{
				XMUINT3 temp1, temp2, temp3;
				// Read the face data in backwards to convert it to a left hand system from right hand system.
				infile >> temp1.x >> input2 >> temp1.y >> input2 >> temp1.z
					>> temp2.x >> input2 >> temp2.y >> input2 >> temp2.z
					>> temp3.x >> input2 >> temp3.y >> input2 >> temp3.z;
				pos_ind.push_back(temp3.x);
				pos_ind.push_back(temp2.x);
				pos_ind.push_back(temp1.x);

				uv_ind.push_back(temp3.y);
				uv_ind.push_back(temp2.y);
				uv_ind.push_back(temp1.y);

				nrm_ind.push_back(temp3.z);
				nrm_ind.push_back(temp2.z);
				nrm_ind.push_back(temp1.z);
			}
		}

		// Read in the remainder of the line.
		while (input != '\n')
		{
			infile.get(input);
		}

		// Start reading the beginning of the next line.
		infile.get(input);
	}

	// Close the file.
	infile.close();

	numVerticies = (unsigned int)pos_ind.size();
	verticies = new Vertex[numVerticies];
	numIndicies = (unsigned int)numVerticies;
	indicies = new unsigned int[numIndicies];

	// Now loop through all the faces and output the three vertices for each face.
	for (int i = 0; i < (int)pos_ind.size(); i++)
	{
		verticies[i].pos = pos[pos_ind[i] - 1];
		verticies[i].uvw.x = uvs[uv_ind[i] - 1].x;
		verticies[i].uvw.y = uvs[uv_ind[i] - 1].y;
		verticies[i].nrm = nrms[nrm_ind[i] - 1];
		indicies[i] = i;
	}


	for (int i = 0; i < numVerticies; i += 3)
	{
		Vertex tempVert1 = verticies[i];
		Vertex tempVert2 = verticies[i + 1];
		Vertex tempVert3 = verticies[i + 2];
		Vertex edge0, edge1;

		edge0.pos.x = tempVert2.pos.x - tempVert1.pos.x;
		edge0.pos.y = tempVert2.pos.y - tempVert1.pos.y;
		edge0.pos.z = tempVert2.pos.z - tempVert1.pos.z;

		edge1.pos.x = tempVert3.pos.x - tempVert1.pos.x;
		edge1.pos.y = tempVert3.pos.y - tempVert1.pos.y;
		edge1.pos.z = tempVert3.pos.z - tempVert1.pos.z;

		edge0.uvw.x = tempVert2.uvw.x - tempVert1.uvw.x;
		edge0.uvw.y = tempVert2.uvw.y - tempVert1.uvw.y;
		edge0.uvw.z = tempVert2.uvw.z - tempVert1.uvw.z;

		edge1.uvw.x = tempVert3.uvw.x - tempVert1.uvw.x;
		edge1.uvw.y = tempVert3.uvw.y - tempVert1.uvw.y;
		edge1.uvw.z = tempVert3.uvw.z - tempVert1.uvw.z;

		float ratio = 1.0f / (edge0.uvw.x * edge1.uvw.y - edge0.uvw.y * edge1.uvw.x);

		XMVECTOR uDir, vDir;
		uDir.m128_f32[0] = (edge1.uvw.y * edge0.pos.x - edge0.uvw.y * edge1.pos.x) * ratio;
		uDir.m128_f32[1] = (edge1.uvw.y * edge0.pos.y - edge0.uvw.y * edge1.pos.y) * ratio;
		uDir.m128_f32[2] = (edge1.uvw.y * edge0.pos.z - edge0.uvw.y * edge1.pos.z) * ratio;

		vDir.m128_f32[0] = (edge0.uvw.x * edge1.pos.x - edge1.uvw.x * edge0.pos.x) * ratio;
		vDir.m128_f32[1] = (edge0.uvw.x * edge1.pos.y - edge1.uvw.x * edge0.pos.y) * ratio;	
		vDir.m128_f32[2] = (edge0.uvw.x * edge1.pos.z - edge1.uvw.x * edge0.pos.z) * ratio;

		uDir = XMVector3Normalize(uDir);
		vDir = XMVector3Normalize(vDir);
		for (int j = 0; j < 3; ++j)
		{
			XMVECTOR normal;
			normal.m128_f32[0] = verticies[i + j].nrm.x;
			normal.m128_f32[1] = verticies[i + j].nrm.y;
			normal.m128_f32[2] = verticies[i + j].nrm.z;
			XMVECTOR dotResult = XMVector3Dot(normal, uDir);

			XMVECTOR result = XMVector3Normalize(uDir - normal * dotResult);
			verticies[i + j].tan.x = result.m128_f32[0];
			verticies[i + j].tan.y = result.m128_f32[1];
			verticies[i + j].tan.z = result.m128_f32[2];

			XMVECTOR cross = XMVector3Cross(normal, uDir);
			XMVECTOR handedness = vDir;

			dotResult = XMVector3Dot(cross, handedness);
			verticies[i + j].tan.w = (dotResult.m128_f32[0] < 0.0f) ? -1.0f : 1.0f;
		}
	}

	return true;
}