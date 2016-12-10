#pragma once
#include <DirectXMath.h>
#include <string>
using namespace DirectX;


struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT3 normal;
    XMFLOAT2 texcoord;
};

struct Ray
{
    XMFLOAT3 Position;
    XMFLOAT3 Direction;
    XMFLOAT3 Color;
    int lastVertexIndex;
    float reflectionFactor;
};

struct ColorData
{
    int indexTriangle;
    int indexSphere;
    XMFLOAT3 startPosition;
    XMFLOAT3 direction;
    XMFLOAT3 hitPosition;
    float u;
    float v;
    float t;
};

struct Sphere
{
    XMFLOAT3 position;
    float radius;
    XMFLOAT3 color;
};

struct PointLight
{
    XMFLOAT3 position;
    float radius;
    XMFLOAT3 color;
};

struct ShaderMaterial
{
    XMFLOAT4 Ambient;
    XMFLOAT4 Diffuse;
    XMFLOAT4 Specular;
};

struct Material
{
    XMFLOAT3 Ambient;
    XMFLOAT3 Diffuse;
    XMFLOAT3 Specular;
    std::string diffuseTexture;
};


struct ConstantBuffer
{
    XMMATRIX Proj;
    XMUINT2 ScreenDimensions; //width height
    XMFLOAT2 DoubleScreenByDimension;
};

struct PerFramebuffer
{
    XMMATRIX InvView;

    XMFLOAT3 CameraPosition;
    unsigned int NumOfPointLights;

    unsigned int NumOfVertices;
    unsigned int NumOfSpheres;

    XMFLOAT2 filler;
};


/*
Ns 96.078431
Ka 0.000000 0.000000 0.000000
Kd 0.640000 0.640000 0.640000
Ks 0.500000 0.500000 0.500000
map_Kd sword.dds
*/