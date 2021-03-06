#include "GraphicsEngine.h"
#include "ObjLoader.h"
#include <stdexcept>
#include "DDSTextureLoader.h"
#include "CameraManager.h"
#include "GameOptions.h"
#include "Timer.h"
#include <iostream>
#include <fstream>


GraphicsEngine* GraphicsEngine::m_singleton = nullptr;

GraphicsEngine * GraphicsEngine::GetInstance()
{
    if (m_singleton == nullptr)
        throw std::runtime_error("Non-initialized graphic engine received");

    return m_singleton;
}

void GraphicsEngine::Startup(HINSTANCE p_hInstance, int p_nCmdShow, WNDPROC p_winProc)
{
    if (m_singleton != nullptr)
        throw std::runtime_error("Startup called multiple times");

    m_singleton = new GraphicsEngine(p_hInstance, p_nCmdShow, p_winProc);
}

GraphicsEngine::GraphicsEngine(HINSTANCE p_hInstance, int p_nCmdShow, WNDPROC p_winProc)
{
    HRESULT hr S_OK;
    m_handleInstance = p_hInstance;

    hr = InitializeWindow(p_nCmdShow, p_winProc);
    if (FAILED(hr))
        throw std::runtime_error("Startup error");
    
    hr = InitializeDirectX();
    if(FAILED(hr))
        throw std::runtime_error("Startup error");

    hr = InitializeBackBuffer();
    if (FAILED(hr))
        throw std::runtime_error("Startup error");

    //create helper sys and compute shader instance
    m_computeWrapper = new ComputeWrap(m_device, m_deviceContext);
    m_objLoader = new ObjLoader();
    
    hr = InitializeShaders();
    if (FAILED(hr))
        throw std::runtime_error("Startup error");


    hr = InitializeBuffers();
    if (FAILED(hr))
        throw std::runtime_error("Startup error");

    hr = InitializeTextureArrays();
    if (FAILED(hr))
        throw std::runtime_error("Startup error");

    hr = InitializeSamplers();

    m_numBounces = 0;
    m_curDiffuseIndex = 0;
    m_curNormalIndex = 0;
    m_numVertices = 0;
    printFrame = false;
}


GraphicsEngine::~GraphicsEngine()
{
}

char* FeatureLevelToString(D3D_FEATURE_LEVEL featureLevel)
{
    if (featureLevel == D3D_FEATURE_LEVEL_11_1)
        return "11.1";
    if (featureLevel == D3D_FEATURE_LEVEL_11_0)
        return "11.0";
    if (featureLevel == D3D_FEATURE_LEVEL_10_1)
        return "10.1";
    if (featureLevel == D3D_FEATURE_LEVEL_10_0)
        return "10.0";

    return "Unknown";
}

char* DriverTypeToString(D3D_DRIVER_TYPE driverType)
{
    if (driverType == D3D_DRIVER_TYPE_HARDWARE)
        return "Hardware";
    if (driverType == D3D_DRIVER_TYPE_REFERENCE)
        return "Reference";
    if (driverType == D3D_DRIVER_TYPE_NULL)
        return "Null";
    if (driverType == D3D_DRIVER_TYPE_SOFTWARE)
        return "Software";
    if (driverType == D3D_DRIVER_TYPE_WARP)
        return "Warp";
        
    return "Unknown";
}

HRESULT GraphicsEngine::InitializeWindow(int p_nCmdShow, WNDPROC p_winProc)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = p_winProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = m_handleInstance;
    wcex.hIcon = 0;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = _T("BTH_D3D_Template");
    wcex.hIconSm = 0;
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    RECT rc = { 0, 0, WINDOW_SIZE_X, WINDOW_SIZE_Y};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    if (!(m_handleWindow = CreateWindow(
        _T("BTH_D3D_Template"),
        _T("BTH - Direct3D 11.0 Template"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rc.right - rc.left,
        rc.bottom - rc.top,
        NULL,
        NULL,
        m_handleInstance,
        NULL)))
    {
        return E_FAIL;
    }

    ShowWindow(m_handleWindow, p_nCmdShow);

    return S_OK;
}

HRESULT GraphicsEngine::InitializeDirectX()
{
    HRESULT hr = S_OK;;

    // Get size of window
    RECT rec;
    GetClientRect(m_handleWindow, &rec);
    m_width = rec.right - rec.left;;
    m_height = rec.bottom - rec.top;

    // Add debug flag
    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // Create swap chain
    DXGI_SWAP_CHAIN_DESC swapDesc;
    ZeroMemory(&swapDesc, sizeof(swapDesc));
    swapDesc.BufferCount = 1;
    swapDesc.BufferDesc.Width = m_width;
    swapDesc.BufferDesc.Height = m_height;
    swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
    swapDesc.OutputWindow = m_handleWindow;
    swapDesc.SampleDesc.Count = 1;
    swapDesc.SampleDesc.Quality = 0;
    swapDesc.Windowed = TRUE;

    // Get best drivers and featurelevel for run
    D3D_FEATURE_LEVEL initiatedFeatureLevel;
    D3D_FEATURE_LEVEL featureLevelsToTry[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    D3D_DRIVER_TYPE driverType;
    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_REFERENCE,
        D3D_DRIVER_TYPE_NULL,
        D3D_DRIVER_TYPE_SOFTWARE,
        D3D_DRIVER_TYPE_WARP
    };

    // Try create swap with driver and feature
    for (UINT driverTypeIndex = 0; driverTypeIndex < ARRAYSIZE(driverTypes); driverTypeIndex++)
    {
        driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(
            NULL,
            driverType,
            NULL,
            createDeviceFlags,
            featureLevelsToTry,
            ARRAYSIZE(featureLevelsToTry),
            D3D11_SDK_VERSION,
            &swapDesc,
            &m_swapChain,
            &m_device,
            &initiatedFeatureLevel,
            &m_deviceContext);

        // Set name of window
        if (SUCCEEDED(hr))
        {
            char title[256];
            sprintf_s(
                title,
                sizeof(title),
                "DX Feature level: %s, with driver of %s",
                FeatureLevelToString(initiatedFeatureLevel), DriverTypeToString(driverType)
            );
            SetWindowTextA(m_handleWindow, title);
            break;
        }
    }

    return hr;
}

HRESULT GraphicsEngine::InitializeBackBuffer()
{
    HRESULT hr;
    // Create a render target view
    ID3D11Texture2D* backBuffer;
    hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    if (FAILED(hr))
        return hr;

    // create shader unordered access view on back buffer for compute shader to write into texture
    hr = m_device->CreateUnorderedAccessView(backBuffer, NULL, &m_backBufferUAV);

    return hr;
}

HRESULT GraphicsEngine::InitializeShaders()
{
    m_initRaysShader = m_computeWrapper->CreateComputeShader(_T("InitRaysCS.hlsl"), NULL, "CS", NULL);
    m_createRaysShader = m_computeWrapper->CreateComputeShader(_T("CreateRaysCS.hlsl"), NULL, "CS", NULL);
    m_intersectionShader = m_computeWrapper->CreateComputeShader(_T("IntersectionCS.hlsl"), NULL, "CS", NULL);
    m_coloringShader = m_computeWrapper->CreateComputeShader(_T("ColoringCS.hlsl"), NULL, "CS", NULL);
    m_lightCalcShader = m_computeWrapper->CreateComputeShader(_T("LightCalcCS.hlsl"), NULL, "CS", NULL);
    m_ssShader = m_computeWrapper->CreateComputeShader(_T("SuperSampleCS.hlsl"), NULL, "CS", NULL);

    return S_OK;
}

HRESULT GraphicsEngine::InitializeBuffers()
{
    m_rayBuffer = m_computeWrapper->CreateBuffer(COMPUTE_BUFFER_TYPE::STRUCTURED_BUFFER, sizeof(Ray), WINDOW_DRAW_SIZE_X*WINDOW_DRAW_SIZE_Y, true, true, nullptr);
    m_colorDataBuffer = m_computeWrapper->CreateBuffer(COMPUTE_BUFFER_TYPE::STRUCTURED_BUFFER, sizeof(ColorData), WINDOW_DRAW_SIZE_X*WINDOW_DRAW_SIZE_Y, true, true, nullptr);

    ConstantBuffer constBuf;
    constBuf.Proj = XMMatrixTranspose(CameraManager::GetInstance()->GetProj());
    constBuf.ScreenDimensions = XMUINT2(WINDOW_DRAW_SIZE_X, WINDOW_DRAW_SIZE_Y);
    constBuf.DoubleScreenByDimension = XMFLOAT2((2.0f /(float)WINDOW_DRAW_SIZE_X), -(2.0f / (float)WINDOW_DRAW_SIZE_Y));
    

    m_constantBuffer = m_computeWrapper->CreateConstantBuffer(sizeof(ConstantBuffer), &constBuf, D3D11_USAGE::D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE);

    m_perFrameBuffer = m_computeWrapper->CreateConstantBuffer(sizeof(PerFramebuffer), nullptr, D3D11_USAGE::D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE);

    // Set the buffers
    m_deviceContext->CSSetConstantBuffers(0, 1, &m_constantBuffer);
    m_deviceContext->CSSetConstantBuffers(1, 1, &m_perFrameBuffer);

    return S_OK;
}

HRESULT GraphicsEngine::InitializeTextureArrays()
{
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = 1024;
    desc.Height = 1024;
    desc.MipLevels = 1;
    desc.ArraySize = 50;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    HRESULT hr = m_device->CreateTexture2D(&desc, nullptr, &m_diffuseTextureArray);
    if (FAILED(hr))
    {
        throw std::runtime_error("Error loading texture");
    }

    hr = m_device->CreateTexture2D(&desc, nullptr, &m_normalTextureArray);
    if (FAILED(hr))
    {
        throw std::runtime_error("Error loading texture");
    }

    D3D11_TEXTURE2D_DESC td;
    m_diffuseTextureArray->GetDesc(&td);

    //init view description
    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
    ZeroMemory(&viewDesc, sizeof(viewDesc));

    viewDesc.Format = td.Format;
    viewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2DARRAY;
    viewDesc.Texture2D.MipLevels = td.MipLevels;
    viewDesc.Texture2DArray.ArraySize = td.ArraySize;

    hr = m_device->CreateShaderResourceView(m_diffuseTextureArray, &viewDesc, &m_diffuseTextureArraySRV);
    if (FAILED(hr))
    {
        throw std::runtime_error("Error loading texture");
    }

    hr = m_device->CreateShaderResourceView(m_normalTextureArray, &viewDesc, &m_normalTextureArraySRV);
    if (FAILED(hr))
    {
        throw std::runtime_error("Error loading texture");
    }

    m_deviceContext->CSSetShaderResources(6, 1, &m_diffuseTextureArraySRV);
    m_deviceContext->CSSetShaderResources(9, 1, &m_normalTextureArraySRV);

    return hr;
}

HRESULT GraphicsEngine::InitializeSamplers()
{
    HRESULT hr;
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_ANISOTROPIC; // D3D11_FILTER_ANISOTROPIC  D3D11_FILTER_MIN_MAG_MIP_LINEAR
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX; //D3D11_FLOAT32_MAX
    sampDesc.MaxAnisotropy = 16; //why not max it out when we can?
    hr = m_device->CreateSamplerState(&sampDesc, &m_simpleSampler);
    if (FAILED(hr))
        return hr;

    m_deviceContext->CSSetSamplers(0, 1, &m_simpleSampler);

    return hr;
}

void GraphicsEngine::LoadObject(const std::string & p_name)
{
    std::vector<Vertex> t_vertices;
    std::vector<ObjMaterial> t_material;

    m_objLoader->Load(p_name, t_vertices, t_material);


    /*
    float z = -5;
    
    // Add big square
    Vertex newVert;
    newVert.position = XMFLOAT3(-10, 10, z); // Top left
    newVert.normal = XMFLOAT3(0, 0, 1);
    newVert.texcoord = XMFLOAT2(0, 0);
    t_vertices.push_back(newVert);

    newVert.position = XMFLOAT3(10, 10, z); // Top right
    newVert.normal = XMFLOAT3(0, 0, 1);
    newVert.texcoord = XMFLOAT2(1, 0);
    t_vertices.push_back(newVert);

    newVert.position = XMFLOAT3(-10, -10, z); // Bot left
    newVert.normal = XMFLOAT3(0, 0, 1);
    newVert.texcoord = XMFLOAT2(0, 1);
    t_vertices.push_back(newVert);

    newVert.position = XMFLOAT3(10, 10, z); // Top right
    newVert.normal = XMFLOAT3(0, 0, 1);
    newVert.texcoord = XMFLOAT2(1, 0);
    t_vertices.push_back(newVert);

    newVert.position = XMFLOAT3(10, -10, z); // Bot right
    newVert.normal = XMFLOAT3(0, 0, 1);
    newVert.texcoord = XMFLOAT2(1, 1);
    t_vertices.push_back(newVert);


    newVert.position = XMFLOAT3(-10, -10, z);
    newVert.normal = XMFLOAT3(0, 0, 1);
    newVert.texcoord = XMFLOAT2(0, 1);
    t_vertices.push_back(newVert);
    */

    // http://www.rastertek.com/dx11tut20.html
    // + 3D Game Programming with DirectX 11

    for (size_t i = 0; i < t_vertices.size(); i += 3)
    {

        XMFLOAT3 Aver = t_vertices[i].position;
        XMFLOAT3 Bver = t_vertices[i + 1].position;
        XMFLOAT3 Cver = t_vertices[i + 2].position;

        XMFLOAT2 Atex = t_vertices[i].texcoord;
        XMFLOAT2 Btex = t_vertices[i + 1].texcoord;
        XMFLOAT2 Ctex = t_vertices[i + 2].texcoord;

        // Create A->B, A->C vectors
        XMFLOAT3 AtoBver, AtoCver;
        XMStoreFloat3(&AtoBver, XMLoadFloat3(&Bver) - XMLoadFloat3(&Aver));
        XMStoreFloat3(&AtoCver, XMLoadFloat3(&Cver) - XMLoadFloat3(&Aver));

        XMFLOAT2 texU, texV;
        texU = XMFLOAT2(Btex.x - Atex.x, Ctex.x - Atex.x);
        texV = XMFLOAT2(Btex.y - Atex.y, Ctex.y - Atex.y);

        // Denominator
        float den = 1.0f / (texU.x * texV.y - texU.y * texV.x);

        XMFLOAT3 tangent;
        tangent = XMFLOAT3(AtoBver.x * texV.y - AtoCver.x * texV.x, AtoBver.y * texV.y - AtoCver.y * texV.x, AtoBver.z * texV.y - AtoCver.z * texV.x);
        tangent.x *= den;
        tangent.y *= den;
        tangent.z *= den;


        float length = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));

        tangent.x /= length;
        tangent.y /= length;
        tangent.z /= length;

        t_vertices[i].tangent = tangent;
        t_vertices[i + 1].tangent = tangent;
        t_vertices[i + 2].tangent = tangent;
    }
    

    // Get path
    std::string path = "";
    size_t lastSlash = p_name.find_last_of("/");
    if (lastSlash != std::string::npos)
    {
        path = p_name.substr(0, lastSlash + 1);
    }


    std::vector<ShaderMaterial> t_shaderMaterials;
    // Load each material
    for (size_t i = 0; i < t_material.size(); i++)
    {
        // Load the textures to the 2d arrays
        ID3D11Resource* testRes = nullptr;
        ID3D11ShaderResourceView* texture;
        ID3D11ShaderResourceView* normTexture;

        std::string fullPath = path + t_material[i].diffuseTexture;
        std::wstring t_fullPath;
        t_fullPath.assign(fullPath.begin(), fullPath.end());

        HRESULT hr = CreateDDSTextureFromFile(m_device, t_fullPath.c_str(), &testRes, &texture);
        if (FAILED(hr))
        {
            throw std::runtime_error("Error loading texture");
        }

        m_deviceContext->CopySubresourceRegion(m_diffuseTextureArray, m_curDiffuseIndex, 0, 0, 0, testRes, 0, nullptr);
        m_curDiffuseIndex++;
        
        int normalTextureID = -1;
        if (t_material[i].normalTexture != "")
        {

            fullPath = path + t_material[i].normalTexture;
            t_fullPath.assign(fullPath.begin(), fullPath.end());
            ID3D11ShaderResourceView* t_normTexture;
            hr = CreateDDSTextureFromFile(m_device, t_fullPath.c_str(), &testRes, &normTexture);
            if (FAILED(hr))
            {
                throw std::runtime_error("Error loading texture");
            }
            m_deviceContext->CopySubresourceRegion(m_normalTextureArray, m_curNormalIndex, 0, 0, 0, testRes, 0, nullptr);
            normalTextureID = m_curNormalIndex;
            m_curNormalIndex++;
        }


        // Create a material
        ShaderMaterial t_mat;
        t_mat.Ambient = t_material[i].Ambient;
        t_mat.Diffuse = t_material[i].Diffuse;
        t_mat.diffuseTexture = m_curDiffuseIndex - 1;
        t_mat.normalTexture = normalTextureID;
        t_mat.Specular = t_material[i].Specular;
        t_mat.specularFactor = t_material[i].specularFactor;
        t_mat.transparency = t_material[i].transparency;
        t_shaderMaterials.push_back(t_mat);
    }



    ComputeBuffer* matBuffer = m_computeWrapper->CreateBuffer(COMPUTE_BUFFER_TYPE::STRUCTURED_BUFFER, sizeof(ShaderMaterial), t_shaderMaterials.size(), true, false, &t_shaderMaterials[0]);
    ComputeBuffer* meshBuffer = m_computeWrapper->CreateBuffer(COMPUTE_BUFFER_TYPE::STRUCTURED_BUFFER, sizeof(Vertex), t_vertices.size(), true, false, &t_vertices[0]);
    int preNumVertices = m_numVertices;
    m_numVertices += t_vertices.size();

    // Load to graphics
    // TODO change this,
    ID3D11ShaderResourceView* meshView = meshBuffer->GetResourceView();
    ID3D11ShaderResourceView* matView = matBuffer->GetResourceView();
    m_deviceContext->CSSetShaderResources(2, 1, &meshView);
    m_deviceContext->CSSetShaderResources(3, 1, &matView);

    m_loadedObjects[p_name] = VertexObject(preNumVertices, m_numVertices);
}

void GraphicsEngine::CreateSphere(XMFLOAT3 p_position, float p_radius, XMFLOAT3 p_color)
{
    // Clear previous buffer if needed
    Sphere newSphere;
    newSphere.position = p_position;
    newSphere.radius = p_radius;
    newSphere.color = p_color;

    m_spheres.push_back(newSphere);

    if(m_sphereBuffer != nullptr)
        m_sphereBuffer->Release();

    m_sphereBuffer = m_computeWrapper->CreateBuffer(COMPUTE_BUFFER_TYPE::STRUCTURED_BUFFER, sizeof(Sphere), m_spheres.size(), true, false, &m_spheres[0]);
    ID3D11ShaderResourceView* resourceView = m_sphereBuffer->GetResourceView();
    m_deviceContext->CSSetShaderResources(4, 1, &resourceView);
}



uint32_t GraphicsEngine::AddToRender(const DirectX::XMFLOAT4X4 &p_world, const std::string &p_objName)
{

    auto find = m_loadedObjects.find(p_objName);
    if (find == m_loadedObjects.end())
        return -1;


    ObjectInstance newInstance;
    newInstance.startVertex = find->second.startIndex;
    newInstance.stopVertex = find->second.stopIndex;
    newInstance.world = p_world;
    m_instances.push_back(newInstance);

    if (m_instanceBuffer != nullptr)
        m_instanceBuffer->Release();


    m_instanceBuffer = m_computeWrapper->CreateBuffer(COMPUTE_BUFFER_TYPE::STRUCTURED_BUFFER, sizeof(ObjectInstance), m_instances.size(), true, false, &m_instances[0], false, true);

    ID3D11ShaderResourceView* t_instanceResource = m_instanceBuffer->GetResourceView();
    m_deviceContext->CSSetShaderResources(10, 1, &t_instanceResource);

    return m_instances.size() - 1;
}

UINT GraphicsEngine::CreatePointLight(XMFLOAT3 p_position, float p_radius, XMFLOAT3 p_color)
{
    // Clear previous buffer if needed
    PointLight newLight;
    newLight.position = p_position;
    newLight.radius = p_radius;
    newLight.color = p_color;

    m_pointLights.push_back(newLight);

    if (m_pointLightBuffer != nullptr)
        m_pointLightBuffer->Release();

    m_pointLightBuffer = m_computeWrapper->CreateBuffer(COMPUTE_BUFFER_TYPE::STRUCTURED_BUFFER, sizeof(PointLight), m_pointLights.size(), true, false, &m_pointLights[0], false, true);
    ID3D11ShaderResourceView* resourceView = m_pointLightBuffer->GetResourceView();
    m_deviceContext->CSSetShaderResources(5, 1, &resourceView);

    return m_pointLights.size() - 1;
}

void GraphicsEngine::RemovePointLight()
{
    m_pointLights.erase(m_pointLights.end() - 1);

    if (m_pointLightBuffer != nullptr)
        m_pointLightBuffer->Release();

    // Can't create 0 size buffer
    if (m_pointLights.size() == 0)
        return;

    m_pointLightBuffer = m_computeWrapper->CreateBuffer(COMPUTE_BUFFER_TYPE::STRUCTURED_BUFFER, sizeof(PointLight), m_pointLights.size(), true, false, &m_pointLights[0], false, true);
    ID3D11ShaderResourceView* resourceView = m_pointLightBuffer->GetResourceView();
    m_deviceContext->CSSetShaderResources(5, 1, &resourceView);
}

void GraphicsEngine::UpdatePointLight(XMFLOAT3 p_position, float p_radius, XMFLOAT3 p_color, UINT p_ID)
{
    m_pointLights[p_ID].position = p_position;
    m_pointLights[p_ID].radius = p_radius;
    m_pointLights[p_ID].color = p_color;

    D3D11_MAPPED_SUBRESOURCE MappedResource;
    if (SUCCEEDED(m_deviceContext->Map(m_pointLightBuffer->GetResource(), 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource)))
    {
        for (size_t i = 0; i < m_pointLights.size(); i++)
        {
            ((PointLight*)MappedResource.pData)[i] = m_pointLights[i];
        }
    }

    m_deviceContext->Unmap(m_pointLightBuffer->GetResource(), 0);
}

void GraphicsEngine::CreateSpotLight(XMFLOAT3 p_position, XMFLOAT3 p_direction, float p_radius, XMFLOAT3 p_color, float p_spot)
{
    // Clear previous buffer if needed
    SpotLight newLight;
    newLight.Position = p_position;
    newLight.Direction = p_direction;
    newLight.Radius = p_radius;
    newLight.Color = p_color;
    newLight.Spot = p_spot;

    m_spotLights.push_back(newLight);

    if (m_spotLightBuffer != nullptr)
        m_spotLightBuffer->Release();

    m_spotLightBuffer = m_computeWrapper->CreateBuffer(COMPUTE_BUFFER_TYPE::STRUCTURED_BUFFER, sizeof(SpotLight), m_spotLights.size(), true, false, &m_spotLights[0], false, true);
    ID3D11ShaderResourceView* resourceView = m_spotLightBuffer->GetResourceView();
    m_deviceContext->CSSetShaderResources(8, 1, &resourceView);
}


void GraphicsEngine::RemoveSpotLight()
{
    m_spotLights.erase(m_spotLights.end() - 1);
}

void GraphicsEngine::UpdateWorldPosition(const DirectX::XMFLOAT4X4 &p_world)
{

}

void GraphicsEngine::UpdatePerFrameBuffer()
{
    CameraManager* camMan = CameraManager::GetInstance();
    camMan->Update();
    PerFramebuffer perFrame;
    perFrame.CameraPosition = camMan->GetPosition();
    perFrame.InvView = XMMatrixTranspose(camMan->GetInvView());
    perFrame.NumOfPointLights = m_pointLights.size();
    perFrame.NumOfSpheres = m_spheres.size();
    perFrame.NumOfVertices = m_numVertices;
    perFrame.NumOfInstances = m_instances.size();
    perFrame.NumOfSpotLights = m_spotLights.size();


    D3D11_MAPPED_SUBRESOURCE MappedResource;
    if (SUCCEEDED(m_deviceContext->Map(m_perFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource)))
        *(PerFramebuffer*)MappedResource.pData = perFrame;

    m_deviceContext->Unmap(m_perFrameBuffer, 0);
}


void GraphicsEngine::Render()
{
    UpdatePerFrameBuffer();


    UINT x = ceil((float)WINDOW_DRAW_SIZE_X / (float)THREAD_GROUP_SIZE);
    UINT y = ceil((float)WINDOW_DRAW_SIZE_Y / (float)THREAD_GROUP_SIZE);
    UINT sx = ceil((float)WINDOW_SIZE_X / (float)THREAD_GROUP_SIZE);
    UINT sy = ceil((float)WINDOW_SIZE_Y / (float)THREAD_GROUP_SIZE);

    ID3D11UnorderedAccessView* t_rays = m_rayBuffer->GetUnorderedAccessView();
    ID3D11UnorderedAccessView* t_colordata = m_colorDataBuffer->GetUnorderedAccessView();
    
    m_deviceContext->CSSetUnorderedAccessViews(0, 1, &t_rays, nullptr);
    m_deviceContext->CSSetUnorderedAccessViews(1, 1, &t_colordata, nullptr);

    // Init rays
    m_initRaysShader->Set();
    m_deviceContext->Dispatch(x, y, 1);

    // Intersect
    m_intersectionShader->Set();
    m_deviceContext->Dispatch(x, y, 1);

    // Color
    m_coloringShader->Set();
    m_deviceContext->Dispatch(x, y, 1);

    // For number of bounces
    for (size_t i = 0; i < m_numBounces; i++)
    {
        // Create rays
        m_createRaysShader->Set();
        m_deviceContext->Dispatch(x, y, 1);

        // Intersect
        m_intersectionShader->Set();
        m_deviceContext->Dispatch(x, y, 1);

        // Color
        m_coloringShader->Set();
        m_deviceContext->Dispatch(x, y, 1);

    }
    
    // Supersample
    m_deviceContext->CSSetUnorderedAccessViews(7, 1, &m_backBufferUAV, nullptr);

    m_ssShader->Set();
    m_deviceContext->Dispatch(sx, sy, 1);

    ID3D11ShaderResourceView* t_nullSRVView = nullptr;
    m_deviceContext->CSSetShaderResources(0, 1, &t_nullSRVView);
    
    Timer t_timer;

    if (printFrame)
    {
        t_timer.StartTimer();
    }

    m_swapChain->Present(0, 0);

    if (printFrame)
    {
        t_timer.StopTimer();
        double time = t_timer.GetTime();

        WriteFrameDataToFile(time);

        OutputDebugString(L"PRINTED FRAME \n");
        printFrame = false;
    }
}

void GraphicsEngine::WriteFrameDataToFile(double p_time)
{
    //Number of threads per thread group 
    int numThreads = THREAD_GROUP_SIZE;

    //Screen resolution 
    int screenResX = WINDOW_SIZE_X;
    int screenResY = WINDOW_SIZE_Y;

    //Trace depth 
    int depth = m_numBounces;

    //Number of light sources 
    int lights = m_pointLights.size() + m_spotLights.size();

    //Number of triangles 
    int numVertices = m_numVertices * 2;

    
    std::string fileName = "../../Data/Thread_" +
        std::to_string(numThreads) +
        "_Res_" + std::to_string(screenResX) +
        "_Bounce_" + std::to_string(depth) + 
        "_Light_" + std::to_string(lights) + 
        "_Vert_" + std::to_string(numVertices) + ".txt";


    std::ofstream myfile;
    myfile.open(fileName.c_str(), std::ios::app);
    if (myfile.is_open())
    {
        myfile << p_time << std::endl;

        myfile.close();
    }


}

void GraphicsEngine::IncrementBounces()
{
    m_numBounces++;
}

void GraphicsEngine::DecrementBounces()
{
    if(m_numBounces > 0)
        m_numBounces--;
}

void GraphicsEngine::PrintDataForNextFrame()
{
    printFrame = true;
}