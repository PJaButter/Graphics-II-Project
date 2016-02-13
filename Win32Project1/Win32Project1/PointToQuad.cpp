#include "PointToQuad.h"
#include "PointToQuadVertexShader.csh"
#include "PointToQuadPixelShader.csh"
#include "PointToQuadGeometryShader.csh"

#define NUMVERTICIES 1

#define SAFE_RELEASE(p) { if(p) {p->Release(); p = nullptr;}}

PointToQuad::PointToQuad()
{
	worldMatrix = XMMatrixIdentity();
	verticies = new SIMPLE_VERTEX[NUMVERTICIES];
}


PointToQuad::~PointToQuad()
{
	SAFE_RELEASE(buffer);
	SAFE_RELEASE(vertexShader);
	SAFE_RELEASE(geometryShader);
	SAFE_RELEASE(pixelShader);
	SAFE_RELEASE(layout);
	delete[] verticies;
}

void PointToQuad::Initialize(ID3D11Device* device, float initX, float initY, float initZ)
{
	worldMatrix = XMMatrixTranslation(initX, initY, initZ);
	CreateVerticies();

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(SIMPLE_VERTEX)* NUMVERTICIES;

	D3D11_SUBRESOURCE_DATA subresourceDesc;
	subresourceDesc.pSysMem = verticies;

	HRESULT result = device->CreateBuffer(&bufferDesc, &subresourceDesc, &buffer);

	result = device->CreateVertexShader(PointToQuadVertexShader, sizeof(PointToQuadVertexShader), NULL, &vertexShader);
	result = device->CreateGeometryShader(PointToQuadGeometryShader, sizeof(PointToQuadGeometryShader), NULL, &geometryShader);
	result = device->CreatePixelShader(PointToQuadPixelShader, sizeof(PointToQuadPixelShader), NULL, &pixelShader);

	D3D11_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	result = device->CreateInputLayout(inputLayout, 2, PointToQuadVertexShader, sizeof(PointToQuadVertexShader), &layout);

	toObject.worldMatrix = worldMatrix;
}

void PointToQuad::Run(ID3D11DeviceContext* deviceContext)
{
	deviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);

	unsigned int vertexSize = sizeof(SIMPLE_VERTEX);
	unsigned int offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &buffer, &vertexSize, &offset);
	deviceContext->VSSetShader(vertexShader, NULL, 0);
	deviceContext->GSSetShader(geometryShader, nullptr, 0);
	deviceContext->PSSetShader(pixelShader, NULL, 0);
	deviceContext->IASetInputLayout(layout);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	deviceContext->Draw(NUMVERTICIES, 0);
}

void PointToQuad::Translate(float offsetX, float offsetY, float offsetZ)
{
	worldMatrix = XMMatrixTranslation(offsetX, offsetY, offsetZ);
}

XMMATRIX PointToQuad::GetWorldMatrix()
{
	return worldMatrix;
}

ID3D11Buffer* PointToQuad::GetBuffer() const
{
	return buffer;
}

ID3D11VertexShader* PointToQuad::GetVertexShader() const
{
	return vertexShader;
}

ID3D11PixelShader* PointToQuad::GetPixelShader() const
{
	return pixelShader;
}

ID3D11InputLayout* PointToQuad::GetLayout() const
{
	return layout;
}

void PointToQuad::SetWorldMatrix(const XMMATRIX* matrix)
{
	worldMatrix = *matrix;
}

// Private Member Functions
void PointToQuad::CreateVerticies()
{
	//verticies[0].pos = XMFLOAT3(rand() % 6 - 3, rand() % 6, rand() % 6 - 3);
	verticies[0].pos = XMFLOAT3(0, 0, 0);
	verticies[0].rgba = XMFLOAT4(0, 0, 1, 1);
}