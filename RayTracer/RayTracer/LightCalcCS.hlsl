#include "IntersectionFunctions.fx"
#include "IlluminationFunctions.fx"

bool CheckWorldCollision(Ray pRay, float pLengthToLight)
{
    float t = 0;
    float u = 0;
    float v = 0;


    for (uint i = 0; i < NumOfVertices; i += 3)
    {
        if (CheckTriangleCollision(pRay, i, t, u, v))
        {
            if (t < pLengthToLight && t > kEpsilon*1000.0f) //&& data.indexTriangle != i
            {
                return true;
            }
        }
    }


    for (uint i = 0; i < NumOfSpheres; i++)
    {
        if (CheckSphereCollision(pRay, i, t))
        {
            if (t < pLengthToLight)
            {
                return true;
            }
        }
    }
    
    return false;
}


[numthreads(32, 32, 1)]
void CS( uint3 threadID : SV_DispatchThreadID )
{
    uint index = threadID.y * ScreenDimensions.x + threadID.x;
    if (threadID.x >= ScreenDimensions.x || threadID.y >= ScreenDimensions.y)
        return;

    ColorData data = colorData[index];

    float3 finalColor = data.matColor * 0.1f;
    float3 matColor = data.matColor;
    float3 normal = data.normal;
    float3 hitPosition = data.hitPosition;

    // For each light
    for (uint i = 0; i < NumOfPointLights; i++)
    {
        PointLight light = pointLights[i];
        Ray newRay;
        newRay.Position = data.hitPosition;
        newRay.Direction = normalize(light.Position - data.hitPosition);
        float lengthToLight = length(light.Position - data.hitPosition);

        
        // Add light
        if (!CheckWorldCollision(newRay, lengthToLight))
        {
            finalColor += DirectIlluminationPointLight(hitPosition, normal, light, 0.5f) * matColor;
            //finalColor += float3(9, 9, 9)* matColor;
        }
    }

    // For each spotlight
    for (uint i = 0; i < NumOfSpotLights; i++)
    {
        SpotLight light = spotLights[i];
        Ray newRay;
        newRay.Position = data.hitPosition;
        newRay.Direction = normalize(light.Position - data.hitPosition);
        float lengthToLight = length(light.Position - data.hitPosition);


        // Add light
        if (!CheckWorldCollision(newRay, lengthToLight))
        {
            finalColor += DirectIlluminationSpotLight(hitPosition, normal, spotLights[0], 0.5f) * matColor;
            //finalColor += float3(9, 9, 9)* matColor;
        }
    }


    finalColor = finalColor*data.reflection + data.color*(1.0f - data.reflection);

    data.direction = normalize(reflect(data.direction, normal));
    data.color = finalColor;
    data.reflection = 0.5f;
    colorData[index] = data;

    output[threadID.xy] = float4(finalColor, 0);
}