
float3 DirectIlluminationPointLight(float3 pos, float3 norm, PointLight light, float inSpec)
{
    float3 lightPos = light.Position;

    float3 lightVec = lightPos - pos;

    float d = length(lightVec);
    if (d > light.Radius)
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

    float att = pow(max(0.0f, 1.0 - (d / light.Radius)), 2.0f);

    float3 toEye = normalize(CameraPosition - pos);
    float3 v = reflect(-lightVec, norm);


    float specFactor = pow(max(dot(v, toEye), 0.0f), 1.0f)*inSpec;

    return (light.Color *att * (diffuseFactor + specFactor));
}

float3 DirectIlluminationSpotLight(float3 pos, float3 norm, SpotLight light, float inSpec)
{
    float3 lightPos = light.Position;

    float3 lightVec = lightPos - pos;

    float d = length(lightVec);
    if (d > light.Radius)
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

    float spot = pow(max(dot(-lightVec, light.Direction), 0.0f), light.Spot);

    float att = pow(max(0.0f, 1.0 - (d / light.Radius)), 2.0f) * spot;

    float3 toEye = normalize(CameraPosition - pos);
    float3 v = reflect(-lightVec, norm);


    float specFactor = pow(max(dot(v, toEye), 0.0f), 1.0f)*inSpec;

    return (light.Color *att * (diffuseFactor + specFactor));
}