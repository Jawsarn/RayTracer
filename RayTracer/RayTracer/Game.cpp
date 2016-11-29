#include "Game.h"
#include "InputSystem.h"
#include "GraphicsEngine.h"
#include "TimeSystem.h"
#include "CameraManager.h"
#include "GameOptions.h"
#include <iostream>

Game::Game()
{
}


Game::~Game()
{
}

void Game::Startup(HINSTANCE p_hInstance, int p_nCmdShow)
{
    CameraManager* camMan = CameraManager::GetInstance();
    camMan->SetPerspective(XM_PIDIV4, (float)(WINDOW_SIZE_X), (float)(WINDOW_SIZE_Y), 0.1f, 1000000.0f);
    camMan->LookTo(XMFLOAT3(2, 0, -8), XMFLOAT3(-1, 0, 3), XMFLOAT3(0, 1, 0));

    InputSystem::Startup();
    m_inputSystem = InputSystem::GetInstance();
    
    GraphicsEngine::Startup(p_hInstance, p_nCmdShow, WndProc);
    m_graphicsEngine = GraphicsEngine::GetInstance();

    TimeSystem::Startup();


    GameObject newObj;
    m_graphicsEngine->LoadObject("../../sword/Sword.obj");
    DirectX::XMStoreFloat4x4(&newObj.world, DirectX::XMMatrixIdentity());
    newObj.objectID = m_graphicsEngine->AddToRender(newObj.world, "../../sword/Sword.obj");

    m_graphicsEngine->CreateSphere(XMFLOAT3(-5, 0, 0), 3, XMFLOAT3(1, 1, 1));
}

void Game::Run()
{
    TimeSystem* timeSystem = TimeSystem::GetInstance();

    timeSystem->StartClock();

    while (true)
    {
        // Tick time
        timeSystem->Tick();

        // Loop as many update-steps we will take this frame
        while (timeSystem->ShouldUpdateFrame())
        {
            // Update game based on state
            Update(timeSystem->GetStepLength());

            // Update accumulator and gametime
            timeSystem->UpdateAccumulatorAndGameTime();
        }

        // Update alpha used for inteprolation
        double alpha = timeSystem->GetFrameAlpha();

        // Update camera after we update positions
        Render();
    }
}

void Game::Update(double p_stepLength)
{
    m_inputSystem->Update();
}

void Game::Render()
{
    m_graphicsEngine->Render();
}
