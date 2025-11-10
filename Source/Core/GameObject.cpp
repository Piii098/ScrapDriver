#include <algorithm>
#include "Core/Scene.h"
#include "Core/GameObject.h"
#include "Core/Component.h"
#include "LevelLoader/LevelLoader.h"
#include "Input/InputAction.h"

//==============================================================================
// コンストラクタ・デストラクタ
//==============================================================================

/**
 * コンストラクタ
 *
 * 親オブジェクトを持たない
 */
GameObject::GameObject(Scene& scene)
	: _state(STATE::ACTIVE)
	, _scene(scene)
	, _position(Vector3::Zero)
	, _scale(Vector3::One)
	, _rotation(Quaternion::Identity)
	, _recomputeWorldTransform(true) {
	SetParent(nullptr);
}

GameObject::GameObject(GameObject* parent)
	: _state(STATE::ACTIVE)
	, _scene(parent->_scene)
	, _position(Vector3::Zero)
	, _scale(Vector3::One)
	, _rotation(Quaternion::Identity)
	, _recomputeWorldTransform(true) {
	parent->AddChildObject(this);
	SetParent(parent);
}

/**
 * デストラクタ
 *
 * 親オブジェクトから自身を削除する
 * 子オブジェクトも全て削除する
 */
GameObject::~GameObject() {
	// 親から自分を削除
	if (_parent != nullptr) {
		_parent->RemoveChildObject(this);
	}
	
	// 全ての子オブジェクトを削除（逆順で削除して安全に）
	while (!_children.empty()) {
		auto child = _children.back();
		_children.pop_back();
		child->Destroy();
	}
}


//==============================================================================
// パブリック関数
//==============================================================================

bool GameObject::Initialize(const std::string& name) {
	_name = name;
	SDL_Log("GameObject Initialize : %s", _name.c_str());
	return true;
}

void GameObject::Cleanup() {
	// コンポーネントのクリーンアップ処理を呼び出す
	for (auto& comp : _components) {
		//comp->Cleanup();
	}
}

// ===== ループ処理 ===== //

/**
 * 入力処理
 *
 * コンポーネントと独自の入力処理を呼び出す
 * アクティブ状態でなければ処理しない
 */
void GameObject::Input(const InputAction& input) {

	if (_state != STATE::ACTIVE) return;

	InputComponents(input);
	InputObject(input);
}

/**
 * 更新処理
 *
 * コンポーネントと独自の更新処理を呼び出し、ワールド変換行列を計算する
 * アクティブ状態でなければ処理しない
 */
void GameObject::Update(const float deltaTime) {

	if (_state != STATE::ACTIVE) return;

	ComputeWorldTransform();

	UpdateComponents(deltaTime);
	UpdateObject(deltaTime);

	ComputeWorldTransform();

}

/**
 * 物理更新処理
 *
 * コンポーネントと独自の物理更新処理を呼び出し、ワールド変換行列を計算する
 * アクティブ状態でなければ処理しない
 */
void GameObject::FixedUpdate(const float deltaTime) {

	if (_state != STATE::ACTIVE) return;

	ComputeWorldTransform();

	FixedUpdateComponents(deltaTime);
	FixedUpdateObject(deltaTime);

	ComputeWorldTransform();

}

void GameObject::LateUpdate(const float deltaTime) {

	if (_state != STATE::ACTIVE) return;

	LateUpdateComponents(deltaTime);
	LateUpdateObject(deltaTime);
}

void GameObject::Render() {
	if (_state != STATE::ACTIVE) return;
	for (auto& comp : _components) {
		comp->Render();
	}
}

// ====== コンポーネント関連 ===== //

/**
 * コンポーネントの削除
 *
 * コンポーネントを検索し削除する
 */
void GameObject::RemoveComponent(Component* component) {
	/*
	// 検索
	auto comp = std::find_if(_components.begin(), _components.end(),
		[component](const std::unique_ptr<Component>& comp) {
			return comp.get() == component; // ポインタが一致するか
		});

	if (comp != _components.end()) {
		_components.erase(comp);
	}
	*/
}

void GameObject::DestroyComponent(Component* component) {
	auto iter = std::find_if(_components.begin(), _components.end(),
		[component](const std::unique_ptr<Component>& comp) {
			return comp.get() == component; // ポインタが一致するか
		});
	if (iter != _components.end()) {
		_components.erase(iter);
	}
}

void GameObject::AddChildObject(GameObject* child) {
	_children.push_back(child);
}

void GameObject::RemoveChildObject(GameObject* child) {

	auto iter = std::find(_children.begin(), _children.end(), child);
	if (iter != _children.end()) {
		_children.erase(iter);
	}
}

void GameObject::Destroy() {
	// 親から削除（デストラクタでも行うが、明示的な削除の場合は先に行う）
	if (_parent != nullptr) {
		_parent->RemoveChildObject(this);
		_parent = nullptr; // 親への参照をクリア
	}
	
	// 子オブジェクトのコピーを作成してから削除（イテレータの無効化を防ぐ）
	std::vector<GameObject*> childrenCopy = _children;
	_children.clear(); // 先にクリアしてデストラクタでの重複削除を防ぐ
	
	for (auto& child : childrenCopy) {
		child->Destroy();
	}
	
	// シーンから削除（これによりunique_ptrが解放され、デストラクタが呼ばれる）
	_scene.DestroyObject(this);
}

// ====== ワールド変換行列関連 ===== //

/**
 * ワールド変換行列の計算
 * 位置、スケール、回転からワールド変換行列を計算する
 */
void GameObject::ComputeWorldTransform() {
	if (_recomputeWorldTransform) { // 再計算フラグが立っている場合のみ計算
		_recomputeWorldTransform = false;

		// 自身のワールド変換行列を計算
		_worldTransform = Matrix4x4::CreateScale(_scale);
		_worldTransform *= Matrix4x4::CreateFromQuaternion(_rotation);
		_worldTransform *= Matrix4x4::CreateTranslation(_position);

		// 親オブジェクトがある場合は親のワールド変換行列を乗算
		if (_parent != nullptr) {
			_worldTransform *= _parent->WorldTransform();
		}

		// すべてのコンポーネントのワールド変換行列を更新
		for (auto& comp : _components) {
			comp->OnUpdateWorldTransform();
		}

		// 子オブジェクトのワールド変換行列も再計算
		for (auto& child : _children) {
			// 子オブジェクトに再計算フラグを立てる
			child->_recomputeWorldTransform = true;
			// 子オブジェクトの変換行列を再計算
			child->ComputeWorldTransform();
		}
	}
}

void GameObject::LoadProperties(const rapidjson::Value& inObj) {
	//std::string state;
	//if (JsonHelper::GetString(inObj, "state", state)) {
	//}
	//JsonHelper::GetVector3(inObj, "positioz", _position);
	//JsonHelper::GetQuaternion(inObj, "rotation", _rotation);
	//JsonHelper::GetFloat(inObj, "scale", _scale);
	//ComputeWorldTransform(); // ワールド変換行列を計算
}

// ====== ゲッターセッター ===== //

void GameObject::PositionPercent(const Vector2& percent) {
	//_position.x = percent.x * _scene.GetManager().GetGame().WinfowWidth();
	//_position.y = percent.y * _scene.GetManager().GetGame().WinfowHeight();
	//_recomputeWorldTransform = true; // ワールド変換行列を再計算するフラグを立てる
}

void GameObject::MarkChildrenDirty() {
	for (auto& child : _children) {
		child->_recomputeWorldTransform = true;
		child->MarkChildrenDirty(); // 再帰的に子孫全てに適用
	}
}

//==============================================================================
// プライベート関数
//==============================================================================

/**
 * コンポーネントの追加
 *
 * コンポーネントのupdateLayerを取得し、その値を比較して挿入する
 * 所有権は移動する
 */
void GameObject::AddComponent(std::unique_ptr<Component> component) {

	/*コンポーネントのupdateLayerの値に応じて昇順に並べる*/

	int layer = component->UpdateLayer();
	auto iter = _components.begin();

	for (; iter != _components.end(); ++iter) {

		if (layer < (*iter)->UpdateLayer()) { // 階層数が既存のコンポーネントより小さければ
			break; // 抜ける
		}

	}

	//_components.insert(iter, std::move(component));
}


// ====== ループ処理 ===== //

/**
 * 更新処理
 *
 * コンポーネントの更新処理を呼び出す
 */
void GameObject::InputComponents(const InputAction& input) {
	for (auto& comp : _components) {
		if (comp->State() == STATE::ACTIVE) { // アクティブ状態のコンポーネントのみ入力処理
			comp->ProcessInput(input);
		}
	}
}

/**
 * 更新処理
 *
 * コンポーネントの更新処理を呼び出す
 */
void GameObject::UpdateComponents(const float deltaTime) {
	for (auto& comp : _components) {
		if (comp->State() == STATE::ACTIVE) { // アクティブ状態のコンポーネントのみ更新
			comp->Update(deltaTime);
		}
	}
}

/**
 * 物理更新処理
 *
 * コンポーネントの物理更新処理を呼び出す
 */
void GameObject::FixedUpdateComponents(const float deltaTime) {
	for (auto& comp : _components) {
		if (comp->State() == STATE::ACTIVE) { // アクティブ状態のコンポーネントのみ更新
			comp->FixedUpdate(deltaTime);
		}
	}
}

/**
 * レイト更新処理
 *
 * コンポーネントのレイト更新処理を呼び出す
 */
void GameObject::LateUpdateComponents(const float deltaTime) {
	for (auto& comp : _components) {
		if (comp->State() == STATE::ACTIVE) { // アクティブ状態のコンポーネントのみ更新
			comp->LateUpdate(deltaTime);
		}
	}
}
