#pragma once
#include <DirectXMath.h>
#include <vector>

using namespace DirectX;

struct Light
{
    Light(XMFLOAT3 p_pos, float p_radius, XMFLOAT3 p_color)
    {
        pos = p_pos;
        radius = p_radius;
        color = p_color;
    }
    XMFLOAT3 pos;
    float radius;
    XMFLOAT3 color;
};

class LightManager
{
public:
    static LightManager* GetInstance();

    void AddLight();
    void RemoveLight();

    void Update();
private:
    static LightManager* m_singleton;
    std::vector<Light> m_lights;
 
    LightManager();
    ~LightManager();
    float m_lightIntensity;
};

