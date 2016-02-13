#include "Plane.h"
#include "PlaneVertexShader.csh"
#include "PlanePixelShader.csh"
#include "DDSTextureLoader.h"

#define NUMVERTICIES 24
#define NUMINDICIES 36

#define SAFE_RELEASE(p) { if(p) {p->Release(); p = nullptr;}}

Plane::Plane()
{
	worldMatrix = XMMatrixIdentity();
	verticies = new Vertex[NUMVERTICIES];
}


Plane::~Plane()
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

void Plane::Initialize(ID3D11Device* device, float initX, float initY, float initZ, const wchar_t* filename)
{
	worldMatrix = XMMatrixIdentity();
	worldMatrix = XMMatrixTranslation(initX, initY, initZ);
	numIndicies = NUMINDICIES;
	CreateVerticies();

	HRESULT result = CreateDDSTextureFromFile(device, filename, nullptr, &shaderResourceView);

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(Vertex)* NUMVERTICIES;

	D3D11_SUBRESOURCE_DATA subresourceDesc;
	subresourceDesc.pSysMem = verticies;

	result = device->CreateBuffer(&bufferDesc, &subresourceDesc, &buffer);

	result = device->CreateVertexShader(PlaneVertexShader, sizeof(PlaneVertexShader), NULL, &vertexShader);
	result = device->CreatePixelShader(PlanePixelShader, sizeof(PlanePixelShader), NULL, &pixelShader);

	D3D11_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXTPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMALS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;

	result = device->CreateSamplerState(&samplerDesc, &sampler);

	result = device->CreateInputLayout(inputLayout, 3, PlaneVertexShader, sizeof(PlaneVertexShader), &layout);

	toObject.worldMatrix = worldMatrix;

	unsigned int tempIndicies[6] =
	{
		3, 2, 0,
		0, 1, 3
	};

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.ByteWidth = sizeof(unsigned int) * NUMINDICIES;

	D3D11_SUBRESOURCE_DATA indexInitData;
	indexInitData.pSysMem = tempIndicies;

	result = device->CreateBuffer(&indexBufferDesc, &indexInitData, &indexBuffer);
}

void Plane::Run(ID3D11DeviceContext* deviceContext)
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

void Plane::Translate(float offsetX, float offsetY, float offsetZ)
{
	worldMatrix = XMMatrixTranslation(offsetX, offsetY, offsetZ);
}

XMMATRIX Plane::GetWorldMatrix()
{
	return worldMatrix;
}

ID3D11Buffer* Plane::GetBuffer() const
{
	return buffer;
}

ID3D11Buffer* Plane::GetIndexBuffer() const
{
	return indexBuffer;
}

unsigned int Plane::GetNumIndicies() const
{
	return numIndicies;
}

ID3D11VertexShader* Plane::GetVertexShader() const
{
	return vertexShader;
}

ID3D11PixelShader* Plane::GetPixelShader() const
{
	return pixelShader;
}

ID3D11InputLayout* Plane::GetLayout() const
{
	return layout;
}

ID3D11SamplerState* Plane::GetSampler() const
{
	return sampler;
}

void Plane::SetWorldMatrix(const XMMATRIX* matrix)
{
	worldMatrix = *matrix;
}

// Private Member Functions
void Plane::CreateVerticies()
{
	verticies[0].pos = XMFLOAT3(-50, 0, 50);
	verticies[0].uvw = XMFLOAT3(0, 0, 0);
	
	verticies[1].pos = XMFLOAT3(50, 0, 50);
	verticies[1].uvw = XMFLOAT3(50, 0, 0);
	
	verticies[2].pos = XMFLOAT3(-50, 0, -50);
	verticies[2].uvw = XMFLOAT3(0, 50, 0);
	
	verticies[3].pos = XMFLOAT3(50, 0, -50);
	verticies[3].uvw = XMFLOAT3(50, 50, 0);
}