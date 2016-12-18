#pragma once
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <string>

#include "Graphics.h"
#include "Camera.h"
#include "Input.h"
#include "RayTracer.h"

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
UINT				WIDTH = 1024;
UINT				HEIGHT = 1024;

#ifdef _DEBUG
ID3D11Debug			*g_debugger; //use this for more detailed graphics debug output
#endif

HINSTANCE			hInst		= NULL;
HWND				hWnd		= NULL;
Graphics			*g			= NULL;
Camera				*camera		= NULL;
Input				*input		= NULL;
RayTracer			*rayTracer	= NULL;
Settings			settings;

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
int	WINAPI			run(HINSTANCE hInstance, HINSTANCE hPrevInst, LPWSTR lpCmdLine, int nCmdShow);
HRESULT				initWindow(HINSTANCE hInstance, int nCmdShow);
LRESULT	CALLBACK	wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void				createConsoleLog(const char *winTitle);
void				update(double dt);
void				draw(double dt);
void				readSettingsFromFile();


//--------------------------------------------------------------------------------------
// Entry point
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPWSTR lpCmdLine, int nCmdShow)
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(630);
#endif
	int temp;
	temp = run(hInstance, hPrevInst, lpCmdLine, nCmdShow);

#ifdef _DEBUG
	g_debugger->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	g_debugger->Release();
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
	MSG msg = { 0 };
	srand(0);
	g = new Graphics();

	if (g->init(&hWnd))
	{
#ifdef _DEBUG
		g->getDevice()->QueryInterface(IID_PPV_ARGS(&g_debugger));
#endif

		input = Input::getInstance();
		input->init(&hInstance, &hWnd);
		camera = new Camera(-10, 0, -50);
		camera->rotate(0, 0.5, 0);
		readSettingsFromFile();
		rayTracer = new RayTracer(settings, g, WIDTH, HEIGHT);
		//set timer
		__int64			currTimeStamp = 0, prevTimeStamp = 0, cntsPerSec = 0;
		QueryPerformanceFrequency((LARGE_INTEGER*)&cntsPerSec);
		double			dt = 0, time = 0;
		double			secsPerCnt = 1.0 / (double)cntsPerSec;

		QueryPerformanceCounter((LARGE_INTEGER*)&currTimeStamp);
		prevTimeStamp = currTimeStamp;
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

				if (GetActiveWindow() == hWnd) //if the window is active
				{
					g->clearRenderTarget(.3f, 0.4f, .2f, 1.0f);
					update(dt);
					draw(dt);
				}

				prevTimeStamp = currTimeStamp;
				//put the delta-time in the window bar
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
		delete rayTracer;
		delete camera;
		delete input;
	}
	delete g;
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
	RECT rc = { 0, 0, (LONG)WIDTH, (LONG)HEIGHT };
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

void update(double dt)
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
		XMStoreFloat3(&movement, XMVector3Normalize(XMLoadFloat3(&movement)) * 0.1f * speed * (float)dt);
	
	if (input->getKeyState()[DIK_ADD] & 0x80 && !(input->getPrevKeyState()[DIK_ADD] & 0x80))
	{
		settings.numLights++;
		if (settings.numLights > 10)
			settings.numLights = 10;
		rayTracer->setSettings(settings, g);
	}
	else if (input->getKeyState()[DIK_SUBTRACT] & 0x80 && !(input->getPrevKeyState()[DIK_SUBTRACT] & 0x80))
	{
		settings.numLights--;
		if (settings.numLights < 0)
			settings.numLights = 0;
		rayTracer->setSettings(settings, g);
	}
	else if (input->getKeyState()[DIK_Z] & 0x80 && !(input->getPrevKeyState()[DIK_Z] & 0x80))
	{
		settings.spotLights = true;
		rayTracer->setSettings(settings, g);
	}
	else if (input->getKeyState()[DIK_X] & 0x80 && !(input->getPrevKeyState()[DIK_X] & 0x80))
	{
		settings.spotLights = false;
		rayTracer->setSettings(settings, g);
	}
	else if (input->getKeyState()[DIK_C] & 0x80 && !(input->getPrevKeyState()[DIK_C] & 0x80))
	{
		settings.shadows = true;
		rayTracer->setSettings(settings, g);
	}
	else if (input->getKeyState()[DIK_V] & 0x80 && !(input->getPrevKeyState()[DIK_V] & 0x80))
	{
		settings.shadows = false;
		rayTracer->setSettings(settings, g);
	}
	else if (input->getKeyState()[DIK_Q] & 0x80 && !(input->getPrevKeyState()[DIK_Q] & 0x80))
	{
		settings.bounces--;
		if (settings.bounces < 0)
			settings.bounces = 0;
		rayTracer->setSettings(settings, g);
	}
	else if (input->getKeyState()[DIK_E] & 0x80 && !(input->getPrevKeyState()[DIK_E] & 0x80))
	{
		settings.bounces++;
		rayTracer->setSettings(settings, g);
	}

	camera->fly(movement);
	if(input->getDeltaX() != 0 || input->getDeltaY() != 0)
		camera->rotate((float)input->getDeltaX()*0.01f, (float)input->getDeltaY()*0.01f, 0);

	rayTracer->updateCamera(camera->getDir(), camera->getPos(), camera->getUp(), g);
	rayTracer->update(dt, g);
}

void draw(double dt)
{
	//g->draw(dt);
	rayTracer->draw(dt, g);
	g->swapPresent(0, 0);
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

void readSettingsFromFile()
{
	FILE *settingsFile;
	fopen_s(&settingsFile, "initialSettings.txt", "r");

	settings.bounces = 0;
	settings.numLights = 1;
	settings.shadows = true;
	settings.spotLights = false;

	if (settingsFile == NULL)
	{
		printf("Unable to open settings file\n");
	}
	else
	{
		bool cont = true;
		while (cont)
		{
			const int headerSize = 128;
			char lineHeader[headerSize];
			// read the first word of the line
			int res = fscanf_s(settingsFile, "%s", lineHeader, headerSize);
			if (res == EOF)
				cont = false; // EOF = End Of File. Quit the loop.
			else
			{
				if (strcmp(lineHeader, "lights") == 0)
				{
					int d = 1;
					fscanf_s(settingsFile, "%d", &d);
					settings.numLights = d;
				}
				else if (strcmp(lineHeader, "bounces") == 0)
				{
					int d = 0;
					fscanf_s(settingsFile, "%d", &d);
					settings.bounces = d;
				}
				else if (strcmp(lineHeader, "wall") == 0)
				{
					int d = 0;
					fscanf_s(settingsFile, "%d", &d);
					settings.wall = d;
				}
				else if (strcmp(lineHeader, "lightType") == 0)
				{
					int d = 0;
					fscanf_s(settingsFile, "%d", &d);
					settings.spotLights = d;
				}
				else if (strcmp(lineHeader, "shadows") == 0)
				{
					int d = 0;
					fscanf_s(settingsFile, "%d", &d);
					settings.shadows = d;
				}
			}
		}
		fclose(settingsFile);
	}
}