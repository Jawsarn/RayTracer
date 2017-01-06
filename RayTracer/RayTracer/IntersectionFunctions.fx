#include "Common.fx"

//https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection
// Could try, might be faster http://www.lighthouse3d.com/tutorials/maths/ray-sphere-intersection/
bool CheckSphereCollision(Ray mRay, uint index, out float t)
{
    Sphere sphere = spheres[index];

    float3 sphereToRay = (mRay.Position - sphere.Position);
    float lenStoR = length(sphereToRay);

    float lenDotDist = dot(mRay.Direction, sphereToRay);

    if (lenDotDist > 0) // possible speedup
        return false;

    float underRoot = lenDotDist*lenDotDist - lenStoR*lenStoR + sphere.Radius*sphere.Radius;

    if (underRoot < 0)
    {
        return false;
    }

    t = -lenDotDist - sqrt(underRoot);

    return true;
}

// See if there are better triangle collision ways
// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
//http://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/barycentric-coordinates
//kinda same https://courses.cs.washington.edu/courses/cse457/07sp/lectures/triangle_intersection.pdf
bool CheckTriangleCollision(Ray pRay, uint startIndex, matrix world, out float t, out float u, out float v)
{
    Vertex A = vertices[startIndex];
    Vertex B = vertices[startIndex + 1];
    Vertex C = vertices[startIndex + 2];
    A.Position = mul(float4(A.Position, 1), world).xyz;
    B.Position = mul(float4(B.Position, 1), world).xyz;
    C.Position = mul(float4(C.Position, 1), world).xyz;

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
    if (u < 0.0f || u > 1.0f)
    {
        return false;
    }

    float3 qVec = cross(tVec, AtoB);
    v = dot(pRay.Direction, qVec) * invDet;
    if (v < 0.0f || u + v > 1.0f)
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