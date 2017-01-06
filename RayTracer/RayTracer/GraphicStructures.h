#pragma once
#include <DirectXMath.h>
#include <string>
using namespace DirectX;


struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT3 normal;
    XMFLOAT3 tangent;
    XMFLOAT2 texcoord;
    int materialID;
};

struct Ray
{
    int dead;
    XMFLOAT3 Position;
    XMFLOAT3 Direction;
    XMFLOAT3 Color;
    int lastVertexIndex;
    int lastSphereIndex;
    int lastInstanceIndex;
    float reflectionFactor;
};

struct ColorData
{
    int dead;
    int indexTriangle;
    int indexSphere;
    int indexInstance;
    XMFLOAT3 startPosition;
    XMFLOAT3 direction;
    XMFLOAT3 hitPosition;
    XMFLOAT3 color;
    float reflection;
    float u;
    float v;
};

struct ObjectInstance
{
    XMFLOAT4X4 world;
    int startVertex;
    int stopVertex;
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

struct SpotLight
{
    XMFLOAT3 Position;
    XMFLOAT3 Direction;
    float Radius;
    XMFLOAT3 Color;
    float Spot;
};

struct ObjMaterial
{
    int id;
    std::string name;
    XMFLOAT3 Ambient;
    XMFLOAT3 Diffuse;
    XMFLOAT3 Specular;
    float specularFactor;
    float transparency; // d
    std::string diffuseTexture;
    std::string normalTexture;
};

struct ShaderMaterial
{
    XMFLOAT3 Ambient;
    XMFLOAT3 Diffuse;
    XMFLOAT3 Specular;
    float specularFactor;
    float transparency; // d
    int diffuseTexture;
    int normalTexture;
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

    unsigned int NumOfInstances;
    unsigned int NumOfVertices;
    unsigned int NumOfSpheres;

    unsigned int NumOfSpotLights;
};


/*
Ns 96.078431
Ka 0.000000 0.000000 0.000000
Kd 0.640000 0.640000 0.640000
Ks 0.500000 0.500000 0.500000
map_Kd sword.dds
*/