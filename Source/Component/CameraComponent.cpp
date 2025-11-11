#include "Component/CameraComponent.h"
#include "Core/GameObject.h"

CameraComponent* CameraComponent::s_activeCamera = nullptr;

CameraComponent::CameraComponent(class GameObject& parent, int updateLayer)
    : Component(parent, updateLayer) {
    // 最初のカメラは自動的にアクティブにする
    if (s_activeCamera == nullptr) {
        SetAsActiveCamera();
    }
}

CameraComponent::~CameraComponent() {
    if (s_activeCamera == this) {
        s_activeCamera = nullptr;
    }
}

void CameraComponent::Update(float deltaTime) {
    switch (_controlMode) {
        case ControlMode::NONE:
            // カメラ制御なし
            break;
        case ControlMode::ORBIT:
            // オービットカメラ制御の実装(必要に応じて追加)
            break;
        case ControlMode::FIRST_PERSON:
            UpdateFirstPersonCamera(deltaTime);
            break;
    }
}

void CameraComponent::UpdateFirstPersonCamera(float deltaTime) {
    Vector3 pos = GetParent().Position();

    _pitch += _pitchSpeed * deltaTime;
    _pitch = Math::Clamp(_pitch, -_maxPitch, _maxPitch);

    _pitchSpeed = 0.0f;

    // GameObjectの回転(Yaw)から基本の方向ベクトルを取得
    const Quaternion& yawRotation = GetParent().Rotation();
    Vector3 baseForward = Vector3::Transform(Vector3::UnitZ, yawRotation);
    Vector3 baseRight = Vector3::Transform(Vector3::UnitX, yawRotation);
    
    // Pitchの回転を適用
    Quaternion pitchRotation(baseRight, _pitch);
    
    // 最終的な方向ベクトルを計算してキャッシュ
    _cachedForward = Vector3::Transform(baseForward, pitchRotation);
    _cachedForward.Normalize();
    
    _cachedRight = baseRight;
    _cachedRight.Normalize();
    
    _cachedUp = Vector3::Transform(Vector3::UnitY, pitchRotation);
    _cachedUp.Normalize();

    // ビュー行列も更新
    Vector3 target = pos + _cachedForward * 100.f;
    _viewMatrix = Matrix4x4::CreateLookAtLH(pos, target, _cachedUp);
}

Matrix4x4 CameraComponent::GetViewMatrix() const {
    // FPSモードの場合は計算済みのビュー行列を使用
    if (_controlMode == ControlMode::FIRST_PERSON) {
        return _viewMatrix;
    }

    // 通常モード:GameObjectのワールド変換を使用してビュー行列を作成
    const Vector3& position = GetParent().Position();
    const Vector3 forward = GetForward();
    const Vector3 up = GetUp();
    
    Vector3 target = position + forward;
    
    return Matrix4x4::CreateLookAtLH(position, target, up);
}

Matrix4x4 CameraComponent::GetProjectionMatrix(float aspectRatio) const {
    float fovRadians = Math::ToRadians(_fov);
    return Matrix4x4::CreatePerspectiveFovLH(fovRadians, aspectRatio, _nearClip, _farClip);
}

Vector3 CameraComponent::GetPosition() const {
    return GetParent().Position();
}

Vector3 CameraComponent::GetForward() const {
    // FPSモードの場合はキャッシュされた値を使用
    if (_controlMode == ControlMode::FIRST_PERSON) {
        return _cachedForward;
    }
    
    // 通常モード:GameObjectの回転から前方ベクトルを取得
    const Quaternion& rotation = GetParent().Rotation();
    Vector3 forward = Vector3::Transform(Vector3::UnitZ, rotation);
    forward.Normalize();
    return forward;
}

Vector3 CameraComponent::GetRight() const {
    // FPSモードの場合はキャッシュされた値を使用
    if (_controlMode == ControlMode::FIRST_PERSON) {
        return _cachedRight;
    }
    
    // 通常モード:GameObjectの回転から右方向ベクトルを取得
    const Quaternion& rotation = GetParent().Rotation();
    Vector3 right = Vector3::Transform(Vector3::UnitX, rotation);
    right.Normalize();
    return right;
}

Vector3 CameraComponent::GetUp() const {
    // FPSモードの場合はキャッシュされた値を使用
    if (_controlMode == ControlMode::FIRST_PERSON) {
        return _cachedUp;
    }
    
    // 通常モード:GameObjectの回転から上方向ベクトルを取得
    const Quaternion& rotation = GetParent().Rotation();
    Vector3 up = Vector3::Transform(Vector3::UnitY, rotation);
    up.Normalize();
    return up;
}

void CameraComponent::SetAsActiveCamera() {
    s_activeCamera = this;
    //SDL_Log("CameraComponent set as active: %s", GetParent().Name().c_str());
}
