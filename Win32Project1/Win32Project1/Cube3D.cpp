#include "Cube3D.h"
#include "GeneralVertexShader.csh"
#include "GeneralPixelShader.csh"
#include "DDSTextureLoader.h"

#define NUMVERTICIES 24
#define NUMINDICIES 36

#define SAFE_RELEASE(p) { if(p) {p->Release(); p = nullptr;}}

Cube3D::Cube3D()
{
	worldMatrix = XMMatrixIdentity();
	verticies = new Vertex[NUMVERTICIES];
}


Cube3D::~Cube3D()
{
	SAFE_RELEASE(buffer);
	SAFE_RELEASE(vertexShader);
	SAFE_RELEASE(pixelShader);
	SAFE_RELEASE(layout);
	SAFE_RELEASE(indexBuffer);
	SAFE_RELEASE(shaderResourceView);
	SAFE_RELEASE(sampler);
	delete[] verticies;
}

void Cube3D::Initialize(ID3D11Device* device, float initX, float initY, float initZ, const wchar_t* filename) 
{
	worldMatrix = XMMatrixIdentity();
	worldMatrix = XMMatrixTranslation(initX, initY, initZ);
	numIndicies = NUMINDICIES;
	CreateVerticies();

	HRESULT result = CreateDDSTextureFromFile(device, filename, nullptr, &shaderResourceView);

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(Vertex)* NUMINDICIES;

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

	unsigned int tempIndicies[NUMINDICIES] =
	{
		0, 1, 2, 1, 3, 2,
		5, 6, 7, 5, 4, 6,
		8, 11, 10, 9, 11, 8,
		12, 13, 14, 13, 15, 14,
		16, 17, 18, 17, 19, 18,
		20, 21, 23, 23, 22, 20
	};

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.ByteWidth = sizeof(tempIndicies);

	D3D11_SUBRESOURCE_DATA indexInitData;
	indexInitData.pSysMem = tempIndicies;

	result = device->CreateBuffer(&indexBufferDesc, &indexInitData, &indexBuffer);
}

void Cube3D::Run(ID3D11DeviceContext* deviceContext)
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
	deviceContext->DrawIndexed(numIndicies, 0, 0);
}

void Cube3D::Translate(float offsetX, float offsetY, float offsetZ)
{
	worldMatrix = XMMatrixTranslation(offsetX, offsetY, offsetZ);
}

XMMATRIX Cube3D::GetWorldMatrix()
{
	return worldMatrix;
}

ID3D11Buffer* Cube3D::GetBuffer() const
{
	return buffer;
}

ID3D11Buffer* Cube3D::GetIndexBuffer() const
{
	return indexBuffer;
}

unsigned int Cube3D::GetNumIndicies() const
{
	return numIndicies;
}

ID3D11VertexShader* Cube3D::GetVertexShader() const
{
	return vertexShader;
}

ID3D11PixelShader* Cube3D::GetPixelShader() const
{
	return pixelShader;
}

ID3D11InputLayout* Cube3D::GetLayout() const
{
	return layout;
}

ID3D11SamplerState* Cube3D::GetSampler() const
{
	return sampler;
}

void Cube3D::SetWorldMatrix(const XMMATRIX* matrix)
{
	worldMatrix = *matrix;
}

// Private Member Functions
void Cube3D::CreateVerticies()
{
	// Front Back
	verticies[0].pos.x = -0.5f;
	verticies[0].pos.y = 0.5f;
	verticies[0].pos.z = -0.5f;
	verticies[0].uvw.x = 0;
	verticies[0].uvw.y = 0;
	verticies[0].uvw.z = 0;
	verticies[0].nrm.x = 0;
	verticies[0].nrm.y = 0;
	verticies[0].nrm.z = -1;

	verticies[1].pos.x = 0.5f;
	verticies[1].pos.y = 0.5f;
	verticies[1].pos.z = -0.5f;
	verticies[1].uvw.x = 1;
	verticies[1].uvw.y = 0;
	verticies[1].uvw.z = 0;
	verticies[1].nrm = verticies[0].nrm;

	verticies[2].pos.x = -0.5f;
	verticies[2].pos.y = -0.5f;
	verticies[2].pos.z = -0.5f;
	verticies[2].uvw.x = 0;
	verticies[2].uvw.y = 1;
	verticies[2].uvw.z = 0;
	verticies[2].nrm = verticies[0].nrm;

	verticies[3].pos.x = 0.5f;
	verticies[3].pos.y = -0.5f;
	verticies[3].pos.z = -0.5f;
	verticies[3].uvw.x = 1;
	verticies[3].uvw.y = 1;
	verticies[3].uvw.z = 0;
	verticies[3].nrm = verticies[0].nrm;

	verticies[4].pos.x = -0.5f;
	verticies[4].pos.y = 0.5f;
	verticies[4].pos.z = 0.5f;
	verticies[4].uvw.x = 1;
	verticies[4].uvw.y = 0;
	verticies[4].uvw.z = 0;
	verticies[4].nrm.x = 0;
	verticies[4].nrm.y = 0;
	verticies[4].nrm.z = 1;

	verticies[5].pos.x = 0.5f;
	verticies[5].pos.y = 0.5f;
	verticies[5].pos.z = 0.5f;
	verticies[5].uvw.x = 0;
	verticies[5].uvw.y = 0;
	verticies[5].uvw.z = 0;
	verticies[5].nrm = verticies[4].nrm;

	verticies[6].pos.x = -0.5f;
	verticies[6].pos.y = -0.5f;
	verticies[6].pos.z = 0.5f;
	verticies[6].uvw.x = 1;
	verticies[6].uvw.y = 1;
	verticies[6].uvw.z = 0;
	verticies[6].nrm = verticies[4].nrm;

	verticies[7].pos.x = 0.5f;
	verticies[7].pos.y = -0.5f;
	verticies[7].pos.z = 0.5f;
	verticies[7].uvw.x = 0;
	verticies[7].uvw.y = 1;
	verticies[7].uvw.z = 0;
	verticies[7].nrm = verticies[4].nrm;

	// Left Right

	verticies[8].pos = verticies[4].pos;
	verticies[8].uvw.x = 0;
	verticies[8].uvw.y = 0;
	verticies[8].uvw.z = 0;
	verticies[8].nrm.x = -1;
	verticies[8].nrm.y = 0;
	verticies[8].nrm.z = 0;

	verticies[9].pos = verticies[0].pos;
	verticies[9].uvw.x = 1;
	verticies[9].uvw.y = 0;
	verticies[9].uvw.z = 0;
	verticies[9].nrm = verticies[8].nrm;

	verticies[10].pos = verticies[6].pos;
	verticies[10].uvw.x = 0;
	verticies[10].uvw.y = 1;
	verticies[10].uvw.z = 0;
	verticies[10].nrm = verticies[8].nrm;

	verticies[11].pos = verticies[2].pos;
	verticies[11].uvw.x = 1;
	verticies[11].uvw.y = 1;
	verticies[11].uvw.z = 0;
	verticies[11].nrm = verticies[8].nrm;

	verticies[12].pos = verticies[1].pos;
	verticies[12].uvw.x = 0;
	verticies[12].uvw.y = 0;
	verticies[12].uvw.z = 0;
	verticies[12].nrm.x = 1;
	verticies[12].nrm.y = 0;
	verticies[12].nrm.z = 0;

	verticies[13].pos = verticies[5].pos;
	verticies[13].uvw.x = 1;
	verticies[13].uvw.y = 0;
	verticies[13].uvw.z = 0;
	verticies[13].nrm = verticies[12].nrm;

	verticies[14].pos = verticies[3].pos;
	verticies[14].uvw.x = 0;
	verticies[14].uvw.y = 1;
	verticies[14].uvw.z = 0;
	verticies[14].nrm = verticies[12].nrm;

	verticies[15].pos = verticies[7].pos;
	verticies[15].uvw.x = 1;
	verticies[15].uvw.y = 1;
	verticies[15].uvw.z = 0;
	verticies[15].nrm = verticies[12].nrm;

	// Top Bottom

	verticies[16].pos = verticies[1].pos;
	verticies[16].uvw.x = 0;
	verticies[16].uvw.y = 0;
	verticies[16].uvw.z = 0;
	verticies[16].nrm.x = 0;
	verticies[16].nrm.y = 1;
	verticies[16].nrm.z = 0;

	verticies[17].pos = verticies[0].pos;
	verticies[17].uvw.x = 1;
	verticies[17].uvw.y = 0;
	verticies[17].uvw.z = 0;
	verticies[17].nrm = verticies[16].nrm;

	verticies[18].pos = verticies[5].pos;
	verticies[18].uvw.x = 0;
	verticies[18].uvw.y = 1;
	verticies[18].uvw.z = 0;
	verticies[18].nrm = verticies[16].nrm;

	verticies[19].pos = verticies[4].pos;
	verticies[19].uvw.x = 1;
	verticies[19].uvw.y = 1;
	verticies[19].uvw.z = 0;
	verticies[19].nrm = verticies[16].nrm;

	verticies[20].pos = verticies[2].pos;
	verticies[20].uvw.x = 0;
	verticies[20].uvw.y = 0;
	verticies[20].uvw.z = 0;
	verticies[20].nrm.x = 0;
	verticies[20].nrm.y = -1;
	verticies[20].nrm.z = 0;

	verticies[21].pos = verticies[3].pos;
	verticies[21].uvw.x = 1;
	verticies[21].uvw.y = 0;
	verticies[21].uvw.z = 0;
	verticies[21].nrm = verticies[20].nrm;

	verticies[22].pos = verticies[6].pos;
	verticies[22].uvw.x = 0;
	verticies[22].uvw.y = 1;
	verticies[22].uvw.z = 0;
	verticies[22].nrm = verticies[20].nrm;

	verticies[23].pos = verticies[7].pos;
	verticies[23].uvw.x = 1;
	verticies[23].uvw.y = 1;
	verticies[23].uvw.z = 0;
	verticies[23].nrm = verticies[20].nrm;
}