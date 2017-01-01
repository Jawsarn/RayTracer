#include "LightManager.h"
#include "GraphicsEngine.h"

LightManager* LightManager::m_singleton = nullptr;

LightManager::LightManager()
{
    m_lights.push_back(Light(XMFLOAT3(1, 0, 0), 15, XMFLOAT3(1, 1, 1)));
    m_lights.push_back(Light(XMFLOAT3(0, 0, 1), 15, XMFLOAT3(1, 1, 1)));

    for (size_t i = 0; i < m_lights.size(); i++)
    {
        GraphicsEngine::GetInstance()->CreatePointLight(m_lights[i].pos, m_lights[i].radius, m_lights[i].color);
    }
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
    m_lights.push_back(Light(XMFLOAT3(1, 0, 0), 15, XMFLOAT3(1, 1, 1)));
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
