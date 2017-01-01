#include "InputSystem.h"
#include "CameraManager.h"
#include "LightManager.h"
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
    m_prevXPressed = false;
    lightPressed = false;
    m_mouseActive = false;
    m_resetPosX = WINDOW_SIZE_X/2;
    m_resetPosY = WINDOW_SIZE_Y/2;
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

    GetKeyInputs();
    GetMouseInputs();
    if (lightPressed)
    {
        cooldown -= 17;

        if (cooldown <= 0)
            lightPressed = false;
    }
}

void InputSystem::GetKeyInputs()
{
    CameraManager* camMan = CameraManager::GetInstance();
    LightManager* lightMan = LightManager::GetInstance();

    float moveAmount = 0.5f;
    // W
    if (GetAsyncKeyState(0x57))
    {
        camMan->Walk(moveAmount);
    }
    if (GetAsyncKeyState(0x41)) // A
    {
        camMan->Strafe(-moveAmount);
    }
    if (GetAsyncKeyState(0x44)) // D
    {
        camMan->Strafe(moveAmount);
    }
    if (GetAsyncKeyState(0x53)) // S
    {
        camMan->Walk(-moveAmount);
    }
    if (GetAsyncKeyState(0x51)) // Q
    {
        camMan->HoverY(-moveAmount);
    }
    if (GetAsyncKeyState(0x45)) // E
    {
        camMan->HoverY(moveAmount);
    }

    if (GetAsyncKeyState(0x5A) && !lightPressed) // Z
    {
        lightMan->AddLight();
        lightPressed = true;
        cooldown = 1000;
    }
    if (GetAsyncKeyState(0x58) && !lightPressed) // X
    {
        lightMan->RemoveLight();
        lightPressed = true;
        cooldown = 1000;
    }


    // X for activating
    if (GetAsyncKeyState(0x43) && !m_prevXPressed)
    {
        m_mouseActive = !m_mouseActive;
        ShowCursor(!m_mouseActive);
        if (m_mouseActive)
        {
            // Set cursor position to middle to not give movement directly
            SetCursorPos(m_resetPosX, m_resetPosY);
        }
    }

    m_prevXPressed = GetAsyncKeyState(0x43);
}

void InputSystem::GetMouseInputs()
{
    if(m_mouseActive)
    {
        CameraManager* camMan = CameraManager::GetInstance();

        POINT t_curPoint;
        if (GetCursorPos(&t_curPoint))
        {
            int diffX = m_resetPosX - t_curPoint.x;
            int diffY = m_resetPosY - t_curPoint.y;

            camMan->Pitch(-diffY*MOUSE_SENSE);
            camMan->RotateY(-diffX*MOUSE_SENSE);

            // Reset pos
            SetCursorPos(m_resetPosX, m_resetPosY);
        }
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{


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
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}