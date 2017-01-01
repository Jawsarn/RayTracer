#pragma once
#include "GameOptions.h"
#include <Windows.h>

class InputSystem
{
public:
    static InputSystem* GetInstance();
    static void Startup();
    void Update();





private:
    InputSystem();
    ~InputSystem();

    void GetKeyInputs();
    void GetMouseInputs();

    static InputSystem* m_singleton;
    bool m_prevXPressed;
    bool m_mouseActive;

    int m_resetPosX;
    int m_resetPosY;

    bool lightPressed;
    float cooldown;
};


extern LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

