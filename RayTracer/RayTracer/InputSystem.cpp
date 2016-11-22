#include "InputSystem.h"
#include "CameraManager.h"
#include <stdexcept>



InputSystem* InputSystem::m_singleton = nullptr;

InputSystem * InputSystem::GetInstance()
{
    if (m_singleton == nullptr)
        throw std::runtime_error("Non-initialized input system received");

    return m_singleton;
}

void InputSystem::Startup()
{
    if (m_singleton != nullptr)
        throw std::runtime_error("Startup called multiple times");

    m_singleton = new InputSystem();
}

InputSystem::InputSystem()
{

}


InputSystem::~InputSystem()
{

}

void InputSystem::Update()
{
    MSG msg = { 0 };
    while (true)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            break;
        }
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CameraManager* camMan = CameraManager::GetInstance();
    int moveAmount = 100;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_KEYDOWN:


        switch (wParam)
        {
        case VK_ESCAPE:
            PostQuitMessage(0);
            break;
        }
        // W
        if (GetAsyncKeyState(0x57))
        {
            camMan->Walk(moveAmount);
        }
        else if (GetAsyncKeyState(0x41)) // A
        {
            camMan->Strafe(moveAmount);
        }
        else if (GetAsyncKeyState(0x44)) // D
        {
            camMan->Strafe(-moveAmount);
        }
        else if (GetAsyncKeyState(0x53)) // S
        {
            camMan->Walk(-moveAmount);
        }

        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}