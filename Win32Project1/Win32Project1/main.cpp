//************************************************************
//************ INCLUDES & DEFINES ****************************
//************************************************************

#include "defines.h"
#include "SkyBox.h"
#include "Plane.h"
#include "LoadedModel3D.h"
#include "StarVertexShader.csh"
#include "StarPixelShader.csh"
#include "GeneralVertexShader.csh"
#include "GeneralPixelShader.csh"
#include "Cube3D.h"
#include "PointToQuad.h"
#include "Trivial_PS.csh"

IDXGISwapChain*					swapChain = nullptr;
ID3D11DeviceContext*			deviceContext = nullptr;
ID3D11RenderTargetView*			renderTargetView = nullptr;
ID3D11Device*					device = nullptr;
D3D11_VIEWPORT					viewports[2];
ID3D11DepthStencilView*			depthStencilView = nullptr;
ID3D11Texture2D*				depthStencil = nullptr;
XMMATRIX						ProjectionMatricies[2];
unsigned int					currentViewport = 0;

//************************************************************
//************ SIMPLE WINDOWS APP CLASS **********************
//************************************************************

class DEMO_APP
{	
	HINSTANCE						application;
	WNDPROC							appWndProc;
	HWND							window;

	XMMATRIX triangleWorldMatrix;
	XMMATRIX ViewMatricies[2];

	Cube3D cube1, cube2;
	SkyBox skyBox;
	Plane floor;
	LoadedModel3D brazier, turret;// , willowTree;
	PointToQuad pointToQuad;
	vector<thread> threads;
	
	ID3D11Buffer* starBuffer = nullptr;
	const unsigned int starNumVertices = 12;
	
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* layout;
	ID3D11RasterizerState* rasterizerStateEnabled = nullptr;
	ID3D11RasterizerState* rasterizerStateDisabled = nullptr;
	bool antialiasedEnabled = true;
	ID3D11BlendState* blendState = nullptr;
	
	ID3D11Buffer* constantBuffer[3];
	const unsigned int numConstantBuffers = 3;
	ID3D11Buffer* starIndexBuffer = nullptr;
	unsigned int starNumIndicies = 60; 

	XTime timer;
	
	struct SEND_TO_OBJECT
	{
		XMMATRIX worldMatrix;
	};

	struct SEND_TO_SCENE
	{
		XMMATRIX viewMatrix;
		XMMATRIX projectionMatrix;
	};
	
	SEND_TO_OBJECT toObject;
	SEND_TO_OBJECT toStarObject;
	SEND_TO_SCENE toScene;

public:

	DEMO_APP(HINSTANCE hinst, WNDPROC proc);
	bool Run();
	bool ShutDown();
};

//************************************************************
//************ CREATION OF OBJECTS & RESOURCES ***************
//************************************************************

DEMO_APP::DEMO_APP(HINSTANCE hinst, WNDPROC proc)
{
	application = hinst; 
	appWndProc = proc; 

	WNDCLASSEX  wndClass;
    ZeroMemory( &wndClass, sizeof( wndClass ) );
    wndClass.cbSize         = sizeof( WNDCLASSEX );             
    wndClass.lpfnWndProc    = appWndProc;						
    wndClass.lpszClassName  = L"DirectXApplication";            
	wndClass.hInstance      = application;		               
    wndClass.hCursor        = LoadCursor( NULL, IDC_ARROW );    
    wndClass.hbrBackground  = ( HBRUSH )( COLOR_WINDOWFRAME ); 
	//wndClass.hIcon			= LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FSICON));
    RegisterClassEx( &wndClass );

	RECT window_size = { 0, 0, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	window = CreateWindow(	L"DirectXApplication", L"Philip Bracco's Graphics II Project",	WS_OVERLAPPEDWINDOW, 
							CW_USEDEFAULT, CW_USEDEFAULT, window_size.right-window_size.left, window_size.bottom-window_size.top,					
							NULL, NULL,	application, this );												

    ShowWindow( window, SW_SHOW );

	DXGI_MODE_DESC modeDesc = {};
	modeDesc.Width = BACKBUFFER_WIDTH;
	modeDesc.Height = BACKBUFFER_HEIGHT;
	modeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferDesc = modeDesc;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.SampleDesc.Count = 1; // Number of msaa
	swapChainDesc.OutputWindow = window;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	
	HRESULT result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, NULL, 0, D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, NULL, &deviceContext);
	
	ID3D11Resource* pBackBuffer;
	swapChain->GetBuffer(0, __uuidof(pBackBuffer), reinterpret_cast<void**>(&pBackBuffer));
	device->CreateRenderTargetView(pBackBuffer, NULL, &renderTargetView);
	SAFE_RELEASE(pBackBuffer);
	
	viewports[0].TopLeftX = 0;
	viewports[0].TopLeftY = 0;
	viewports[0].MinDepth = 0;
	viewports[0].MaxDepth = 1;
	DXGI_SWAP_CHAIN_DESC tempDesc = {};
	swapChain->GetDesc(&tempDesc);
	viewports[0].Width = (float)tempDesc.BufferDesc.Width;
	viewports[0].Height = (float)tempDesc.BufferDesc.Height;

	viewports[1].TopLeftX = 0;
	viewports[1].TopLeftY = 0;
	viewports[1].MinDepth = 0;
	viewports[1].MaxDepth = 1;
	viewports[1].Width = (float)tempDesc.BufferDesc.Width / 4.0f;
	viewports[1].Height = (float)tempDesc.BufferDesc.Height / 4.0f;


	SIMPLE_VERTEX triangle[12];
	int counter = 0;
	for (unsigned int i = 0; i < 360; ++i)
	{
		if (i % 36 == 0)
		{
			if (counter % 2 == 0)
			{
				triangle[counter].pos.x = cos(XMConvertToRadians((float)i));
				triangle[counter].pos.y = sin(XMConvertToRadians((float)i));
			}
			else
			{
				triangle[counter].pos.x = cos(XMConvertToRadians((float)i)) * 0.5f;
				triangle[counter].pos.y = sin(XMConvertToRadians((float)i)) * 0.5f;
			}
			triangle[counter].pos.z = 0;
			triangle[counter].rgba.x = 0;
			triangle[counter].rgba.y = 1;
			triangle[counter].rgba.z = 0;
			triangle[counter].rgba.w = 1;
			counter++;
		}
		if (counter == 10)
			break;
	}

	triangle[10].pos.x = 0;
	triangle[10].pos.y = 0;
	triangle[10].pos.z = 0.25f;
	triangle[10].rgba.x = 1;
	triangle[10].rgba.y = 1;
	triangle[10].rgba.z = 1;

	triangle[11].pos.x = 0;
	triangle[11].pos.y = 0;
	triangle[11].pos.z = -0.25f;
	triangle[11].rgba.x = 1;
	triangle[11].rgba.y = 1;
	triangle[11].rgba.z = 1;

	triangleWorldMatrix = XMMatrixIdentity();
	triangleWorldMatrix = XMMatrixTranslation(2, 2, 3);

	ViewMatricies[0] = XMMatrixIdentity();
	ViewMatricies[1] = XMMatrixRotationX(5);
	ViewMatricies[1] = XMMatrixMultiply(ViewMatricies[1], XMMatrixTranslation(0, 30, 0));

	ProjectionMatricies[0] = XMMatrixPerspectiveFovLH(XMConvertToRadians(65), ASPECTRATIO, NEARPLANE, FARPLANE);
	ProjectionMatricies[1] = XMMatrixPerspectiveFovLH(XMConvertToRadians(65), ASPECTRATIO, NEARPLANE, FARPLANE);

	D3D11_BUFFER_DESC starBufferDesc = {};
	starBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	starBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	starBufferDesc.ByteWidth = sizeof(SIMPLE_VERTEX)* starNumVertices;
	// TODO: PART 2 STEP 3c
	D3D11_SUBRESOURCE_DATA starSubresourceDesc;
	starSubresourceDesc.pSysMem = triangle;
	// TODO: PART 2 STEP 3d
	result = device->CreateBuffer(&starBufferDesc, &starSubresourceDesc, &starBuffer);
	
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = BACKBUFFER_WIDTH;
	descDepth.Height = BACKBUFFER_HEIGHT;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	result = device->CreateTexture2D(&descDepth, NULL, &depthStencil);

	result = device->CreateDepthStencilView(depthStencil, nullptr, &depthStencilView);

	result = device->CreateVertexShader(StarVertexShader, sizeof(StarVertexShader), NULL, &vertexShader);
	result = device->CreatePixelShader(StarPixelShader, sizeof(StarPixelShader), NULL, &pixelShader);

	D3D11_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	result = device->CreateInputLayout(inputLayout, 2, StarVertexShader, sizeof(StarVertexShader), &layout);

	toStarObject.worldMatrix = triangleWorldMatrix;
	toScene.viewMatrix = ViewMatricies[currentViewport];
	toScene.projectionMatrix = ProjectionMatricies[currentViewport];

	D3D11_BUFFER_DESC bufferDesc2 = {};
	bufferDesc2.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc2.ByteWidth = sizeof(SEND_TO_OBJECT);
	bufferDesc2.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA subresourceDesc2;
	(subresourceDesc2.pSysMem) = &toObject;

	result = device->CreateBuffer(&bufferDesc2, &subresourceDesc2, &constantBuffer[0]);

	D3D11_BUFFER_DESC bufferDesc3 = {};
	bufferDesc3.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc3.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc3.ByteWidth = sizeof(SEND_TO_SCENE);
	bufferDesc3.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA subresourceDesc3;
	(subresourceDesc3.pSysMem) = &toScene;

	result = device->CreateBuffer(&bufferDesc3, &subresourceDesc3, &constantBuffer[1]);

	D3D11_BUFFER_DESC bufferDesc5 = {};
	bufferDesc5.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc5.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc5.ByteWidth = sizeof(SEND_TO_OBJECT);
	bufferDesc5.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA subresourceDesc5;
	(subresourceDesc5.pSysMem) = &toStarObject;

	result = device->CreateBuffer(&bufferDesc5, &subresourceDesc5, &constantBuffer[2]);

	D3D11_BUFFER_DESC starIndexBufferDesc = {};
	starIndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	starIndexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	starIndexBufferDesc.ByteWidth = 250;

	unsigned int starIndicies[60] =
	{ 
		2, 1, 10, 
		1, 0, 10, 
		0, 9, 10, 
		9, 8, 10, 
		8, 7, 10, 
		7, 6, 10, 
		6, 5, 10, 
		5, 4, 10, 
		4, 3, 10, 
		3, 2, 10,

	    1, 2, 11, 
		0, 1, 11, 
		9, 0, 11, 
		8, 9, 11, 
		7, 8, 11, 
		6, 7, 11, 
		5, 6, 11, 
		4, 5, 11, 
		3, 4, 11, 
		2, 3, 11
	};

	D3D11_SUBRESOURCE_DATA starIndexInitData;
	starIndexInitData.pSysMem = starIndicies;
	result = device->CreateBuffer(&starIndexBufferDesc, &starIndexInitData, &starIndexBuffer);

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;

	const wchar_t* filename1 = L"Box_wood01.dds";
	threads.push_back(thread(&Cube3D::Initialize, &cube1, device, -2, 1, 5, filename1));

	const wchar_t* filename2 = L"Box_wood01.dds";
	threads.push_back(thread(&Cube3D::Initialize, &cube2, device, 0, 5, 10, filename2));

	const wchar_t* skyBoxFilename = L"SkyBoxCube.dds";
	threads.push_back(thread(&SkyBox::Initialize, &skyBox, device, 0, 0, 0, skyBoxFilename, true));

	const wchar_t* floorFilename = L"Floor.dds";
	threads.push_back(thread(&Plane::Initialize, &floor, device, 0, -1, 0, floorFilename));

	const wchar_t* brazierFilename = L"brazier.dds";
	threads.push_back(thread(&LoadedModel3D::Initialize, &brazier, device, 7, -1, 10, brazierFilename, "brazier.obj"));

	const wchar_t* turretFilename = L"T_HeavyTurret_D.dds";
	threads.push_back(thread(&LoadedModel3D::Initialize, &turret, device, -7, -1, 10, turretFilename, "turret.obj"));

	pointToQuad.Initialize(device, 0, 0, 10);

	for (int i = 0; i < threads.size(); ++i)
		threads[i].join();

	//const wchar_t* treeFilename = L"treeWillow.dds";
	//willowTree.Initialize(device, 0, -1, 10, treeFilename, "willowtree.obj");

	D3D11_RASTERIZER_DESC rasterDesc = {};
	rasterDesc.AntialiasedLineEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_BACK;

	result = device->CreateRasterizerState(&rasterDesc, &rasterizerStateEnabled);

	D3D11_RASTERIZER_DESC rasterDesc2 = {};
	rasterDesc2.AntialiasedLineEnable = false;
	rasterDesc2.FillMode = D3D11_FILL_SOLID;
	rasterDesc2.CullMode = D3D11_CULL_BACK;
	result = device->CreateRasterizerState(&rasterDesc2, &rasterizerStateDisabled);

	timer.Restart();

}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	timer.Signal();
	ViewMatricies[0] = XMMatrixInverse(nullptr, ViewMatricies[0]);
	ViewMatricies[1] = XMMatrixInverse(nullptr, ViewMatricies[1]);

	if (GetAsyncKeyState('W'))
	{
		ViewMatricies[0] = XMMatrixMultiply(XMMatrixTranslation(0, 0, 3.5f * (float)timer.Delta()), ViewMatricies[0]);
		ViewMatricies[1] = XMMatrixMultiply(XMMatrixTranslation(0, 0, 3.5f * (float)timer.Delta()), ViewMatricies[1]);
	}
	if (GetAsyncKeyState('S'))
	{
		ViewMatricies[0] = XMMatrixMultiply(XMMatrixTranslation(0, 0, -3.5f * (float)timer.Delta()), ViewMatricies[0]);
		ViewMatricies[1] = XMMatrixMultiply(XMMatrixTranslation(0, 0, -3.5f * (float)timer.Delta()), ViewMatricies[1]);
	}
	if (GetAsyncKeyState('A'))
	{
		ViewMatricies[0] = XMMatrixMultiply(XMMatrixTranslation(-3.5f * (float)timer.Delta(), 0, 0), ViewMatricies[0]);
		ViewMatricies[1] = XMMatrixMultiply(XMMatrixTranslation(-3.5f * (float)timer.Delta(), 0, 0), ViewMatricies[1]);
	}
	if (GetAsyncKeyState('D'))
	{
		ViewMatricies[0] = XMMatrixMultiply(XMMatrixTranslation(3.5f * (float)timer.Delta(), 0, 0), ViewMatricies[0]);
		ViewMatricies[1] = XMMatrixMultiply(XMMatrixTranslation(3.5f * (float)timer.Delta(), 0, 0), ViewMatricies[1]);
	}

	if (GetAsyncKeyState(VK_LEFT))
	{
		XMVECTOR tempPos = ViewMatricies[0].r[3];
		ViewMatricies[0].r[3] = XMVectorSet(0, 0, 0, 1);
		ViewMatricies[0] = XMMatrixMultiply(ViewMatricies[0], XMMatrixRotationY((float)timer.Delta() * -2));
		ViewMatricies[0].r[3] = tempPos;
	}
	if (GetAsyncKeyState(VK_RIGHT))
	{
		XMVECTOR tempPos = ViewMatricies[0].r[3];
		ViewMatricies[0].r[3] = XMVectorSet(0, 0, 0, 1);
		ViewMatricies[0] = XMMatrixMultiply(ViewMatricies[0], XMMatrixRotationY((float)timer.Delta() * 2));
		ViewMatricies[0].r[3] = tempPos;
	}
	if (GetAsyncKeyState(VK_UP))
	{
		XMVECTOR tempPos = ViewMatricies[0].r[3];
		ViewMatricies[0].r[3] = XMVectorSet(0, 0, 0, 1);
		ViewMatricies[0] = XMMatrixMultiply(XMMatrixRotationX((float)timer.Delta() * -2), ViewMatricies[0]);
		ViewMatricies[0].r[3] = tempPos;
	}
	if (GetAsyncKeyState(VK_DOWN))
	{
		XMVECTOR tempPos = ViewMatricies[0].r[3];
		ViewMatricies[0].r[3] = XMVectorSet(0, 0, 0, 1);
		ViewMatricies[0] = XMMatrixMultiply(XMMatrixRotationX((float)timer.Delta() * 2), ViewMatricies[0]);
		ViewMatricies[0].r[3] = tempPos;
	}

	ViewMatricies[0] = XMMatrixInverse(nullptr, ViewMatricies[0]);
	ViewMatricies[1] = XMMatrixInverse(nullptr, ViewMatricies[1]);

	if (GetAsyncKeyState('3'))
	{
		antialiasedEnabled = true;
	}
	else if (GetAsyncKeyState('4'))
	{
		antialiasedEnabled = false;
	}

	float color[4] = { 0, 0, 1, 1 };
	deviceContext->ClearRenderTargetView(renderTargetView, color);
	//if (GetCursorPos(&mousePos))
	//{
	//	int deltaX = prevMousePos.x - mousePos.x;
	//	int deltaY = prevMousePos.y - mousePos.y;
	//	
	//	ViewMatrix = XMMatrixMultiply(XMMatrixRotationX(deltaY * 0.01f), ViewMatrix);
	//	ViewMatrix = XMMatrixMultiply(ViewMatrix, XMMatrixRotationY(deltaX * 0.01f));
	//}
	for (currentViewport = 0; currentViewport < NUMVIEWPORTS; ++currentViewport)
	{
		deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
		deviceContext->RSSetViewports(1, &viewports[currentViewport]);

		deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1, 0);

		D3D11_MAPPED_SUBRESOURCE mapped;
		deviceContext->Map(constantBuffer[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		XMMATRIX* temp = ((XMMATRIX*)mapped.pData);
		cube1.SetWorldMatrix(&XMMatrixMultiply(XMMatrixRotationY((float)timer.Delta()), cube1.GetWorldMatrix()));
		*temp = cube1.GetWorldMatrix();
		deviceContext->Unmap(constantBuffer[0], 0);

		D3D11_MAPPED_SUBRESOURCE mapped2;
		deviceContext->Map(constantBuffer[1], 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped2);
		XMMATRIX* temp2 = ((XMMATRIX*)mapped2.pData);
		temp2[0] = ViewMatricies[currentViewport];
		temp2[1] = ProjectionMatricies[currentViewport];
		deviceContext->Unmap(constantBuffer[1], 0);

		deviceContext->VSSetConstantBuffers(0, numConstantBuffers, constantBuffer);

		cube1.Run(deviceContext);

		deviceContext->Map(constantBuffer[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		temp = ((XMMATRIX*)mapped.pData);
		*temp = cube2.GetWorldMatrix();
		deviceContext->Unmap(constantBuffer[0], 0);
		deviceContext->VSSetConstantBuffers(0, numConstantBuffers, constantBuffer);

		cube2.Run(deviceContext);

		deviceContext->Map(constantBuffer[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		temp = ((XMMATRIX*)mapped.pData);
		*temp = brazier.GetWorldMatrix();
		deviceContext->Unmap(constantBuffer[0], 0);
		deviceContext->VSSetConstantBuffers(0, numConstantBuffers, constantBuffer);

		brazier.Run(deviceContext);

		deviceContext->Map(constantBuffer[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		temp = ((XMMATRIX*)mapped.pData);
		*temp = turret.GetWorldMatrix();
		deviceContext->Unmap(constantBuffer[0], 0);
		deviceContext->VSSetConstantBuffers(0, numConstantBuffers, constantBuffer);

		deviceContext->Map(constantBuffer[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		temp = ((XMMATRIX*)mapped.pData);
		*temp = pointToQuad.GetWorldMatrix();
		deviceContext->Unmap(constantBuffer[0], 0);
		deviceContext->GSSetConstantBuffers(0, numConstantBuffers, constantBuffer);

		pointToQuad.Run(deviceContext);

		//deviceContext->Map(constantBuffer[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		//temp = ((XMMATRIX*)mapped.pData);
		//*temp = willowTree.GetWorldMatrix();
		//deviceContext->Unmap(constantBuffer[0], 0);
		//deviceContext->VSSetConstantBuffers(0, numConstantBuffers, constantBuffer);

		//willowTree.Run(deviceContext);

		deviceContext->Map(constantBuffer[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		temp = ((XMMATRIX*)mapped.pData);
		XMMATRIX tempMatrix = XMMatrixIdentity();
		ViewMatricies[currentViewport] = XMMatrixInverse(nullptr, ViewMatricies[currentViewport]);
		tempMatrix.r[3] = ViewMatricies[currentViewport].r[3];
		ViewMatricies[currentViewport] = XMMatrixInverse(nullptr, ViewMatricies[currentViewport]);
		skyBox.SetWorldMatrix(&tempMatrix);
		*temp = skyBox.GetWorldMatrix();
		deviceContext->Unmap(constantBuffer[0], 0);
		deviceContext->VSSetConstantBuffers(0, numConstantBuffers, constantBuffer);

		skyBox.Run(deviceContext);

		deviceContext->Map(constantBuffer[2], 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		temp = ((XMMATRIX*)mapped.pData);
		triangleWorldMatrix = XMMatrixMultiply(XMMatrixRotationY((float)timer.Delta()), triangleWorldMatrix);
		*temp = triangleWorldMatrix;
		deviceContext->Unmap(constantBuffer[2], 0);

		deviceContext->Map(constantBuffer[1], 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped2);
		temp2 = ((XMMATRIX*)mapped2.pData);
		temp2[0] = ViewMatricies[currentViewport];
		temp2[1] = ProjectionMatricies[currentViewport];
		deviceContext->Unmap(constantBuffer[1], 0);

		deviceContext->VSSetConstantBuffers(0, 3, constantBuffer);
		deviceContext->IASetIndexBuffer(starIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		unsigned int vertexSize = sizeof(SIMPLE_VERTEX);
		unsigned int offset = 0;
		deviceContext->IASetVertexBuffers(0, 1, &starBuffer, &vertexSize, &offset);
		deviceContext->VSSetShader(vertexShader, NULL, 0);
		deviceContext->PSSetShader(pixelShader, NULL, 0);
		deviceContext->IASetInputLayout(layout);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		deviceContext->DrawIndexed(starNumIndicies, 0, 0);

		vertexSize = sizeof(SIMPLE_VERTEX);
		offset = 0;

		deviceContext->Map(constantBuffer[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		temp = ((XMMATRIX*)mapped.pData);
		*temp = floor.GetWorldMatrix();
		deviceContext->Unmap(constantBuffer[0], 0);

		// Draw Floor
		floor.Run(deviceContext);
	}

	if (antialiasedEnabled)
		deviceContext->RSSetState(rasterizerStateEnabled);
	else
		deviceContext->RSSetState(rasterizerStateDisabled);

	swapChain->Present(0, 0);

	return true; 
}

//************************************************************
//************ DESTRUCTION ***********************************
//************************************************************

bool DEMO_APP::ShutDown()
{
	SAFE_RELEASE(device);
	SAFE_RELEASE(deviceContext);
	SAFE_RELEASE(renderTargetView);
	SAFE_RELEASE(swapChain);
	SAFE_RELEASE(vertexShader);
	SAFE_RELEASE(pixelShader);
	SAFE_RELEASE(layout);
	for (unsigned int i = 0; i < numConstantBuffers; ++i)
		SAFE_RELEASE(constantBuffer[i]);
	SAFE_RELEASE(starIndexBuffer);
	SAFE_RELEASE(depthStencil);
	SAFE_RELEASE(depthStencilView);
	SAFE_RELEASE(starBuffer);
	SAFE_RELEASE(rasterizerStateEnabled);
	SAFE_RELEASE(rasterizerStateDisabled);
	SAFE_RELEASE(blendState);

	UnregisterClass( L"DirectXApplication", application ); 
	return true;
}

//************************************************************
//************ WINDOWS RELATED *******************************
//************************************************************
	
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine,	int nCmdShow );						   
LRESULT CALLBACK WndProc(HWND hWnd,	UINT message, WPARAM wparam, LPARAM lparam );		
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE, LPTSTR, int )
{
	srand(unsigned int(time(0)));
	DEMO_APP myApp(hInstance,(WNDPROC)WndProc);	
    MSG msg; ZeroMemory( &msg, sizeof( msg ) );
    while ( msg.message != WM_QUIT && myApp.Run() )
    {	
	    if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        { 
            TranslateMessage( &msg );
            DispatchMessage( &msg ); 
        }
    }
	myApp.ShutDown(); 
	return 0; 
}
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    if(GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;
    switch ( message )
    {
	case WM_SIZE:
	{
		if (swapChain)
		{
			deviceContext->OMSetRenderTargets(0, 0, 0);

			renderTargetView->Release();
			depthStencil->Release();
			depthStencilView->Release();

			HRESULT result = swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

			ID3D11Texture2D* buffer;
			result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&buffer);

			result = device->CreateRenderTargetView(buffer, NULL, &renderTargetView);

			ID3D11Texture2D* texture;
			ID3D11Resource* res;
			D3D11_TEXTURE2D_DESC dsc;

			renderTargetView->GetResource(&res);
			res->QueryInterface(__uuidof(ID3D11Texture2D), (LPVOID*)&texture);
			texture->GetDesc(&dsc);

			UINT height = dsc.Height;
			UINT width = dsc.Width;

			D3D11_TEXTURE2D_DESC descDepth = {};
			descDepth.Width = width;
			descDepth.Height = height;
			descDepth.MipLevels = 1;
			descDepth.ArraySize = 1;
			descDepth.Format = DXGI_FORMAT_D32_FLOAT;
			descDepth.SampleDesc.Count = 1;
			descDepth.Usage = D3D11_USAGE_DEFAULT;
			descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			result = device->CreateTexture2D(&descDepth, NULL, &depthStencil);

			result = device->CreateDepthStencilView(depthStencil, nullptr, &depthStencilView);

			buffer->Release();

			deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
			
			viewports[0].Width = width;
			viewports[0].Height = height;
			viewports[0].MinDepth = 0.0f;
			viewports[0].MaxDepth = 1.0f;
			viewports[0].TopLeftX = 0;
			viewports[0].TopLeftY = 0;

			viewports[1].TopLeftX = 0;
			viewports[1].TopLeftY = 0;
			viewports[1].MinDepth = 0;
			viewports[1].MaxDepth = 1;
			viewports[1].Width = (float)width / 4.0f;
			viewports[1].Height = (float)height / 4.0f;

			deviceContext->RSSetViewports(1, &viewports[0]);

			ProjectionMatricies[0] = XMMatrixPerspectiveFovLH(XMConvertToRadians(65), (float)width / (float)height, NEARPLANE, FARPLANE);
			ProjectionMatricies[1] = XMMatrixPerspectiveFovLH(XMConvertToRadians(65), (float)viewports[1].Width / (float)viewports[1].Height, NEARPLANE, FARPLANE);

			texture->Release();
			res->Release();
		}
		break;
	}
        case ( WM_DESTROY ): { PostQuitMessage( 0 ); }
        break;
    }
    return DefWindowProc( hWnd, message, wParam, lParam );
}


XMMATRIX Movement(float time)
{
	XMMATRIX matrix;
	if (GetAsyncKeyState('W'))
	{
		matrix = XMMatrixMultiply(matrix, XMMatrixTranslation(0, 0, -1.5f * time));
	}
	if (GetAsyncKeyState('S'))
	{
		matrix = XMMatrixMultiply(matrix, XMMatrixTranslation(0, 0, 1.5f * time));
	}
	if (GetAsyncKeyState('A'))
	{
		matrix = XMMatrixMultiply(matrix, XMMatrixTranslation(1.5f * time, 0, 0));
	}
	if (GetAsyncKeyState('D'))
	{
		matrix = XMMatrixMultiply(matrix, XMMatrixTranslation(-1.5f * time, 0, 0));
	}

	if (GetAsyncKeyState(VK_LEFT))
	{
		matrix = XMMatrixMultiply(matrix, XMMatrixRotationY(time * 2));
	}
	if (GetAsyncKeyState(VK_RIGHT))
	{
		matrix = XMMatrixMultiply(matrix, XMMatrixRotationY(time * -2));
	}
	if (GetAsyncKeyState(VK_UP))
	{
		matrix = XMMatrixMultiply(matrix, XMMatrixRotationX(time * 2));
	}
	if (GetAsyncKeyState(VK_DOWN))
	{
		matrix = XMMatrixMultiply(matrix, XMMatrixRotationX(time * -2));
	}

	return matrix;
}