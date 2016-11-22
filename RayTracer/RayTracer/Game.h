#pragma once
#include <Windows.h>
#include <stdint.h>
#include <DirectXMath.h>
#include <vector>

class InputSystem;
class GraphicsEngine;

struct GameObject
{
    DirectX::XMFLOAT4X4 world;
    uint32_t objectID;
};

class Game
{
public:
    Game();
    ~Game();

    void Startup(HINSTANCE p_hInstance, int p_nCmdShow);
    void Run();

private:

    void Update(double p_stepLength);
    void Render();

    InputSystem* m_inputSystem = nullptr;
    GraphicsEngine* m_graphicsEngine = nullptr;

    std::vector<GameObject> m_objects;
};

