#pragma once
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "ComputeHelp.h"
#include "GraphicStructures.h"
#include <string>
#include <d3d11_2.h>
#include <stdint.h>
#include <Windows.h>
#include <DirectXMath.h>
#include <map>
#include <vector>


class ComputeWrap;
class ComputeShader;
class ObjLoader;

class GraphicsEngine
{
public:
    static GraphicsEngine* GetInstance();
    static void Startup(HINSTANCE p_hInstance, int p_nCmdShow, WNDPROC p_winProc);
    void LoadObject(const std::string & p_name);
    void CreateSphere(XMFLOAT3 p_position, float p_radius, XMFLOAT3 p_color);
    uint32_t AddToRender(const DirectX::XMFLOAT4X4 & p_world, const std::string &p_objectName);
    UINT CreatePointLight(XMFLOAT3 p_position, float p_radius, XMFLOAT3 p_color);
    void RemovePointLight();
    void UpdatePointLight(XMFLOAT3 p_position, float p_radius, XMFLOAT3 p_color, UINT p_ID);
    void CreateSpotLight(XMFLOAT3 p_position, XMFLOAT3 p_direction, float p_radius, XMFLOAT3 p_color, float p_spot);
    void RemoveSpotLight();
    void UpdateWorldPosition(const DirectX::XMFLOAT4X4 & p_world);
    void Render();





    void IncrementBounces();

    void DecrementBounces();

    void PrintDataForNextFrame();



private:
    static GraphicsEngine* m_singleton;
    GraphicsEngine(HINSTANCE p_hInstance, int p_nCmdShow, WNDPROC p_winProc);
    ~GraphicsEngine();
    HRESULT InitializeWindow(int p_nCmdShow, WNDPROC p_winProc);
    HRESULT InitializeDirectX();
    HRESULT InitializeBackBuffer();
    HRESULT InitializeShaders();
    HRESULT InitializeBuffers();
    HRESULT InitializeTextureArrays();

    HRESULT InitializeSamplers();

    void UpdatePerFrameBuffer();
    void WriteFrameDataToFile(double p_time);

    // DirectX Windows parameters
    HINSTANCE m_handleInstance = NULL;
    HWND m_handleWindow = NULL;

    // Window properties
    uint16_t m_width;
    uint16_t m_height;

    // Directx API and backbuffer
    IDXGISwapChain* m_swapChain = NULL;
    ID3D11Device* m_device = NULL;
    ID3D11DeviceContext* m_deviceContext = NULL;
    ID3D11UnorderedAccessView*  m_backBufferUAV = NULL;  // compute output

    // Help classes
    ComputeWrap* m_computeWrapper = NULL;
    ObjLoader* m_objLoader = NULL;

    // Shaders
    ComputeShader* m_initRaysShader = NULL;
    ComputeShader* m_createRaysShader = NULL;
    ComputeShader* m_intersectionShader = NULL;
    ComputeShader* m_coloringShader = NULL;
    ComputeShader* m_lightCalcShader = NULL;
    ComputeShader* m_ssShader = NULL;

    // Buffers
    ComputeBuffer* m_rayBuffer;
    ComputeBuffer* m_colorDataBuffer;
    ComputeBuffer* m_instanceBuffer;
    ID3D11Buffer* m_constantBuffer;
    ID3D11Buffer* m_perFrameBuffer;

    // Texturebuffers
    ID3D11Texture2D* m_diffuseTextureArray;
    ID3D11ShaderResourceView* m_diffuseTextureArraySRV;
    ID3D11Texture2D* m_normalTextureArray;
    ID3D11ShaderResourceView* m_normalTextureArraySRV;
    UINT m_curDiffuseIndex;
    UINT m_curNormalIndex;

    // Samplers
    ID3D11SamplerState* m_simpleSampler;

    // Spheres
    ComputeBuffer* m_sphereBuffer;
    std::vector<Sphere> m_spheres;

    // Lights
    ComputeBuffer* m_pointLightBuffer;
    std::vector<PointLight> m_pointLights;

    ComputeBuffer* m_spotLightBuffer;
    std::vector<SpotLight> m_spotLights;

    struct RenderObject
    {
        UINT numOfVertices;
        ComputeBuffer* meshBuffer;
        ComputeBuffer* materialBuffer;
        ID3D11ShaderResourceView* texture;
    };

    struct ObjectInstance
    {
        XMFLOAT4X4 world;
        int startVertex;
        int stopVertex;
    };

    struct VertexObject
    {
        int startIndex;
        int stopIndex;

        VertexObject(int start, int stop)
        {
            startIndex = start;
            stopIndex = stop;
        }
        VertexObject()
        {

        }
    };

    // Resources
    std::map<std::string, RenderObject> m_activeObjects;
    std::map<std::string, VertexObject> m_loadedObjects;
    std::vector<ObjectInstance> m_instances;

    // Should be removed later to be put in struct
    UINT m_numVertices;

    UINT m_numBounces;
    bool printFrame;
};

