#pragma once
#include "Core/Component.h"
#include "Utility/Math.h"

class Camera : public Component {
public:
    Camera(class GameObject& parent, int updateLayer = 100);
    ~Camera();

    // カメラパラメータ
    void SetFOV(float fov) { _fov = fov; }
    void SetNearClip(float nearClip) { _nearClip = nearClip; }
    void SetFarClip(float farClip) { _farClip = farClip; }

    float GetFOV() const { return _fov; }
    float GetNearClip() const { return _nearClip; }
    float GetFarClip() const { return _farClip; }

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

    // 静的メソッド：アクティブなカメラを取得
    static Camera* GetActiveCamera() { return s_activeCamera; }

private:
    float _fov = 60.0f;          // 視野角（度）
    float _nearClip = 0.1f;      // ニアクリップ
    float _farClip = 1000.0f;    // ファークリップ

    static Camera* s_activeCamera; // アクティブなカメラ
};
