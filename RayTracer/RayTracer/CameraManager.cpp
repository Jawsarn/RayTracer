#include "CameraManager.h"


CameraManager* CameraManager::m_singleton = nullptr;

CameraManager* CameraManager::GetInstance()
{
    if (m_singleton == nullptr)
        m_singleton = new CameraManager();

    return m_singleton;
}

CameraManager::CameraManager()
{
    m_HasMoved = false;
}


CameraManager::~CameraManager()
{
}

void CameraManager::LookAt(XMFLOAT3 p_EyePos, XMFLOAT3 p_FocusPos, XMFLOAT3 p_UpPos)
{

    XMVECTOR p_VecEye = XMLoadFloat3(&p_EyePos);
    XMVECTOR p_VecFocus = XMLoadFloat3(&p_FocusPos);
    XMVECTOR p_VecUp = XMLoadFloat3(&p_UpPos);

    XMMATRIX p_View = XMMatrixLookAtLH(p_VecEye, p_VecFocus, p_VecUp);

    XMStoreFloat4x4(&m_View, p_View);


    m_Position = p_EyePos;
    XMStoreFloat3(&m_Look, p_VecFocus - p_VecEye);
    m_Up = p_UpPos;
    XMVECTOR t_Right = XMVector3Cross(p_VecUp, p_VecFocus - p_VecEye);
    XMStoreFloat3(&m_Right, t_Right);
}

void CameraManager::LookTo(XMFLOAT3 p_EyePos, XMFLOAT3 p_EyeDir, XMFLOAT3 p_UpPos)
{
    XMVECTOR p_VecEye = XMLoadFloat3(&p_EyePos);
    XMVECTOR p_VecEyeDir = XMLoadFloat3(&p_EyeDir);
    XMVECTOR p_VecUp = XMLoadFloat3(&p_UpPos);

    XMMATRIX p_View = XMMatrixLookToLH(p_VecEye, p_VecEyeDir, p_VecUp);

    XMStoreFloat4x4(&m_View, p_View);

    m_Position = p_EyePos;
    m_Look = p_EyeDir;
    m_Up = p_UpPos;
    XMVECTOR t_Right = XMVector3Cross(p_VecUp, p_VecEyeDir);
    XMStoreFloat3(&m_Right, t_Right);
}

void CameraManager::SetPerspective(float p_Angle, float p_Width, float p_Height, float p_Near, float p_Far)
{
    XMMATRIX p_Proj = XMMatrixPerspectiveFovLH(p_Angle, p_Width / p_Height, p_Near, p_Far);
    XMStoreFloat4x4(&m_Proj, p_Proj);
}

void CameraManager::SetOrtogonal(float p_Width, float p_Height, float p_Near, float p_Far)
{
    XMMATRIX p_Proj = XMMatrixOrthographicLH(p_Width, p_Height, p_Near, p_Far);
    XMStoreFloat4x4(&m_Proj, p_Proj);
}


void CameraManager::Walk(float p_Distance)
{
    //prettey damn good
    XMVECTOR t_Singed = XMVectorReplicate(p_Distance);
    XMVECTOR t_Look = XMLoadFloat3(&m_Look);
    XMVECTOR t_Pos = XMLoadFloat3(&m_Position);


    XMStoreFloat3(&m_Position, XMVectorMultiplyAdd(t_Singed, t_Look, t_Pos));
    m_HasMoved = true;
}

void CameraManager::Strafe(float p_Distance)
{
    XMVECTOR t_Singed = XMVectorReplicate(p_Distance);
    XMVECTOR t_Right = XMLoadFloat3(&m_Right);
    XMVECTOR t_Pos = XMLoadFloat3(&m_Position);

    XMStoreFloat3(&m_Position, XMVectorMultiplyAdd(t_Singed, t_Right, t_Pos));
    m_HasMoved = true;
}
void CameraManager::Pitch(float p_Angle)
{
    //Rotate up and look vector about the right vector.
    XMMATRIX t_Right = XMMatrixRotationAxis(XMLoadFloat3(&m_Right), p_Angle);

    XMStoreFloat3(&m_Up, XMVector3TransformNormal(XMLoadFloat3(&m_Up), t_Right));
    XMStoreFloat3(&m_Look, XMVector3TransformNormal(XMLoadFloat3(&m_Look), t_Right));
    m_HasMoved = true;
}
void CameraManager::RotateY(float p_Angle)
{
    // Rotate the basis vectors about the world y-axis.
    XMMATRIX t_Rotation = XMMatrixRotationY(p_Angle);

    XMStoreFloat3(&m_Right, XMVector3TransformNormal(XMLoadFloat3(&m_Right), t_Rotation));
    XMStoreFloat3(&m_Up, XMVector3TransformNormal(XMLoadFloat3(&m_Up), t_Rotation));
    XMStoreFloat3(&m_Look, XMVector3TransformNormal(XMLoadFloat3(&m_Look), t_Rotation));
    m_HasMoved = true;
}

void CameraManager::HoverY(float p_Amount)
{
    m_Position.y += p_Amount;
    m_HasMoved = true;
}


XMMATRIX CameraManager::GetViewProj()
{
    return XMMatrixMultiply(GetView(), GetProj());
}

XMMATRIX CameraManager::GetInvViewProj()
{
    XMMATRIX t_Mat = GetViewProj();
    return XMMatrixInverse(nullptr, t_Mat);
}

XMMATRIX CameraManager::GetView()
{
    return XMLoadFloat4x4(&m_View);
}

XMMATRIX CameraManager::GetProj()
{
    return XMLoadFloat4x4(&m_Proj);
}


XMMATRIX CameraManager::GetInvView()
{
    return XMMatrixInverse(nullptr, GetView());
}

XMMATRIX CameraManager::GetInvProj()
{
    return XMMatrixInverse(nullptr, GetProj());
}

void CameraManager::Update()
{
    if (m_HasMoved)
    {



        XMVECTOR t_Right = XMLoadFloat3(&m_Right);
        XMVECTOR t_Up = XMLoadFloat3(&m_Up);
        XMVECTOR t_Look = XMLoadFloat3(&m_Look);
        XMVECTOR t_Pos = XMLoadFloat3(&m_Position);

        //
        // Orthonormalize the right, up and look vectors.
        //
        // Make look vector unit length.
        t_Look = XMVector3Normalize(t_Look);

        // Compute a new corrected "up" vector and normalize it.
        t_Up = XMVector3Normalize(XMVector3Cross(t_Look, t_Right));

        // Compute a new corrected "right" vector.
        t_Right = XMVector3Cross(t_Up, t_Look);

        //
        // Fill in the view matrix entries.
        //
        float t_X = -XMVectorGetX(XMVector3Dot(t_Pos, t_Right));
        float t_Y = -XMVectorGetX(XMVector3Dot(t_Pos, t_Up));
        float t_Z = -XMVectorGetX(XMVector3Dot(t_Pos, t_Look));

        XMStoreFloat3(&m_Right, t_Right);
        XMStoreFloat3(&m_Up, t_Up);
        XMStoreFloat3(&m_Look, t_Look);

        m_View(0, 0) = m_Right.x;
        m_View(1, 0) = m_Right.y;
        m_View(2, 0) = m_Right.z;
        m_View(3, 0) = t_X;
        m_View(0, 1) = m_Up.x;
        m_View(1, 1) = m_Up.y;
        m_View(2, 1) = m_Up.z;
        m_View(3, 1) = t_Y;
        m_View(0, 2) = m_Look.x;
        m_View(1, 2) = m_Look.y;
        m_View(2, 2) = m_Look.z;
        m_View(3, 2) = t_Z;
        m_View(0, 3) = 0.0f;
        m_View(1, 3) = 0.0f;
        m_View(2, 3) = 0.0f;
        m_View(3, 3) = 1.0f;
        m_HasMoved = false;
    }
}