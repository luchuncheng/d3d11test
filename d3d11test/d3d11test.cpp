// d3d11test.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "d3d11test.h"
#include <d3d11.h>
#include <xnamath.h>

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

ID3D11Device* kD3DDevice = NULL;
ID3D11DeviceContext* kD3DDeviceContext = NULL;
IDXGISwapChain* kSwapChain = NULL;
ID3D11Texture2D* kBackBuffer = NULL;
ID3D11Texture2D* kDepthStencilBuffer = NULL;
ID3D11RenderTargetView* kRenderTargetView = NULL;
ID3D11DepthStencilView* kDepthStencilView = NULL;
D3D11_VIEWPORT kScreenViewport;

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

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	
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
			kD3DDeviceContext->ClearRenderTargetView(kRenderTargetView, reinterpret_cast<const float*>(&Colors::Blue));
			kD3DDeviceContext->ClearDepthStencilView(kDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
			kSwapChain->Present(0, 0);
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
	HRESULT hr = S_OK;
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
					RECT rect;
					GetClientRect(hWnd, &rect);
					int width = rect.right - rect.left, height = rect.bottom - rect.top;

					DXGI_SWAP_CHAIN_DESC sd;
					sd.BufferDesc.Width  = width;
					sd.BufferDesc.Height = height;
					sd.BufferDesc.RefreshRate.Numerator = 60;
					sd.BufferDesc.RefreshRate.Denominator = 1;
					sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
					sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

					sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
					sd.SampleDesc.Count   = 1;
					sd.SampleDesc.Quality = 0;

					sd.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
					sd.BufferCount  = 1;
					sd.OutputWindow = hWnd;
					sd.Windowed     = true;
					sd.SwapEffect   = DXGI_SWAP_EFFECT_DISCARD;
					sd.Flags        = 0;

					hr = dxgiFactory->CreateSwapChain(kD3DDevice, &sd, &kSwapChain);
					if(SUCCEEDED(hr))
					{
						hr = kSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&kBackBuffer));
						if(SUCCEEDED(hr))
						{
							kD3DDevice->CreateRenderTargetView(kBackBuffer, 0, &kRenderTargetView);

							// Create the depth/stencil buffer and view.

							D3D11_TEXTURE2D_DESC depthStencilDesc;
	
							depthStencilDesc.Width     = width;
							depthStencilDesc.Height    = height;
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
										kScreenViewport.Width    = static_cast<float>(width);
										kScreenViewport.Height   = static_cast<float>(height);
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
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
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
