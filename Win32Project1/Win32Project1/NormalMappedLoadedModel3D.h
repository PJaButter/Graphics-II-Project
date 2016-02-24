#pragma once
#include "defines.h"
#define NUM_RASTER_STATES 2
#define NUM_SHADER_RESOURCE_VIEWS 2

class NormalMappedLoadedModel3D
{
public:
	NormalMappedLoadedModel3D();
	~NormalMappedLoadedModel3D();

	void Initialize(ID3D11Device* device, float initX, float initY, float initZ, const wchar_t* textureFilename, const wchar_t* normalMapFilename, const char * modelFilename);

	void Run(ID3D11DeviceContext* deviceContext);

	void Translate(float offsetX, float offsetY, float offsetZ);

	// Accessors
	XMMATRIX GetWorldMatrix();
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

	XMMATRIX worldMatrix;
	ID3D11Buffer* buffer;
	ID3D11Buffer* indexBuffer;
	unsigned int numVerticies;
	unsigned int numIndicies;
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* layout;
	ID3D11ShaderResourceView* shaderResourceViews[NUM_SHADER_RESOURCE_VIEWS];
	ID3D11SamplerState* sampler;
	ID3D11BlendState* blendState;
	ID3D11RasterizerState* rasterizerStates[NUM_RASTER_STATES];
	Vertex* verticies;
	unsigned int* indicies;

	struct SEND_TO_OBJECT
	{
		XMMATRIX worldMatrix;
	};
	SEND_TO_OBJECT toObject;

	bool loadOBJ(const char * filename);
};

