#include "LightManager.h"
#include "GraphicsEngine.h"

LightManager* LightManager::m_singleton = nullptr;

LightManager::LightManager()
{
    m_lightIntensity = 0.20f;
    m_lightRange = 20;
    m_lights.push_back(Light(XMFLOAT3(1.8f, 0, 0), m_lightRange, XMFLOAT3(m_lightIntensity, m_lightIntensity, m_lightIntensity)));

    for (size_t i = 0; i < m_lights.size(); i++)
    {
        GraphicsEngine::GetInstance()->CreatePointLight(m_lights[i].pos, m_lights[i].radius, m_lights[i].color);
    }

    GraphicsEngine::GetInstance()->CreateSpotLight(XMFLOAT3(0, 1.0f,0), XMFLOAT3(0,-1,0), 20, XMFLOAT3(1,1,1), 10);
}


LightManager::~LightManager()
{
}

LightManager * LightManager::GetInstance()
{
    if (m_singleton == nullptr)
    {
        m_singleton = new LightManager();
    }
    return m_singleton;
}

void LightManager::AddLight()
{
    m_lights.push_back(Light(XMFLOAT3(1.8f, 0, 0), m_lightRange, XMFLOAT3(m_lightIntensity, m_lightIntensity, m_lightIntensity)));
    int index = m_lights.size() - 1;
    GraphicsEngine::GetInstance()->CreatePointLight(m_lights[index].pos, m_lights[index].radius, m_lights[index].color);
}

void LightManager::RemoveLight()
{
    if (m_lights.size() == 0)
        return;

    m_lights.erase(m_lights.end() - 1);
    GraphicsEngine::GetInstance()->RemovePointLight();
}

void LightManager::Update()
{
    static float rotation = 0.017f;

    for (size_t i = 0; i < m_lights.size(); i++)
    {
        XMStoreFloat3(&m_lights[i].pos, XMVector3Transform(XMLoadFloat3(&m_lights[i].pos), XMMatrixRotationY(rotation)));
        GraphicsEngine::GetInstance()->UpdatePointLight(m_lights[i].pos, m_lights[i].radius, m_lights[i].color, i);
    }
}
