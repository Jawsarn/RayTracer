#include "GraphicsEngine.h"
#include "ObjLoader.h"
#include <stdexcept>
#include "DDSTextureLoader.h"
#include "CameraManager.h"

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
    RECT rc = { 0, 0, 800, 800 };
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
    m_intersectionShader = m_computeWrapper->CreateComputeShader(_T("Intersection.hlsl"), NULL, "CS", NULL);
    m_coloringShader = m_computeWrapper->CreateComputeShader(_T("Coloring.hlsl"), NULL, "CS", NULL);

    return S_OK;
}

HRESULT GraphicsEngine::InitializeBuffers()
{
    m_rayBuffer = m_computeWrapper->CreateBuffer(COMPUTE_BUFFER_TYPE::STRUCTURED_BUFFER, sizeof(Ray), m_width*m_height, true, true, nullptr);

    ConstantBuffer constBuf;
    constBuf.Proj = CameraManager::GetInstance()->GetProj();
    constBuf.ScreenDimensions = XMUINT2(m_width, m_height); 
    constBuf.DoubleScreenByDimension = XMFLOAT2((2.0f /(float)m_width), -(2.0f / (float)m_height));
    

    m_constantBuffer = m_computeWrapper->CreateConstantBuffer(sizeof(ConstantBuffer), &constBuf, D3D11_USAGE::D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE);

    m_perFrameBuffer = m_computeWrapper->CreateConstantBuffer(sizeof(PerFramebuffer), nullptr, D3D11_USAGE::D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE);

    // Set the buffers
    m_deviceContext->CSSetConstantBuffers(0, 1, &m_constantBuffer);
    m_deviceContext->CSSetConstantBuffers(1, 1, &m_perFrameBuffer);

    return S_OK;
}

void GraphicsEngine::LoadObject(const std::string & p_name)
{
    std::vector<Vertex> t_vertices;
    Material t_material;

    m_objLoader->Load(p_name, t_vertices, t_material);

    ShaderMaterial t_mat;
    t_mat.Ambient = DirectX::XMFLOAT4(t_material.Ambient.x, t_material.Ambient.y, t_material.Ambient.z, 0);
    t_mat.Diffuse = DirectX::XMFLOAT4(t_material.Diffuse.x, t_material.Diffuse.y, t_material.Diffuse.z, 0);
    t_mat.Specular = DirectX::XMFLOAT4(t_material.Specular.x, t_material.Specular.y, t_material.Specular.z, 0);

    RenderObject newRenderObj;
    newRenderObj.meshBuffer = m_computeWrapper->CreateBuffer(COMPUTE_BUFFER_TYPE::STRUCTURED_BUFFER, sizeof(Vertex), t_vertices.size(), true, false, &t_vertices[0]);
    newRenderObj.materialBuffer = m_computeWrapper->CreateBuffer(COMPUTE_BUFFER_TYPE::STRUCTURED_BUFFER, sizeof(ShaderMaterial), 1, true, false, &t_mat);


    std::string path = "";

    size_t lastSlash = p_name.find_last_of("/");
    if (lastSlash != std::string::npos)
    {
        path = p_name.substr(0, lastSlash + 1);
    }
    path = path + t_material.diffuseTexture;
    std::wstring t_fullPath;
    t_fullPath.assign(path.begin(), path.end());

    HRESULT hr = CreateDDSTextureFromFile(m_device, t_fullPath.c_str(), nullptr, &newRenderObj.texture);
    if (FAILED(hr))
    {
        throw std::runtime_error("Error loading texture");
    }
}

uint32_t GraphicsEngine::AddToRender(const DirectX::XMFLOAT4X4 &p_world, const std::string &p_objName)
{
    ObjectInstance newInstance;
    newInstance.renderObj = p_objName;
    newInstance.world = p_world;
    m_instances.push_back(newInstance);

    return m_instances.size() - 1;
}

void GraphicsEngine::UpdateWorldPosition(const DirectX::XMFLOAT4X4 &p_world)
{

}

void GraphicsEngine::UpdatePerFrameBuffer()
{
    CameraManager* camMan = CameraManager::GetInstance();
    PerFramebuffer perFrame;
    perFrame.CameraPosition = camMan->GetPosition();
    perFrame.InvView = camMan->GetInvView();
    perFrame.NumOfPointLights = 0;
    perFrame.NumOfSpheres = 0;
    perFrame.NumOfVertices = 0;


    D3D11_MAPPED_SUBRESOURCE MappedResource;
    if (SUCCEEDED(m_deviceContext->Map(m_perFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource)))
    *(PerFramebuffer*)MappedResource.pData = perFrame;

    m_deviceContext->Unmap(m_perFrameBuffer, 0);
}

void GraphicsEngine::Render()
{
    UpdatePerFrameBuffer();

    UINT x = ceil((float)m_width / (float)THREAD_GROUP_SIZE_X);
    UINT y = ceil((float)m_height / (float)THREAD_GROUP_SIZE_Y);

    ID3D11UnorderedAccessView* t_rays = m_rayBuffer->GetUnorderedAccessView();
    
    m_deviceContext->CSSetUnorderedAccessViews(0, 1, &m_backBufferUAV, nullptr);
    m_deviceContext->CSSetUnorderedAccessViews(1, 1, &t_rays, nullptr);

    // Init rays
    m_initRaysShader->Set();
    m_deviceContext->Dispatch(x, y, 1);

    // Color
    m_coloringShader->Set();
    m_deviceContext->Dispatch(x, y, 1);

    m_swapChain->Present(0, 0);
}