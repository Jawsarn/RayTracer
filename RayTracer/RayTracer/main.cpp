//--------------------------------------------------------------------------------------
// File: TemplateMain.cpp
//
// BTH-D3D-Template
//
// Copyright (c) Stefan Petersson 2013. All rights reserved.
//--------------------------------------------------------------------------------------


#include "Game.h"
#include <Windows.h>

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR cmdLine, _In_ int nCmdShow)
{
    Game* game = new Game();

    game->Startup(hInstance, nCmdShow);
    game->Run();
}

