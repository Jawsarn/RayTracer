#pragma once
#include <DirectXMath.h>

using namespace DirectX;


class CameraManager
{
public:
    static CameraManager* GetInstance();
    void LookAt(XMFLOAT3 p_EyePos, XMFLOAT3 p_FocusPos, XMFLOAT3 p_UpPos);
    void LookTo(XMFLOAT3 p_EyePos, XMFLOAT3 p_EyeDir, XMFLOAT3 p_UpPos);
    void SetPerspective(float p_Angle, float p_Width, float p_Height, float p_Near, float p_Far);
    void SetOrtogonal(float p_Width, float p_Height, float p_Near, float p_Far);

    void Walk(float p_Amount);
    void Strafe(float p_Amount);
    void HoverY(float p_Amount);
    void Pitch(float p_Dy);
    void RotateY(float p_Dx);
    void Update();


    XMMATRIX GetViewProj();
    XMMATRIX GetInvViewProj();
    XMMATRIX GetView();
    XMMATRIX GetProj();
    XMMATRIX GetInvView();
    XMMATRIX GetInvProj();
    XMFLOAT3 GetPosition() { return m_Position; };

private:

    CameraManager();
    ~CameraManager();

    static CameraManager* m_singleton;

    XMFLOAT4X4 m_View;
    XMFLOAT4X4 m_Proj;

    XMFLOAT3 m_Position;
    XMFLOAT3 m_Look;
    XMFLOAT3 m_Up;
    XMFLOAT3 m_Right;

    bool m_HasMoved;

};

