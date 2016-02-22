#pragma once
#include "defines.h"

class InstancedCube3D
{
public:
	InstancedCube3D();
	~InstancedCube3D();

	void Initialize(ID3D11Device* device, float initX, float initY, float initZ, const wchar_t* filename);

	void Run(ID3D11DeviceContext* deviceContext);

	void Translate(float offsetX, float offsetY, float offsetZ);

	// Accessors
	XMMATRIX GetWorldMatrix(const unsigned int index);
	ID3D11Buffer* GetBuffer() const;
	ID3D11Buffer* GetIndexBuffer() const;
	unsigned int GetNumIndicies() const;
	ID3D11VertexShader* GetVertexShader() const;
	ID3D11PixelShader* GetPixelShader() const;
	ID3D11InputLayout* GetLayout() const;
	ID3D11ShaderResourceView* GetShaderResourceView() const;
	ID3D11SamplerState* GetSampler() const;

	// Mutators

	void SetWorldMatrix(const XMMATRIX* matrix);

private:

	XMMATRIX worldMatrix[6];
	ID3D11Buffer* buffer;
	ID3D11Buffer* indexBuffer;
	unsigned int numIndicies;
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* layout;
	ID3D11ShaderResourceView* shaderResourceView;
	ID3D11SamplerState* sampler;
	Vertex* verticies;

	struct SEND_TO_INST_OBJECT
	{
		XMMATRIX worldMatrix[6];
	};
	SEND_TO_INST_OBJECT toObject;

	void CreateVerticies();
};

