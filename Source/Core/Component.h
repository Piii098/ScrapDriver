#pragma once
#include "Utility/Utility.h"

/// @brief ゲームオブジェクトのコンポーネントクラス
///	
/// ゲームオブジェクトにアタッチされるコンポーネントの基本クラス
class Component {
public:
	// ===== コンストラクタ・デストラクタ ===== //

	Component(class GameObject& parent, int updateLayer = 100);
	virtual ~Component();

	virtual void CleanUp() {};

	// ===== ループ処理 ===== //

	/// @brief 入力処理
	/// コンポーネントの入力処理を行う
	/// /// @param input 入力情報
	virtual void ProcessInput(const class InputAction& input) {};

	/// @brief 更新処理
	/// コンポーネントの更新処理を行う
	/// @param deltaTime フレーム間の時間
	virtual void Update(float deltaTime) {};

	/// @brief 物理更新処理
	/// コンポーネントの物理更新処理を行う
	/// /// @param deltaTime フレーム間の時間
	virtual void FixedUpdate(float deltaTime) {};

	virtual void LateUpdate(float deltaTime) {};

	virtual void Render() {};

	/// @brief ワールド変換行列の計算
	/// コンポーネントのワールド変換行列を計算する
	virtual void OnUpdateWorldTransform() {};

	// ===== ゲッターセッター ===== //

	GameObject& GetParent() const { return _parent; };
	int UpdateLayer() const { return _updateLayer; };

	STATE State() const { return _state; };
	void State(STATE state) { _state = state; };

protected:
	STATE _state;
	GameObject& _parent;

	int _updateLayer;
};