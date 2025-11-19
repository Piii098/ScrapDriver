#pragma once
#include "Core/Component.h"
#include "Utility/Math.h"

class CameraComponent : public Component {
public:
    enum class ControlMode {
        NONE,
        ORBIT,
        FIRST_PERSON,
    };

    CameraComponent(class GameObject& parent, int updateLayer = 100);
    ~CameraComponent();

    void Update(float deltaTime) override;

    // カメラパラメータ
    void SetFOV(float fov) { _fov = fov; }
    void SetNearClip(float nearClip) { _nearClip = nearClip; }
    void SetFarClip(float farClip) { _farClip = farClip; }

    float GetFOV() const { return _fov; }
    float GetNearClip() const { return _nearClip; }
    float GetFarClip() const { return _farClip; }

    // カメラ制御モード
    void SetControlMode(ControlMode mode) { _controlMode = mode; }
    ControlMode GetControlMode() const { return _controlMode; }

    // FPS カメラ制御用
    void SetPitchSpeed(float speed) { _pitchSpeed = speed; }
    void AddPitch(float deltaPitch) { _pitch += deltaPitch; } // 直接pitchに加算（マウス入力用）
    void SetMaxPitch(float max) { _maxPitch = max; }
    void SetPitch(float pitch) { _pitch = pitch; }
    
    float GetPitchSpeed() const { return _pitchSpeed; }
    float GetMaxPitch() const { return _maxPitch; }
    float GetPitch() const { return _pitch; }

    // カメラのビュー行列を取得
    Matrix4x4 GetViewMatrix() const;
    Matrix4x4 GetProjectionMatrix(float aspectRatio) const;

    // レイトレーシング用のカメラ情報
    Vector3 GetPosition() const;
    Vector3 GetForward() const;
    Vector3 GetRight() const;
    Vector3 GetUp() const;

    // アクティブなカメラとして設定
    void SetAsActiveCamera();

    // 静的メソッド:アクティブなカメラを取得
    static CameraComponent* GetActiveCamera() { return s_activeCamera; }

private:

    void UpdateFirstPersonCamera(float deltaTime);

    ControlMode _controlMode = ControlMode::NONE; // カメラ制御モード

    // FPS カメラ用パラメータ
    float _pitchSpeed = 0.0f;
    float _maxPitch = Math::Pi / 3.0f;
    float _pitch = 0.0f;

    float _fov = 60.0f;          // 視野角(度)
    float _nearClip = 0.1f;      // ニアクリップ
    float _farClip = 1000.0f;    // ファークリップ

    Matrix4x4 _viewMatrix;       // ビュー行列(FPSモード用)
    
    // FPSモード用のキャッシュされた方向ベクトル
    Vector3 _cachedForward = Vector3::UnitZ;
    Vector3 _cachedRight = Vector3::UnitX;
    Vector3 _cachedUp = Vector3::UnitY;

    static CameraComponent* s_activeCamera; // アクティブなカメラ
};
