
#include "Engine.h"
#include <WindowsX.h>
#include <vector>
#include "Texture/TextureRepository.h"
#include "Material/MaterialRepository.h"
#include "Mesh/MeshRepository.h"
#include "World/World.h"
#include "../resource.h"

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before MainWindowHandle is valid.
	return TEngine::GetEngineSingleton()->MsgProc(hwnd, msg, wParam, lParam);
}

TEngine* TEngine::EngineSingleton = nullptr;
TEngine* TEngine::GetEngineSingleton()
{
	return EngineSingleton;
}

TEngine::TEngine(HINSTANCE HInstance)
	: EngineInstHandle(HInstance)
{
	// Only one TEngine can be constructed.
	assert(EngineSingleton == nullptr);
	EngineSingleton = this;
}

TEngine::~TEngine()
{

}

HINSTANCE TEngine::GetEngineInstHandle()const
{
	return EngineInstHandle;
}

HWND TEngine::GetMainWnd()const
{
	return MainWindowHandle;
}

bool TEngine::Initialize(TWorld* InWorld, const TRenderSettings& RenderSettings)
{
	if (!InitMainWindow())
		return false;

	D3D12RHI = std::make_unique<TD3D12RHI>();
	D3D12RHI->Initialize(MainWindowHandle, WindowWidth, WindowHeight);

	TTextureRepository::Get().Load();
	TMaterialRepository::Get().Load();
	TMeshRepository::Get().Load();

	World.reset(InWorld);
	World->InitWorld(this);

	Render = std::make_unique<TRender>();
	if (!Render->Initialize(WindowWidth, WindowHeight, D3D12RHI.get(), World.get(), RenderSettings))
		return false;

	bInitialize = true;

	return true;
}

int TEngine::Run()
{
	MSG msg = { 0 };

	Timer.Reset();

	while (msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Otherwise, do animation/game stuff.
		else
		{
			Timer.Tick();

			if (!bAppPaused)
			{
				CalculateFrameStats();

				Update(Timer);
				
				EndFrame(Timer);
			}
			else
			{
				Sleep(100);
			}
		}
	}

	return (int)msg.wParam;
}

bool TEngine::Destroy()
{
	Render.reset();

	World.reset();

	TTextureRepository::Get().Unload();
	TMaterialRepository::Get().Unload();
	TMeshRepository::Get().Unload();

	D3D12RHI.reset();

	return true;
}


LRESULT TEngine::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (!bInitialize)
	{
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	switch (msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			bAppPaused = true;
			Timer.Stop();
		}
		else
		{
			bAppPaused = false;
			Timer.Start();
		}
		return 0;

		// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions.
		WindowWidth = LOWORD(lParam);
		WindowHeight = HIWORD(lParam);

		if (wParam == SIZE_MINIMIZED)
		{
			bAppPaused = true;
			bAppMinimized = true;
			bAppMaximized = false;
		}
		else if (wParam == SIZE_MAXIMIZED)
		{
			bAppPaused = false;
			bAppMinimized = false;
			bAppMaximized = true;
			OnResize();
		}
		else if (wParam == SIZE_RESTORED)
		{

			// Restoring from minimized state?
			if (bAppMinimized)
			{
				bAppPaused = false;
				bAppMinimized = false;
				OnResize();
			}

			// Restoring from maximized state?
			else if (bAppMaximized)
			{
				bAppPaused = false;
				bAppMaximized = false;
				OnResize();
			}
			else if (bResizing)
			{
				// If user is dragging the resize bars, we do not resize 
				// the buffers here because as the user continuously 
				// drags the resize bars, a stream of WM_SIZE messages are
				// sent to the window, and it would be pointless (and slow)
				// to resize for each WM_SIZE message received from dragging
				// the resize bars.  So instead, we reset after the user is 
				// done resizing the window and releases the resize bars, which 
				// sends a WM_EXITSIZEMOVE message.
			}
			else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
			{
				OnResize();
			}
		}
		
		return 0;

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		bAppPaused = true;
		bResizing = true;
		Timer.Stop();
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		bAppPaused = false;
		bResizing = false;
		Timer.Start();
		OnResize();
		return 0;

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEWHEEL:
		OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
		return 0;
	case WM_KEYUP:
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}
			
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool TEngine::InitMainWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = EngineInstHandle;
	//wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hIcon = LoadIcon(EngineInstHandle, MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"MainWnd";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, WindowWidth, WindowHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	MainWindowHandle = CreateWindow(L"MainWnd", WindowTile.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, EngineInstHandle, 0);
	if (!MainWindowHandle)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(MainWindowHandle, SW_SHOW);
	UpdateWindow(MainWindowHandle);

	return true;
}

void TEngine::OnResize()
{
	if (Render->IsInitialize()) {
		Render->OnResize(WindowWidth, WindowHeight);
	}	
}

void TEngine::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.

	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if ((Timer.TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wstring fpsStr = std::to_wstring(fps);
		std::wstring mspfStr = std::to_wstring(mspf);

		std::wstring windowText = WindowTile +
			L"    fps: " + fpsStr +
			L"   mspf: " + mspfStr;

		SetWindowText(MainWindowHandle, windowText.c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

void TEngine::Update(const GameTimer& gt)
{
	World->Update(Timer);

	Render->Draw(Timer);
}

void TEngine::EndFrame(const GameTimer& gt)
{
	World->EndFrame(gt);

	Render->EndFrame();
}
