#include "LoadedModel3D.h"
#include "GeneralVertexShader.csh"
#include "GeneralPixelShader.csh"
#include "SkyBoxPixelShader.csh"
#include "DDSTextureLoader.h"

#define SAFE_RELEASE(p) { if(p) {p->Release(); p = nullptr;}}

LoadedModel3D::LoadedModel3D()
{
	worldMatrix = XMMatrixIdentity();
}


LoadedModel3D::~LoadedModel3D()
{
	SAFE_RELEASE(buffer);
	SAFE_RELEASE(vertexShader);
	SAFE_RELEASE(pixelShader);
	SAFE_RELEASE(layout);
	SAFE_RELEASE(indexBuffer);
	SAFE_RELEASE(shaderResourceView);
	SAFE_RELEASE(sampler);
	SAFE_RELEASE(blendState);
	for (int i = 0; i < NUM_RASTER_STATES; i++)
		SAFE_RELEASE(rasterizerStates[i]);
	delete[] verticies;
	delete[] indicies;
}

void LoadedModel3D::Initialize(ID3D11Device* device, float initX, float initY, float initZ, const wchar_t* textureFilename, const char* modelFilename)
{
	worldMatrix = XMMatrixIdentity();
	worldMatrix = XMMatrixTranslation(initX, initY, initZ);

	HRESULT result = CreateDDSTextureFromFile(device, textureFilename, nullptr, &shaderResourceView);

	loadOBJ(modelFilename);

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(Vertex)* numVerticies;

	D3D11_SUBRESOURCE_DATA subresourceDesc;
	subresourceDesc.pSysMem = verticies;

	result = device->CreateBuffer(&bufferDesc, &subresourceDesc, &buffer);

	result = device->CreateVertexShader(GeneralVertexShader, sizeof(GeneralVertexShader), NULL, &vertexShader);
	result = device->CreatePixelShader(GeneralPixelShader, sizeof(GeneralPixelShader), NULL, &pixelShader);

	D3D11_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXTPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMALS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	result = device->CreateSamplerState(&samplerDesc, &sampler);

	result = device->CreateInputLayout(inputLayout, 3, GeneralVertexShader, sizeof(GeneralVertexShader), &layout);

	toObject.worldMatrix = worldMatrix;

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.ByteWidth = sizeof(unsigned int) * numIndicies;

	D3D11_SUBRESOURCE_DATA indexInitData;
	indexInitData.pSysMem = indicies;

	result = device->CreateBuffer(&indexBufferDesc, &indexInitData, &indexBuffer);

	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = false;
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

void LoadedModel3D::Run(ID3D11DeviceContext* deviceContext)
{
	deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	unsigned int vertexSize = sizeof(Vertex);
	unsigned int offset = 0;


	deviceContext->IASetVertexBuffers(0, 1, &buffer, &vertexSize, &offset);
	deviceContext->VSSetShader(vertexShader, NULL, 0);
	deviceContext->PSSetShader(pixelShader, NULL, 0);
	deviceContext->IASetInputLayout(layout);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->PSSetShaderResources(0, 1, &shaderResourceView);
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

void LoadedModel3D::Translate(float offsetX, float offsetY, float offsetZ)
{
	worldMatrix = XMMatrixTranslation(offsetX, offsetY, offsetZ);
}

XMMATRIX LoadedModel3D::GetWorldMatrix()
{
	return worldMatrix;
}

ID3D11Buffer* LoadedModel3D::GetBuffer() const
{
	return buffer;
}

ID3D11Buffer* LoadedModel3D::GetIndexBuffer() const
{
	return indexBuffer;
}

unsigned int LoadedModel3D::GetNumIndicies() const
{
	return numIndicies;
}

ID3D11VertexShader* LoadedModel3D::GetVertexShader() const
{
	return vertexShader;
}

ID3D11PixelShader* LoadedModel3D::GetPixelShader() const
{
	return pixelShader;
}

ID3D11InputLayout* LoadedModel3D::GetLayout() const
{
	return layout;
}

ID3D11SamplerState* LoadedModel3D::GetSampler() const
{
	return sampler;
}

void LoadedModel3D::SetWorldMatrix(const XMMATRIX* matrix)
{
	worldMatrix = *matrix;
}

bool LoadedModel3D::loadOBJ(const char * filename)
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

	return true;
}