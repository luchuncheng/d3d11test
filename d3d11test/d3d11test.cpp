// d3d11test.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "d3d11test.h"
#include <d3d11.h>
#include <D3DX11tex.h>
#include <xnamath.h>
#include <fstream>
#include <vector>
#include "d3dx11effect.h"

#include <gdiplus.h>

using namespace Gdiplus;

#pragma comment(lib, "gdiplus.lib")  

const float Pi = 3.1415926535f;

namespace Colors
{
	XMGLOBALCONST XMVECTORF32 White     = {1.0f, 1.0f, 1.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Black     = {0.0f, 0.0f, 0.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Red       = {1.0f, 0.0f, 0.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Green     = {0.0f, 1.0f, 0.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Blue      = {0.0f, 0.0f, 0.5f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Yellow    = {1.0f, 1.0f, 0.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Cyan      = {0.0f, 1.0f, 1.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Magenta   = {1.0f, 0.0f, 1.0f, 1.0f};

	XMGLOBALCONST XMVECTORF32 Silver    = {0.75f, 0.75f, 0.75f, 1.0f};
	XMGLOBALCONST XMVECTORF32 LightSteelBlue = {0.69f, 0.77f, 0.87f, 1.0f};
}

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
	XMFLOAT2 Texcoord;
};

float kWidth = 800.0f, kHeight = 600.0f;

ID3D11Device* kD3DDevice = NULL;
ID3D11DeviceContext* kD3DDeviceContext = NULL;
ID3D11Texture2D* kBackBuffer = NULL;
ID3D11Texture2D* kDepthStencilBuffer = NULL;
ID3D11RenderTargetView* kRenderTargetView = NULL;
ID3D11DepthStencilView* kDepthStencilView = NULL;
ID3D11InputLayout *kInputLayout = NULL;
D3D11_VIEWPORT kScreenViewport;

ID3DX11Effect* kFX = NULL;
ID3DX11EffectTechnique* kTech = NULL;
ID3DX11EffectMatrixVariable* fxWorldViewProj = NULL;

ID3D11ShaderResourceView* kTexture = NULL;

ID3D11Buffer* kVB = NULL;
ID3D11Buffer* kIB = NULL;

XMFLOAT4X4 kView;
XMFLOAT4X4 kProj;

BOOL kCapture = FALSE;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;
	UINT  size = 0;
	ImageCodecInfo* pImageCodecInfo = NULL;
	GetImageEncodersSize(&num, &size);
	if (size == 0) return -1;
	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL) return -1;
	GetImageEncoders(num, size, pImageCodecInfo);
	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;
		}
	}
	free(pImageCodecInfo);
	return -1;
}

void SaveTexture(ID3D11Texture2D *texture)
{
	HRESULT hr = S_OK;

	D3D11_TEXTURE2D_DESC desc;
	texture->GetDesc(&desc);

	D3D11_TEXTURE2D_DESC desc2;
	desc2.Width = desc.Width;
	desc2.Height = desc.Height;
	desc2.MipLevels = desc.MipLevels;
	desc2.ArraySize = desc.ArraySize;
	desc2.Format = desc.Format;
	desc2.SampleDesc = desc.SampleDesc;
	desc2.Usage = D3D11_USAGE_STAGING;
	desc2.BindFlags = 0;
	desc2.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc2.MiscFlags = 0;

	ID3D11Texture2D *stagingTexture = NULL;
	hr = kD3DDevice->CreateTexture2D(&desc2, NULL, &stagingTexture);
	if (SUCCEEDED(hr))
	{
		ID3D11Resource *textureRes = 0, *stagingTextureRes = 0;
		hr = texture->QueryInterface(__uuidof(ID3D11Resource), (void**)&textureRes);
		if (SUCCEEDED(hr))
		{
			hr = stagingTexture->QueryInterface(__uuidof(ID3D11Resource), (void**)&stagingTextureRes);
			if (SUCCEEDED(hr))
			{
				// copy the texture to a staging resource
				kD3DDeviceContext->CopyResource(stagingTextureRes, textureRes);

				// now, map the staging resource
				D3D11_MAPPED_SUBRESOURCE mapInfo;
				hr = kD3DDeviceContext->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &mapInfo);
				if (SUCCEEDED(hr))
				{
					int bufsize = mapInfo.RowPitch * desc.Height;
					LPBYTE buffer = (LPBYTE)malloc(bufsize);

					CLSID encoder;
					GetEncoderClsid(L"image/png", &encoder);
					Bitmap bmp(desc.Width, desc.Height, mapInfo.RowPitch, PixelFormat32bppRGB, buffer);
					memcpy(buffer, mapInfo.pData, bufsize);
					bmp.Save(L"E:\\1.png", &encoder);

					free(buffer);

					kD3DDeviceContext->Unmap(stagingTexture, 0);
				}
				stagingTextureRes->Release();
			}
			textureRes->Release();
		}
		stagingTexture->Release();
	}
}

ID3D11Texture2D *CreateTextureFromMemory(void *data, int width, int height, int stride)
{
	ID3D11Texture2D *texture = 0;

	HRESULT hr = S_OK;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.MiscFlags = 0;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; 
	desc.SampleDesc.Count   = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	
	ID3D11Texture2D *stagingTexture = NULL;
	hr = kD3DDevice->CreateTexture2D(&desc, NULL, &stagingTexture);
	if (SUCCEEDED(hr))
	{
		D3D11_MAPPED_SUBRESOURCE mapInfo;
		hr = kD3DDeviceContext->Map(stagingTexture, 0, D3D11_MAP_WRITE, 0, &mapInfo);
		if (SUCCEEDED(hr))
		{
			char *src = (char*)data, *dst = (char*)mapInfo.pData + mapInfo.RowPitch * (height - 1);
			for(int i = 0; i < height; i++, src += stride, dst -= mapInfo.RowPitch)
			{
				memcpy(dst, src, stride);
			}
			kD3DDeviceContext->Unmap(stagingTexture, 0);
		}

	}				
	
	if(stagingTexture != 0)
	{
		D3D11_TEXTURE2D_DESC textureDesc;   
		textureDesc.Width = width;  
		textureDesc.Height = height;  
		textureDesc.MiscFlags = 0;  
		textureDesc.MipLevels = 1;  
		textureDesc.ArraySize = 1;  
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;  
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;  
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;  
		textureDesc.CPUAccessFlags = 0;  
  
		hr = kD3DDevice->CreateTexture2D(&textureDesc, NULL, &texture);
		if (SUCCEEDED(hr))
		{
			kD3DDeviceContext->CopyResource(texture, stagingTexture);
		}
		stagingTexture->Release();
	}

	return texture;
}

ID3D11Texture2D *CreateTextureFromFile(const wchar_t *file)
{
	Bitmap bmp(file);
	Rect rect(0, 0, bmp.GetWidth(), bmp.GetHeight());
	BitmapData bd;
	if(Ok == bmp.LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, &bd))
	{
		return CreateTextureFromMemory(bd.Scan0, bd.Width, bd.Height, bd.Stride);
	}
	return 0;
}


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	
	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_D3D11TEST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}


	MSG msg = {0};
	while(msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if(PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
		{
            TranslateMessage( &msg );
            DispatchMessage( &msg );
		}
		// Otherwise, do animation/game stuff.
		else
        {	
			kD3DDeviceContext->ClearRenderTargetView(kRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
			kD3DDeviceContext->ClearDepthStencilView(kDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
			
			kD3DDeviceContext->IASetInputLayout(kInputLayout);
			kD3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			UINT stride = sizeof(Vertex);
			UINT offset = 0;
			kD3DDeviceContext->IASetVertexBuffers(0, 1, &kVB, &stride, &offset);
			kD3DDeviceContext->IASetIndexBuffer(kIB, DXGI_FORMAT_R32_UINT, 0);			

			// Set constants	
			XMMATRIX view  = XMLoadFloat4x4(&kView);
			XMMATRIX proj  = XMLoadFloat4x4(&kProj);
			XMMATRIX viewProj = view*proj;
 
			D3DX11_TECHNIQUE_DESC techDesc;
			kTech->GetDesc(&techDesc);
			for(UINT p = 0; p < techDesc.Passes; ++p)
			{
				// Draw the grid.
				fxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&viewProj));
				kTech->GetPassByIndex(p)->Apply(0, kD3DDeviceContext);
				kD3DDeviceContext->PSSetShaderResources(0, 1, &kTexture);
				kD3DDeviceContext->DrawIndexed(6, 0, 0);
			}
			
			if(kCapture)
			{
				kCapture = FALSE;
				SaveTexture(kBackBuffer);
			}
			Sleep(10);
        }
    }

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_D3D11TEST));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_D3D11TEST);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

HRESULT CreateD3DDevice(HWND hWnd)
{
	HRESULT hr = S_FALSE;
	// Driver types supported
	D3D_DRIVER_TYPE DriverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

	// Feature levels supported
	D3D_FEATURE_LEVEL FeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_1
	};
	UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);

	D3D_FEATURE_LEVEL FeatureLevel;

	// Create device
	for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
	{
		hr = D3D11CreateDevice(NULL, DriverTypes[DriverTypeIndex], NULL, 0, FeatureLevels, NumFeatureLevels,
			D3D11_SDK_VERSION, &kD3DDevice, &FeatureLevel, &kD3DDeviceContext);
		if (SUCCEEDED(hr))
		{
			// Device creation success, no need to loop anymore
			break;
		}
	}

	if (SUCCEEDED(hr) && kD3DDevice != NULL && kD3DDeviceContext != NULL)
	{
		// Get DXGI device
		IDXGIDevice *dxgiDevice = NULL;
		hr = kD3DDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
			// Get DXGI adapter
			IDXGIAdapter *dxgiAdapter = NULL;
			hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&dxgiAdapter));
			dxgiDevice->Release();
			dxgiDevice = NULL;

			if (SUCCEEDED(hr))
			{

				IDXGIFactory* dxgiFactory = 0;
				hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);
				if(SUCCEEDED(hr))
				{					
					D3D11_TEXTURE2D_DESC textureDesc;  
					ZeroMemory(&textureDesc, sizeof(textureDesc));  
  
					textureDesc.Width = kWidth;  
					textureDesc.Height = kHeight;  
					textureDesc.MipLevels = 1;  
					textureDesc.ArraySize = 1;  
					textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  //纹理像素为12个字节  
					textureDesc.SampleDesc.Count = 1;  
					textureDesc.Usage = D3D11_USAGE_DEFAULT;  
					textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;  
					textureDesc.CPUAccessFlags = 0;  
					textureDesc.MiscFlags = 0;  
  
					hr = kD3DDevice->CreateTexture2D(&textureDesc, NULL, &kBackBuffer);

					if(SUCCEEDED(hr))
					{
						kD3DDevice->CreateRenderTargetView(kBackBuffer, 0, &kRenderTargetView);

						// Create the depth/stencil buffer and view.

						D3D11_TEXTURE2D_DESC depthStencilDesc;
	
						depthStencilDesc.Width     = kWidth;
						depthStencilDesc.Height    = kHeight;
						depthStencilDesc.MipLevels = 1;
						depthStencilDesc.ArraySize = 1;
						depthStencilDesc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
						depthStencilDesc.SampleDesc.Count   = 1;
						depthStencilDesc.SampleDesc.Quality = 0;
						depthStencilDesc.Usage          = D3D11_USAGE_DEFAULT;
						depthStencilDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
						depthStencilDesc.CPUAccessFlags = 0; 
						depthStencilDesc.MiscFlags      = 0;

						hr = kD3DDevice->CreateTexture2D(&depthStencilDesc, 0, &kDepthStencilBuffer);
						if(SUCCEEDED(hr))
						{
							hr = kD3DDevice->CreateDepthStencilView(kDepthStencilBuffer, 0, &kDepthStencilView);
							if(SUCCEEDED(hr))
							{
								kD3DDeviceContext->OMSetRenderTargets(1, &kRenderTargetView, kDepthStencilView);
								if(SUCCEEDED(hr))
								{
									kScreenViewport.TopLeftX = 0;
									kScreenViewport.TopLeftY = 0;
									kScreenViewport.Width    = kWidth;
									kScreenViewport.Height   = kHeight;
									kScreenViewport.MinDepth = 0.0f;
									kScreenViewport.MaxDepth = 1.0f;
									kD3DDeviceContext->RSSetViewports(1, &kScreenViewport);
								}
							}
						}
					}
				}
			}
		}
	}

	if(FAILED(hr))
	{
	}

	return hr;
}
 
BOOL BuildFX()
{
	std::ifstream fin("color.fxo", std::ios::binary);

	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);

	fin.read(&compiledShader[0], size);
	fin.close();
	
	if(SUCCEEDED(D3DX11CreateEffectFromMemory(&compiledShader[0], size, 0, kD3DDevice, &kFX)))
	{
		kTech = kFX->GetTechniqueByName("ColorTech");
		fxWorldViewProj = kFX->GetVariableByName("gWorldViewProj")->AsMatrix();

		return TRUE;
	}
	return FALSE;
}

BOOL BuildVertexLayout()
{
	// Create the vertex input layout.
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT      , 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	// Create the input layout
    D3DX11_PASS_DESC passDesc;
    kTech->GetPassByIndex(0)->GetDesc(&passDesc);
	if(SUCCEEDED(kD3DDevice->CreateInputLayout(vertexDesc, 3, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &kInputLayout)))
	{
		return TRUE;
	}
	return FALSE;
}


void BuildGeometryBuffers()
{
	Vertex vertices[4] = {
		{XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)}, 
		{XMFLOAT3(-1.0f,  1.0f, 0.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f)}, 
		{XMFLOAT3( 1.0f,  1.0f, 0.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f)}, 
		{XMFLOAT3( 1.0f, -1.0f, 0.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f)}, 
	};

	int indices[6] = {0, 1, 2, 0, 2, 3};

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * 4;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices;
    kD3DDevice->CreateBuffer(&vbd, &vinitData, &kVB);

	D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * 6;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = indices;
    kD3DDevice->CreateBuffer(&ibd, &iinitData, &kIB);
}
 

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		200, 150, 1024, 768, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&kView, I);
	XMStoreFloat4x4(&kProj, I);

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.5f*Pi, 1.0f, 1.0f, 1000.0f);
	XMStoreFloat4x4(&kProj, P);

	// Build the view matrix.
	XMVECTOR pos    = XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&kView, V);

	if(SUCCEEDED(CreateD3DDevice(hWnd)))
	{
		if(BuildFX())
		{
			if(BuildVertexLayout())
			{
				BuildGeometryBuffers();	
		
				ID3D11Texture2D *texture = CreateTextureFromFile(L"test.png");

				if(texture != 0)
				{
					D3D11_SHADER_RESOURCE_VIEW_DESC desc;  
					desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
					desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
					desc.Texture2D.MipLevels = 1; 
					desc.Texture2D.MostDetailedMip = 0;
					kD3DDevice->CreateShaderResourceView(texture, &desc, &kTexture);
				}
			}
		}
	}

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			{
				kCapture = TRUE;
				break;
			}
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
