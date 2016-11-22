#include "Common.fx"

[numthreads(32, 32, 1)]
void CS(uint3 threadID : SV_DispatchThreadID)
{

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

struct Sphere
{
    float3 Position;
    float Radius;
    float3 Color;
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

RWStructuredBuffer<ColorData> output : register(u0);

StructuredBuffer<Ray> Rays : register(t0);
StructuredBuffer<Vertex> Vertices : register(t1);
StructuredBuffer<Sphere> Spheres : register(t2);


//http://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/barycentric-coordinates
//kinda same https://courses.cs.washington.edu/courses/cse457/07sp/lectures/triangle_intersection.pdf

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
    //if (det < kEpsilon)
    //{
    //	return false;
    //}

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

bool CheckSphereCollision(Ray mRay, uint index, out float t)
{
    Sphere mSphere = Spheres[index];
    float3 sphereToRay = (mRay.Position - mSphere.Position);
    float lenStoR = length(sphereToRay);
    float lenDotDist = dot(mRay.Direction, sphereToRay);

    float underRoot = dot(lenDotDist, lenDotDist) - lenStoR*lenStoR + mSphere.Radius*mSphere.Radius;

    if (underRoot < 0)
    {
        return false;
    }

    float t1 = -(dot(mRay.Direction, sphereToRay)) - underRoot;
    float t2 = -(dot(mRay.Direction, sphereToRay)) + underRoot;

    t = min(t1, t2);

    return true;
}


[numthreads(32, 32, 1)]
void CS(uint3 threadID : SV_DispatchThreadID)
{
    //warning might not work if we're out of scope with a thread, since it will write to same slot as next row
    unsigned int index = threadID.y * ScreenDimensions.x + threadID.x;

    Ray myRay = Rays[index];
    ColorData tColData;
    tColData.index = -1;
    tColData.w = 0;
    tColData.u = 0;
    tColData.v = 0;
    tColData.hitPos = float3(0, 0, 0);
    tColData.LastColor = myRay.Color;
    tColData.reflectionFactor = myRay.reflectionFactor;
    tColData.filler2 = 0;
    tColData.filler3 = 0;

    float maxT = 10000000000000.0f;
    //check closest triangle 
    for (uint i = 0; i < NumOfVertices; i += 3)
    {
        if (myRay.lastVertexIndex != i)
        {
            float t, u, v;
            if (CheckTriangleCollision(myRay, i, t, u, v))
            {
                if (t < maxT)
                {
                    maxT = t;
                    tColData.w = (1 - u - v);
                    tColData.u = u;
                    tColData.v = v;
                    tColData.index = i;
                    //finalColor = w * Vertices[i].Color + u * Vertices[i + 1].Color + v * Vertices[i + 2].Color;
                }
            }
        }
    }

    ////check closest sphere 
    //for (uint k = 0; k < NumOfSpheres; k++)
    //{
    //	float t, u, v;
    //	if (CheckSphereCollision(myRay, k, t))
    //	{
    //		if (t < bestT)
    //		{
    //			bestT = t;
    //			finalColor = Spheres[k].Color;
    //		}
    //	}
    //}
    if (tColData.index != -1)
    {
        tColData.hitPos = myRay.Position + myRay.Direction*maxT;
        tColData.direction = myRay.Direction;

    }

    output[index] = tColData;
}

*/