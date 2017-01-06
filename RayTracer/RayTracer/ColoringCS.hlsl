#include "IntersectionFunctions.fx"
#include "IlluminationFunctions.fx"


/*
Help text
For each object we hit, we send a ray to each light and see if it hits or misses.
If it hits we add light, if not ignore.

TODO; Remember to add instanceID to all checks vs vertexID, as this will be needed as well

*/

bool CheckWorldCollision(Ray pRay, float pLengthToLight, int sphereIndex, int triangleIndex, int instanceIndex)
{
    float t = 0;
    float u = 0;
    float v = 0;


    for (int inst = 0; inst < NumOfInstances; inst++)
    {
        ObjectInstance objInstance = objInstances[inst];
        matrix world = objInstance.world;
        int vertID = objInstance.startVertex;
        int stop = objInstance.stopVertex;

        for (; vertID < stop; vertID += 3)
        {
            if (CheckTriangleCollision(pRay, vertID, world, t, u, v))
            {
                if (t < pLengthToLight && t > kEpsilon*1000.0f && (triangleIndex != vertID || instanceIndex != inst)) //
                {
                    return true;
                }
            }
        }
    }


    for (uint i = 0; i < NumOfSpheres; i++)
    {
        if (CheckSphereCollision(pRay, i, t))
        {
            if (t < pLengthToLight && sphereIndex != i)
            {
                return true;
            }
        }
    }

    return false;
}


// Created based from 3D Game Programming with DirectX 11
float3 CalculateNormalFromNormalMap(float3 normalFromMap, float3 worldNormal, float3 worldTangent)
{
    // Transform from color values
    float3 realNMapNormal = 2.0f*normalFromMap - 1.0f;

    // Get real tangent, because of interpolation
    float3 tangentN = normalize(worldTangent - dot(worldTangent, worldNormal) * worldNormal);

    // Get binormal
    float3 binormal = cross(worldNormal, tangentN);

    // Create matrix, not sure why tangent is first?
    float3x3 textureMatrix = float3x3(tangentN, binormal, worldNormal);

    // Tranform sampled normal
    float3 sampleNormal = mul(realNMapNormal, textureMatrix);
    return sampleNormal;

}

[numthreads(NUM_GROUP_THREADS, NUM_GROUP_THREADS, 1)]
void CS(uint3 threadID : SV_DispatchThreadID)
{
    uint index = threadID.y * ScreenDimensions.x + threadID.x;
    if (threadID.x >= ScreenDimensions.x || threadID.y >= ScreenDimensions.y)
        return;

    ColorData data = colorData[index];

    if (data.indexSphere == -1 && data.indexTriangle == -1 || data.dead)
    {
        data.dead = 1;
        return;
    }



    float3 normal = float3(0, 0, 0);
    float3 matColor = float3(0, 0, 0);
    float3 Ambient = float3(0,0,0);
    float3 Diffuse = float3(0,0,0);
    float3 Specular = float3(0,0,0);
    float specularFactor = 1.0f;

    if (data.indexSphere != -1)
    {
        Sphere sphere = spheres[data.indexSphere];
        matColor = sphere.Color;
        normal = normalize(data.hitPosition - sphere.Position);
    }
    else if (data.indexTriangle != -1)
    {
        matrix world = objInstances[data.indexInstance].world;

        Vertex v0 = vertices[data.indexTriangle];
        Vertex v1 = vertices[data.indexTriangle + 1];
        Vertex v2 = vertices[data.indexTriangle + 2];

        v0.Position = mul(float4(v0.Position, 1), world).xyz;
        v1.Position = mul(float4(v1.Position, 1), world).xyz;
        v2.Position = mul(float4(v2.Position, 1), world).xyz;

        v0.Normal = mul(float4(v0.Normal, 0), world).xyz;
        v1.Normal = mul(float4(v1.Normal, 0), world).xyz;
        v2.Normal = mul(float4(v2.Normal, 0), world).xyz;

        v0.Tangent = mul(float4(v0.Tangent, 0), world).xyz;
        v1.Tangent = mul(float4(v1.Tangent, 0), world).xyz;
        v2.Tangent = mul(float4(v2.Tangent, 0), world).xyz;


        float w = (1 - data.u - data.v);
        normal = normalize(v0.Normal * w + v1.Normal * data.u + v2.Normal * data.v);
        float3 tangent = normalize(v0.Tangent * w + v1.Tangent * data.u + v2.Tangent * data.v);
        float2 uvCord = v0.TexCord * w + v1.TexCord * data.u + v2.TexCord * data.v;


        Material curMat = materials[v0.materialID];
        Ambient = curMat.Ambient;
        Diffuse = curMat.Diffuse;
        Specular = curMat.Specular;
        specularFactor = curMat.specularFactor;


        matColor = meshTexture.SampleLevel(simpleSampler, float3(uvCord, curMat.diffuseTexture), 0);
        //matColor = float3(0.5f, 0.5f, 0.5f);
        // Get normal from normalmap
        if (curMat.normalTexture != -1)
        {
            float3 nMapNormal = normalMap.SampleLevel(simpleSampler, float3(uvCord, curMat.normalTexture), 0);
            normal = CalculateNormalFromNormalMap(nMapNormal, normal, tangent);
            //matColor = normal;
            //normal = CalculateNormalFromNormalMap(nMapNormal, normal, tangent);
            //normal = nMapNormal;
            //matColor = tangent;
            //matColor = nMapNormal;
        }
    }

    float3 finalColor = matColor * 0.01f;

    // For each light
    for (uint i = 0; i < NumOfPointLights; i++)
    {
        PointLight light = pointLights[i];
        Ray newRay;
        newRay.Position = data.hitPosition;
        newRay.Direction = normalize(light.Position - data.hitPosition);
        float lengthToLight = length(light.Position - data.hitPosition);


        // Add light
        if (!CheckWorldCollision(newRay, lengthToLight, data.indexSphere, data.indexTriangle, data.indexInstance))
        {
            finalColor += DirectIlluminationPointLight(data.hitPosition, normal, light, Ambient, Diffuse, Specular, specularFactor) * matColor;
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
        if (!CheckWorldCollision(newRay, lengthToLight, data.indexSphere, data.indexTriangle, data.indexInstance))
        {
            finalColor += DirectIlluminationSpotLight(data.hitPosition, normal, spotLights[0], Ambient, Diffuse, Specular, specularFactor) * matColor;
        }
    }


    finalColor = finalColor*data.reflection + data.color;

    data.direction = normalize(reflect( data.direction, normal));
    data.color = finalColor;
    data.reflection = data.reflection*0.5f;
    colorData[index] = data;

}
