#include "Common.fx"

bool CheckSphereCollision(Ray mRay, uint index, out float t)
{
    Sphere sphere = spheres[index];

    float3 sphereToRay = (mRay.Position - sphere.Position);
    float lenStoR = length(sphereToRay);
    float lenDotDist = dot(mRay.Direction, sphereToRay);

    float underRoot = dot(lenDotDist, lenDotDist) - lenStoR*lenStoR + sphere.Radius*sphere.Radius;

    if (underRoot < 0)
    {
        return false;
    }

    t = underRoot;
    return true;
}

bool CheckTriangleCollision(Ray pRay, uint startIndex, out float t, out float u, out float v)
{
    Vertex A = vertices[startIndex];
    Vertex B = vertices[startIndex + 1];
    Vertex C = vertices[startIndex + 2];

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