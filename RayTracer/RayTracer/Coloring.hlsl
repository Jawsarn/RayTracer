#include "Common.fx"

RWTexture2D<float4> output : register(u0);
RWStructuredBuffer<Ray> rays : register(u1);

struct Sphere
{
    float3 Position;
    float Radius;
    float3 Color;
};

bool CheckSphereCollision(Ray mRay)
{
    Sphere sphere;
    sphere.Position = float3(0, 0, -20.0f);
    sphere.Radius = 1;

    float3 sphereToRay = (mRay.Position - sphere.Position);
    float lenStoR = length(sphereToRay);
    float lenDotDist = dot(mRay.Direction, sphereToRay);

    float underRoot = dot(lenDotDist, lenDotDist) - lenStoR*lenStoR + sphere.Radius*sphere.Radius;

    if (underRoot < 0)
    {
        return false;
    }

    return true;
}

[numthreads(32, 32, 1)]
void CS(uint3 threadID : SV_DispatchThreadID)
{
    // Out slot for rays 
    uint index = threadID.y * ScreenDimensions.x + threadID.x;

    Ray newRay = rays[index];
    
    if (CheckSphereCollision(newRay))
    {
        output[threadID.xy] = float4(0,0,0, 0);
    }
    else
    {
        output[threadID.xy] = float4(1,1,1,0);
    }
    
}


/*

struct Ray
{
float3 Position;
float3 Direction;
float3 Color;
int lastVertexIndex;
float reflectionFactor;
};

struct Vertex
{
float3 Position;
float3 Normal;
float2 TexCord;
};

struct ColorData
{
float w;
float u;
float v;
int index;
float3 hitPos;
float reflectionFactor;
float3 direction;
float filler2;
float3 LastColor;
float filler3;
};

struct PointLight
{
float3 Position;
float Range;
float3 Color;
};

cbuffer PerFrameBuffer : register(b0)
{
matrix View;
matrix Proj;
float2 ScreenDimensions; //width height
uint NumOfVertices;
uint NumOfSpheres;
float3 CameraPosition;
uint NumOfPointLights;
};

RWTexture2D<float4> output : register(u0);
RWStructuredBuffer<Ray> Rays : register(u1);
StructuredBuffer<Vertex> Vertices : register(t0);
StructuredBuffer<ColorData> ColorDatas : register(t1);
StructuredBuffer<PointLight> PointLights : register(t2);
Texture2D TextureOne : register(t3);

SamplerState Sampler : register(s0);

static const float kEpsilon = 1e-8;

bool CheckTriangleCollision(Ray pRay, uint startIndex, out float t, out float u, out float v)
{
Vertex A = Vertices[startIndex];
Vertex B = Vertices[startIndex + 1];
Vertex C = Vertices[startIndex + 2];

float3 AtoB = B.Position - A.Position;
float3 AtoC = C.Position - A.Position;

float3 pVec = cross(pRay.Direction, AtoC);
float det = dot(AtoB, pVec);

//if culling comment in
/*if (det < kEpsilon)
{
return false;
}

if (abs(det) < kEpsilon)
{
    return false;
}

float invDet = 1 / det;

float3 tVec = pRay.Position - A.Position;

u = dot(tVec, pVec) * invDet;
if (u < 0 || u > 1)
{
    return false;
}

float3 qVec = cross(tVec, AtoB);
v = dot(pRay.Direction, qVec) * invDet;
if (v < 0 || u + v > 1)
{
    return false;
}

t = dot(AtoC, qVec)*invDet;
if (t < 0)
{
    return false;
}

return true;
}

float3 DirectIllumination(float3 pos, float3 norm, PointLight light, float inSpec)
{
    float3 lightPos = light.Position;

    float3 lightVec = lightPos - pos;

    float d = length(lightVec);
    if (d > light.Range)
    {
        return float3(0, 0, 0);
    }

    //normalize vector
    lightVec /= d;

    //diffuse factor
    float diffuseFactor = dot(lightVec, norm);

    if (diffuseFactor < 0.02f)
    {
        return float3(0, 0, 0);
    }

    float att = pow(max(0.0f, 1.0 - (d / light.Range)), 2.0f);

    float3 toEye = normalize(CameraPosition - pos);
    float3 v = reflect(-lightVec, norm);


    float specFactor = pow(max(dot(v, toEye), 0.0f), 1.0f)*inSpec;

    return (light.Color *att * (diffuseFactor + specFactor));
}


[numthreads(32, 32, 1)]
void CS(uint3 threadID : SV_DispatchThreadID)
{
    unsigned int index = threadID.y * ScreenDimensions.x + threadID.x;

    ColorData tColData = ColorDatas[index];
    Ray nextRay;
    nextRay.Position = float3(0, 0, 0);
    nextRay.Direction = float3(0, 0, 0);
    nextRay.Color = float3(0, 0, 0);
    nextRay.lastVertexIndex = -1;
    nextRay.reflectionFactor = 0.5f;


    float3 finalColor = float3(0, 0, 0);

    if (tColData.index > -1)
    {
        Vertex v0 = Vertices[tColData.index];
        Vertex v1 = Vertices[tColData.index + 1];
        Vertex v2 = Vertices[tColData.index + 2];

        //create normal from triangle

        float3 hitPos = tColData.hitPos;


        float2 texCords = v0.TexCord * tColData.w + v1.TexCord * tColData.u + v2.TexCord * tColData.v;
        float3 normal = normalize(v0.Normal * tColData.w + v1.Normal * tColData.u + v2.Normal * tColData.v);
        float3 matColor = TextureOne.SampleLevel(Sampler, texCords, 0.0f);
        finalColor = matColor * 0.1f;



        //for each light, we look if any vertices block it
        for (uint i = 0; i < NumOfPointLights; i++)
        {
            //we create a ray from our position and the target
            Ray tToLightRay;
            PointLight tLight = PointLights[i];
            tToLightRay.Direction = normalize(tLight.Position - hitPos);
            tToLightRay.Position = hitPos;

            bool hit = false;

            //check if we're going from the face outward
            if (dot(tToLightRay.Direction, normal) > 0)
            {
                for (uint k = 0; k < NumOfVertices && !hit; k += 3)
                {
                    if (k != tColData.index)
                    {
                        float t, u, v;
                        if (CheckTriangleCollision(tToLightRay, k, t, u, v))
                        {
                            hit = true;
                        }
                    }
                }
            }

            if (!hit)
            {
                finalColor += matColor*DirectIllumination(hitPos, normal, tLight, 5.0f);
            }

        }

        finalColor *= tColData.reflectionFactor;
        finalColor += tColData.LastColor;

        //maybe move this to the intersection one, because if we do we might be able to use create the rays while we color them?
        ////new ray here
        nextRay.Position = hitPos;
        nextRay.Direction = reflect(tColData.direction, normal);
        nextRay.Color = finalColor;
        nextRay.lastVertexIndex = tColData.index;
        ////reflect direction from ray by normal
        //float3 newDir = reflect(tRay.Direction, normal);


    }


    Rays[index] = nextRay;
    output[threadID.xy] = float4(finalColor, 0);
}

*/