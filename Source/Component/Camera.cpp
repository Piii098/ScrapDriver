#include "Component/Camera.h"
#include "Core/GameObject.h"

Camera* Camera::s_activeCamera = nullptr;

Camera::Camera(class GameObject& parent, int updateLayer)
    : Component(parent, updateLayer) {
    // 最初のカメラは自動的にアクティブにする
    if (s_activeCamera == nullptr) {
        SetAsActiveCamera();
    }
}

Camera::~Camera() {
    if (s_activeCamera == this) {
        s_activeCamera = nullptr;
    }
}

Matrix4x4 Camera::GetViewMatrix() const {
    // GameObjectのワールド変換を使用してビュー行列を作成
    const Vector3& position = GetParent().Position();
    const Vector3 forward = GetForward();
    const Vector3 up = GetUp();
    
    Vector3 target = position + forward;
    
    return Matrix4x4::CreateLookAtLH(position, target, up);
}

Matrix4x4 Camera::GetProjectionMatrix(float aspectRatio) const {
    float fovRadians = Math::ToRadians(_fov);
    return Matrix4x4::CreatePerspectiveFovLH(fovRadians, aspectRatio, _nearClip, _farClip);
}

Vector3 Camera::GetPosition() const {
    return GetParent().Position();
}

Vector3 Camera::GetForward() const {
    // GameObjectの回転から前方ベクトルを取得
    const Quaternion& rotation = GetParent().Rotation();
    Vector3 forward = Vector3::Transform(Vector3::UnitZ, rotation);
    forward.Normalize();
    return forward;
}

Vector3 Camera::GetRight() const {
    // GameObjectの回転から右方向ベクトルを取得
    const Quaternion& rotation = GetParent().Rotation();
    Vector3 right = Vector3::Transform(Vector3::UnitX, rotation);
    right.Normalize();
    return right;
}

Vector3 Camera::GetUp() const {
    // GameObjectの回転から上方向ベクトルを取得
    const Quaternion& rotation = GetParent().Rotation();
    Vector3 up = Vector3::Transform(Vector3::UnitY, rotation);
    up.Normalize();
    return up;
}

void Camera::SetAsActiveCamera() {
    s_activeCamera = this;
    //SDL_Log("Camera set as active: %s", GetParent().Name().c_str());
}
