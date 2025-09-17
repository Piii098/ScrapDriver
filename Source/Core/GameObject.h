#pragma once
#include <rapidjson/document.h>
#include "Framework.h"
#include "Utility/Utility.h"

///@brief ゲーム内のオブジェクトの基本クラス
///
///コンポーネントを持ち、入力処理、更新処理を行う
///子オブジェクトを持つことができる
///DEAD状態のオブジェクトは破棄される
class GameObject {
public:

    // ===== コンストラクタ・デストラクタ =====　//

    GameObject(Scene& scene);

    GameObject(GameObject* parent);

    virtual ~GameObject();

    virtual bool Initialize(const std::string& name = "");

    // ===== ループ処理 ===== //

    /// @brief 入力処理
    /// コンポーネントと独自の入力処理を呼び出す
    /// シーンより呼び出される
    /// @param input 入力情報
    void Input(const class InputAction& input);

    /// @brief 更新処理
    /// コンポーネントと独自の入力処理を呼び出す
    /// シーンより呼び出される
    /// @param deltaTime フレーム間の時間
    void Update(float deltaTime);

    /// @brief 物理更新処理
    /// コンポーネントと独自の入力処理を呼び出す
    /// シーンより呼び出される 
    /// @param deltaTime フレーム間の時間
    void FixedUpdate(float deltaTime);

    void LateUpdate(float deltaTime);

    void Cleanup();


    // ====== ワールド変換行列関連 ===== //

    /// @brief ワールド変換行列の計算
    /// 位置、スケール、回転からワールド変換行列を計算する
    void ComputeWorldTransform();


    // ===== コンポーネント関連 ===== //

    /// @brief コンポーネントの削除
    /// コンポーネントを削除する
    /// @param component 削除するコンポーネント 
    void RemoveComponent(Component* component);

    /// @brief コンポーネントの生成
    /// コンポーネントを生成し、追加する
    /// @tparam T 生成するコンポーネントの型
    /// @tparam Args コンストラクタに渡す引数の型
    /// @param args コンストラクタに渡す引数 ( 第一引数のGameObjectへの参照は自動で設定される )
    /// @return 生成したコンポーネントへの参照( 一時的な参照を推奨 )
    template<typename T, typename... Args>
    T& CreateComponent(Args&&... args);

    /// @brief コンポーネントの取得
    /// 指定した型のコンポーネントを取得する
    /// @tparam T コンポーネントの型
    /// @return コンポーネントのポインタ
    template <typename T>
    T* GetComponent();

    void DestroyComponent(Component* component);

    void Destroy();
    void AddChildObject(GameObject* child);
    void RemoveChildObject(GameObject* child);

    bool IsEqual(const GameObject& object) const { return this == &object; } ///< 自分と同じオブジェクトかどうかを判定する

    void LoadProperties(const rapidjson::Value& inObj);

    // ===== ゲッターセッター ===== //

	void Name(const std::string& name) { _name = name; }
    void Position(const Vector3& position) { _position = position; _recomputeWorldTransform = true; }
    void Position(float x, float y, float z) { _position = Vector3(x, y, z); _recomputeWorldTransform = true; }
    void Position2D(const Vector2& position) { _position = Vector3(round(position.x), round(position.y), Position().z); _recomputeWorldTransform = true; }
    void Position2D(float x, float y) { _position = Vector3(round(x), round(y), Position().z); _recomputeWorldTransform = true; }
    void PositionPercent(const Vector2& percent);
    void Scale(const Vector3& scale) { _scale = scale; _recomputeWorldTransform = true; }
    void Rotation(Quaternion rotation) { _rotation = rotation; _recomputeWorldTransform = true; }
    void State(STATE state) { _state = state; }
    void SetParent(GameObject* parent) { _parent = parent; }

    const STATE State() const { return _state; }
    const Vector3& Scale() const { return _scale; }
    const Quaternion Rotation() const { return _rotation; }
    const Matrix4x4& WorldTransform() const { return _worldTransform; }
    const Vector3& Position() const { return _position; }
    const Vector3& Position() { return _position; }
    const Vector2& Position2D() { return Vector2(_position.x, _position.y); }
    //const Vector3& GetRight() { return Vector3::Transform(Vector3::UnitX, _rotation); } ///< 右方向ベクトルを取得する
    //const Vector3& GetForward() { return Vector3::Transform(Vector3::UnitZ, _rotation); } ///< 前方ベクトルを取得する

private:

    // ===== ループ処理 ===== //

    /// @brief コンポーネントの入力処理
    /// アクティブ状態のコンポーネントの入力処理を行う
    /// @param input 入力情報
    void InputComponents(const InputAction& input);

    /// @brief オブジェクトの入力処理
    /// 仮想関数、オブジェクト独自の入力処理を行う
    /// @param input 
    virtual void InputObject(const InputAction& input) {};

    /// @brief コンポーネントの更新処理
    /// アクティブ状態のコンポーネントの更新処理を行う
    /// @param deltaTime フレーム間の時間
    void UpdateComponents(const float deltaTime);

    /// @brief オブジェクトの更新処理
    /// 仮想関数、オブジェクト独自の更新処理を行う
    /// @param deltaTime フレーム間の時間
    virtual void UpdateObject(const float deltaTime) {};

    /// @brief コンポーネントの物理更新処理
    /// アクティブ状態のコンポーネントの物理更新処理を行う
    /// @param deltaTime フレーム間の時間
    void FixedUpdateComponents(const float deltaTime);

    /// @brief オブジェクトの物理更新処理
    /// 仮想関数、オブジェクト独自の物理更新処理を行う
    /// @param deltaTime フレーム間の時間
    virtual void FixedUpdateObject(const float deltaTime) {};

    void LateUpdateComponents(const float deltaTime);

    virtual void LateUpdateObject(const float deltaTime) {};


    // ===== オブジェクト関連 ===== //

    /// @brief コンポーネントの追加
    /// UpdateLayerの値が小さい順に挿入する
    /// @param component 追加するコンポーネント
    void AddComponent(std::unique_ptr<Component> component);


    // ===== メンバ変数 ===== //

	std::string _name;                      ///< オブジェクト名

	// トランスフォーム関連
    Vector3 _position;                       ///< 位置（整数）
    Quaternion _rotation;                     ///< 回転
    Vector3 _scale;                               ///< スケール

    // シーン関連
    Scene& _scene;                              ///< 所属するシーン
    GameObject* _parent;                        ///< 親オブジェクト

    // 状態関連
    STATE _state;                               ///< オブジェクトの状態

    // コンポーネント
    std::vector <std::unique_ptr<Component>> _components;        ///< コンポーネントリスト
    std::vector <GameObject*> _children;          ///< 子オブジェクトリスト

    // トランスフォーム関連
    Matrix4x4 _worldTransform;                    ///< ワールド変換行列
    bool _recomputeWorldTransform;              ///< 変換行列の再計算フラグ

};


template<typename T, typename... Args>
T& GameObject::CreateComponent(Args&&... args) {
    auto component = std::make_unique<T>(*this, std::forward<Args>(args)...);
    T& compRef = *component; // 生ポインタではなく参照を取得
    _components.emplace_back(std::move(component));
    return compRef;
}

template <typename T>
T* GameObject::GetComponent() {
    for (auto& comp : _components) {
        if (T* t = dynamic_cast<T*>(comp.get())) { // unique_ptrから生ポインタを取得
            return t;
        }
    }
    return nullptr;
}