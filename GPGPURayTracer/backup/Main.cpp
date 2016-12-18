#pragma once
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "Graphics.h"
#include "ObjLoader.h"
#include "Camera.h"
#include "ShaderWrapping.h"
#include "Model3D.h"
#include "Input.h"
//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE			hInst	= NULL;
HWND				hWnd	= NULL;
Graphics			*g		= NULL;
Camera				*camera = NULL;
Input				*input	= NULL;

CBOnce				cbOnce;
CBCamera			cbCamera;
CBPointLight		cbLights;
CBVertexInfo		cbVertexInfo;

//shaders
int					csBasicRaysIndex;
int					csIntersectionsIndex;
int					csColorsIndex;
//Buffers
int					boxBufferIndex;
int					sphereBufferIndex;
//UAVs
int					uavSceneIndex = 0;
int					uavSpheresIndex;
//int					uavBufferIndex;

//SRVs
int					srvBufferIndex;

#define				CBONCE (0)
#define				CBCAMERA (1)
#define				CBVERTEXINFO (2)
#define				CBLIGHTS (3)

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
int	WINAPI			run(HINSTANCE hInstance, HINSTANCE hPrevInst, LPWSTR lpCmdLine, int nCmdShow);
HRESULT				initWindow(HINSTANCE hInstance, int nCmdShow);
LRESULT	CALLBACK	wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void				cleanUp();
void				createConsoleLog(const char *winTitle);
void				update(float dt);
void				draw(float dt);


//--------------------------------------------------------------------------------------
// Entry point
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPWSTR lpCmdLine, int nCmdShow)
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc( 236 );
#endif
	int temp;
	temp = run(hInstance, hPrevInst, lpCmdLine, nCmdShow);

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif

	return temp;
}

int WINAPI run(HINSTANCE hInstance, HINSTANCE hPrevInst, LPWSTR lpCmdLine, int nCmdShow)
{
	createConsoleLog("Output Console");

	if (FAILED(initWindow(hInstance, nCmdShow)))
	{
		MessageBox(0, "Error initializing window!", 0, 0);
		return 0;
	}

	g = new Graphics();
	g->init(&hWnd);
	csBasicRaysIndex		= g->createComputeShader( L"Shaders/RayTracer.hlsl", "csGeneratePrimaryRays" );
	csIntersectionsIndex	= g->createComputeShader( L"Shaders/RayTracer.hlsl", "csComputeIntersections" );
	csColorsIndex			= g->createComputeShader( L"Shaders/RayTracer.hlsl", "csComputeColor" );

	input = Input::getInstance();
	input->init(&hInstance, &hWnd);

	//cbuffer mapping
	UINT cbufferDataSizes[4] =
	{
		sizeof(float) * 4,					//cbOnce
		sizeof(float) * 12,					//cbCamera
		sizeof(int) + sizeof(float) * 3,	//cbVertexInfo
		sizeof(float) * 83 + sizeof(int)	//cbLightSources
	};

	//initialize all cbuffers
	for (unsigned int i = 0; i < 4; i++)
	{
		if (FAILED(g->createCBuffer(cbufferDataSizes[i], i)))
		{
			MessageBox(0, "Error creating cBuffer!", 0, 0);
			return -1;
		}
	}
	ObjLoader objLoader;
	//load model
	Model3D *box = new Model3D(XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1));
	objLoader.loadModel(box, "cube/boax.obj");
	g->createShaderResourceViewFromFile(&box->textureIndices, box->texturePaths);
	int vertexAmount = box->vertices.size();

	/*if (!g->createBuffer_UAV(boxBufferIndex, uavBufferIndex, &(*box->getVertices())[0], sizeof(MeshVertex), vertexAmount))
	{
		MessageBox(0, "Error creating shader resource buffer and/or view", 0, 0);
		return -1;
	}*/
	if (!g->createStructuredBuffer(boxBufferIndex, srvBufferIndex, vertexAmount, false, false, &(box->vertices)[0], sizeof(MeshVertex)))
	{
		MessageBox(0, "Error creating structured buffer buffer and/or view", 0, 0);
		return -1;
	}

	cbVertexInfo.noVertices = vertexAmount;

	camera	= new Camera(-10, 0, -50);

	cbCamera.dir = camera->getDir();
	cbCamera.pos = camera->getPos();
	cbCamera.up = camera->getUp();
	cbOnce.resolution = DirectX::XMFLOAT4(800, 600, 0, 0);
	cbLights.noLights = 5;
	cbLights.positions[0] = XMFLOAT4(-40, 50, 20, 1);
	cbLights.positions[1] = XMFLOAT4(40, 100, -20, 1);
	cbLights.positions[2] = XMFLOAT4(-50, 60, 30, 1);
	cbLights.positions[3] = XMFLOAT4(33, 33, 33, 1);
	cbLights.positions[4] = XMFLOAT4(-100, -50, 0, 1);
	/*cbLights.positions[5] = XMFLOAT4(-30, -10, -70, 1);
	cbLights.positions[6] = XMFLOAT4(-70, 0, -10, 1);
	cbLights.positions[7] = XMFLOAT4(0, 200, 0, 1);
	cbLights.positions[8] = XMFLOAT4(-50, 30, 40, 1);
	cbLights.positions[9] = XMFLOAT4(40, -40, 10, 1);*/
	cbLights.colorReach[0] = XMFLOAT4(1, 1, 0, 1000);
	cbLights.colorReach[1] = XMFLOAT4(1, 1, 1, 1000);
	cbLights.colorReach[2] = XMFLOAT4(0, 1, 1, 1000);
	cbLights.colorReach[3] = XMFLOAT4(1, 0, 1, 1000);
	cbLights.colorReach[4] = XMFLOAT4(0.5, 0.5, 0.5, 1000);
	/*cbLights.colorReach[5] = XMFLOAT4(1, 1, 1, 1000);
	cbLights.colorReach[6] = XMFLOAT4(1, 1, 1, 1000);
	cbLights.colorReach[7] = XMFLOAT4(0, 0, 1, 1000);
	cbLights.colorReach[8] = XMFLOAT4(1, 0, 0, 1000);
	cbLights.colorReach[9] = XMFLOAT4(0, 1, 0, 1000);*/

	g->updateSubResource(CBONCE, 0, NULL, &cbOnce, 0, 0);
	g->updateSubResource(CBCAMERA, 0, NULL, &cbCamera, 0, 0);
	g->updateSubResource(CBVERTEXINFO, 0, NULL, &cbVertexInfo, 0, 0);
	g->updateSubResource(CBLIGHTS, 0, NULL, &cbLights, 0, 0);
	g->setUnorderedAccessViews(0, 1, uavSceneIndex, NULL);
	//g->setUnorderedAccessViews(1, 1, uavBufferIndex, NULL);
	g->setShaderResourceView(1, 1, srvBufferIndex);
	g->setShaderResourceView(0, 1, box->textureIndices[0]);

	__int64			currTimeStamp = 0, prevTimeStamp = 0, cntsPerSec = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&cntsPerSec);
	double			dt = 0, time = 0;
	double			secsPerCnt = 1.0 / (double)cntsPerSec;

	QueryPerformanceCounter((LARGE_INTEGER*)&currTimeStamp);
	prevTimeStamp = currTimeStamp;
	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			__int64 currTimeStamp = 0;
			QueryPerformanceCounter((LARGE_INTEGER*)&currTimeStamp);
			double dt = (currTimeStamp - prevTimeStamp) * secsPerCnt;	//time since last frame

			if (GetActiveWindow() == hWnd)
			{
				g->clearRenderTarget(.3f, 0.4f, .2f, 1.0f);
				// run stuff here
				update(dt);
				draw(dt);
			}

			prevTimeStamp = currTimeStamp;

			char title[256];
			sprintf_s(
				title,
				sizeof(title),
				"RayTracer dt: %f",
				dt
			);
			SetWindowText(hWnd, title);
		}
	}
	delete box;
	delete g;
	delete camera;
	delete input;

	return (int)msg.wParam;
}

HRESULT initWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = wndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = 0;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "RayTracer";
	wcex.hIconSm = 0;
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	// Create window
	hInst = hInstance;
	RECT rc = { 0, 0, 800, 600 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	if (!(hWnd = CreateWindow("RayTracer",
		"Initializing",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		NULL,
		NULL,
		hInstance,
		NULL)))
	{
		return E_FAIL;
	}

	ShowWindow(hWnd, nCmdShow);

	return S_OK;
}

LRESULT CALLBACK wndProc(HWND _hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(_hWnd, &ps);
		EndPaint(_hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(_hWnd, message, wParam, lParam);
	}

	return 0;
}

void update(float dt)
{
	input->update();
	XMFLOAT3 movement = XMFLOAT3(0, 0, 0);

	float speed;
	if (input->getKeyState()[DIK_LSHIFT] & 0x80)
		speed = 25.0f;
	else
		speed = 5.0f;
	if (input->getKeyState()[DIK_W] & 0x80)
		movement.z++;
	if (input->getKeyState()[DIK_S] & 0x80)
		movement.z--;
	if (input->getKeyState()[DIK_A] & 0x80)
		movement.x++;
	if (input->getKeyState()[DIK_D] & 0x80)
		movement.x--;
	if (input->getKeyState()[DIK_SPACE] & 0x80)
		movement.y++;
	if (input->getKeyState()[DIK_LCONTROL] & 0x80)
		movement.y--;
	if (input->getKeyState()[DIK_ESCAPE] & 0x80)
		PostQuitMessage(0);
	if (!(movement.x == 0 && movement.y == 0 && movement.z == 0))
		XMStoreFloat3(&movement, XMVector3Normalize(XMLoadFloat3(&movement)) * 0.1f * speed * dt);

	camera->fly(movement);
	if(input->getDeltaX() != 0 || input->getDeltaY() != 0)
		camera->rotate((float)input->getDeltaX()*0.01f, (float)input->getDeltaY()*0.01f, 0);

	cbCamera.dir = camera->getDir();
	cbCamera.pos = camera->getPos();
	cbCamera.up = camera->getUp();

	g->updateSubResource( 1, 0, NULL, &cbCamera, 0, 0 );


}

void draw(float dt)
{
	//g->draw(dt);
	g->setShader(0, 0);
	g->dispatch(32, 32, 1);
	g->unsetShader(0);
	g->swapPresent(0, 0);
}

void cleanUp()
{
}

void createConsoleLog(const char *winTitle)
{
	AllocConsole();
	SetConsoleTitle(winTitle);

	int hConHandle;
	long lStdHandle;
	FILE fp;

	// redirect unbuffered STDOUT to the console
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = *_fdopen(hConHandle, "w");
	*stdout = fp;
	setvbuf(stdout, NULL, _IONBF, 0);

	// redirect unbuffered STDIN to the console
	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = *_fdopen(hConHandle, "r");
	*stdin = fp;
	setvbuf(stdin, NULL, _IONBF, 0);

	// redirect unbuffered STDERR to the console
	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = *_fdopen(hConHandle, "w");
	*stderr = fp;
	setvbuf(stderr, NULL, _IONBF, 0);
}