#pragma once

#include "defines.h"

class PointToQuad
{
public:
	PointToQuad();
	~PointToQuad();

	void Initialize(ID3D11Device* device, float initX, float initY, float initZ);

	void Run(ID3D11DeviceContext* deviceContext);

	void Translate(float offsetX, float offsetY, float offsetZ);

	// Accessors
	XMMATRIX GetWorldMatrix();
	ID3D11Buffer* GetBuffer() const;
	unsigned int GetNumIndicies() const;
	ID3D11VertexShader* GetVertexShader() const;
	ID3D11GeometryShader* GetGeometryShader() const;
	ID3D11PixelShader* GetPixelShader() const;
	ID3D11InputLayout* GetLayout() const;

	// Mutators

	void SetWorldMatrix(const XMMATRIX* matrix);

private:

	XMMATRIX worldMatrix;
	ID3D11Buffer* buffer;
	ID3D11VertexShader* vertexShader;
	ID3D11GeometryShader* geometryShader;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* layout;
	SIMPLE_VERTEX* verticies;

	struct SEND_TO_OBJECT
	{
		XMMATRIX worldMatrix;
	};
	SEND_TO_OBJECT toObject;

	void CreateVerticies();
};



