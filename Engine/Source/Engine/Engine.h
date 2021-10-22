#pragma once

#include "GameTimer.h"
#include "World/World.h"
#include "Render/Render.h"
#include "D3D12/D3D12RHI.h"

class TEngine
{
public:
    TEngine(HINSTANCE HInstance);

    TEngine(const TEngine& rhs) = delete;

    TEngine& operator=(const TEngine& rhs) = delete;

    virtual ~TEngine();

public:
    static TEngine* GetEngineSingleton();

    HINSTANCE GetEngineInstHandle()const;
    HWND      GetMainWnd()const;

    bool Initialize(TWorld* InWorld, const TRenderSettings& RenderSettings);

    int Run();

    bool Destroy();

   LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

   int GetWindowWidth() { return WindowWidth; }

   int GetWindowHeight() { return WindowHeight; }

   TRender* GetRender() { return Render.get(); }

private:
    bool InitMainWindow();

    void OnResize();

    void CalculateFrameStats();


protected:
    void Update(const GameTimer& gt);

    void EndFrame(const GameTimer& gt);
    
    // Convenience overrides for handling mouse input.
    void OnMouseDown(WPARAM btnState, int x, int y) { World->OnMouseDown(btnState, x, y); }
    void OnMouseUp(WPARAM btnState, int x, int y) { World->OnMouseUp(btnState, x, y); }
    void OnMouseMove(WPARAM btnState, int x, int y) { World->OnMouseMove(btnState, x, y); }
    void OnMouseWheel(float WheelDistance) { World->OnMouseWheel(WheelDistance); }

protected:
    static TEngine* EngineSingleton;
    std::wstring WindowTile = L"TotoroEngine";

    bool bInitialize = false;

    HINSTANCE EngineInstHandle = nullptr; // application instance handle
    HWND      MainWindowHandle = nullptr; // main window handle
    bool      bAppPaused = false;  // is the application paused?
    bool      bAppMinimized = false;  // is the application minimized?
    bool      bAppMaximized = false;  // is the application maximized?
    bool      bResizing = false;   // are the resize bars being dragged?
    bool      bFullscreenState = false;// fullscreen enabled

    int WindowWidth = 1280;
    int WindowHeight = 720;

    GameTimer Timer;

    std::unique_ptr<TD3D12RHI> D3D12RHI;

    std::unique_ptr<TWorld> World;

    std::unique_ptr<TRender> Render; 
};

