#include "Common.fx"

// Resources
// TODO Check difference from changing color on backbuffer then variable in Rays
RWTexture2D<float4> output : register(u0);
RWStructuredBuffer<Ray> rays : register(u1);





[numthreads(32, 32, 1)]
void CS( uint3 threadID : SV_DispatchThreadID )
{
	//output[threadID.xy] = float4((float)threadID.x / 800, (float)threadID.y / 800, 1,1); // temp

    // Screen cords
    // (1/ Y) * X * 2 - 1 to get screenspace
    // Less computation with;
    // (1/Y)*2) * X - 1

    float2 screenPos = float2(threadID.xy) * DoubleScreenByDimension + float2(-1.0f, 1.0f);

    // Create ray in screenspace
    // As the proj is diagonal matrix, we can use mul=>div to get from screen=>
    float2 screenSpaceDirection = float2(screenPos.x / Proj._11, screenPos.y / Proj._22);

    // Multiplie with inverse view here
    // TODO check if We could maybe multiply the vertices with view instead?
    float3 viewSpacePos = mul(float4(screenPos, 2.0f, 1.0f), InvView).xyz;

    // Get direction in view space
    float3 viewSpaceDirection = mul(float4(screenSpaceDirection, 1.0f, 0.0f), InvView).xyz;

    // Out slot for rays 
    uint outIndex = threadID.y * ScreenDimensions.x + threadID.x;

    // Create ray
    Ray newRay;
    newRay.Position = viewSpacePos;
    newRay.Direction = float3(screenSpaceDirection, 0);
    newRay.Color = float3((float)threadID.x / 800, 0, (float)threadID.y / 800);
    newRay.lastVertexIndex = -1;
    newRay.reflectionFactor = 1.0f;
    rays[outIndex] = newRay;
}

/*

//warning might not work if we're out of scope with a thread, since it will write to same slot as next row
unsigned int outIndex = threadID.y * ScreenDimensions.x + threadID.x;

//get clobal cords
uint2 t_GlobalCord = threadID.xy;

//get the screen cords
float2 t_ScreenPixelOffset = float2(2.0f, -2.0f) / ScreenDimensions;
float2 t_ScreenPos = (float2(t_GlobalCord) + 0.5f) * t_ScreenPixelOffset + float2(-1.0f, 1.0f);

//create ray in screenspace, this should now only have to be multiplied with a view matrix to get to view space?
float2 screenSpaceRay = float2(t_ScreenPos.x / Proj._11, t_ScreenPos.y / Proj._22);

//near plane
float t_Near = Proj._43;

//either multiplie with inverse view here, or multiply the vertices with view?
float3 t_ViewSpacePos = mul(float4(t_ScreenPos, 2.0f, 1.0f), InvView).xyz;

float3 viewSpaceRay = mul(float4(screenSpaceRay, 1.0f, 0.0f), InvView).xyz;

Ray t_Ray;
//t_Ray.Direction = screenSpaceRay;
t_Ray.Position = t_ViewSpacePos;
t_Ray.Direction = normalize(viewSpaceRay);
t_Ray.Color = float3(0, 0, 0);
t_Ray.lastVertexIndex = -1;
t_Ray.reflectionFactor = 1.0f;

outputRays[outIndex] = t_Ray;

*/