#pragma once
#include <variant>
#include <SDL3/SDL.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <SDL3/SDL_keycode.h>
#include <memory>
#include "Utility/Math.h"

/// @brief マウスボタン
    /// マウスのボタンを表す(SDLには存在しないため自作)
enum class SDL_MouseButton {
    SDL_MOUSE_LEFT = 1,
    SDL_MOUSE_RIGHT = 2,
    SDL_MOUSE_MIDDLE = 3,
    SDL_MOUSE_X1 = 4,
    SDL_MOUSE_X2 = 5
};

/// @brief ゲームパッドのスティック
/// ゲームパッドのスティックを表す(SDLには存在しないため自作)
enum class SDL_GamepadStick {
    SDL_GAMEPAD_LEFT_STICK = 1,
    SDL_GAMEPAD_RIGHT_STICK = 2
};

enum class SDL_GamepadStickButton {

};

/// @brief マウスボタンのマスクを取得
/// マウスボタンのマスクを取得する
inline Uint32 GetMouseButtonMask(SDL_MouseButton button) {
    return 1u << (static_cast<int>(button) - 1);
}

/// @brief 複数の入力をまとめた型
/// 入力アクションを使用する際に使用する
using InputCode = std::variant<std::monostate, SDL_Scancode, SDL_GamepadButton, SDL_MouseButton>;

using AxisCode = std::variant<std::monostate, SDL_GamepadAxis, std::string>;

class InputAction {
public:

	InputAction(class InputManager& inputManager);
	~InputAction();

	void MapAction(const std::string& name, InputCode code, int playerIndex = 0); // 入力アクションを登録

	void MapActionAxis(const std::string& name, InputCode negative, InputCode positive);
	void MapActionAxis(const std::string& name, SDL_GamepadAxis axis); // 入力アクションを登録

	bool IsAction(const std::string& name, int player = 0) const; // 入力があるかどうか
	bool IsActionDown(const std::string& name, int player = 0) const; // 入力があった瞬間かどうか
	bool IsActionUp(const std::string& name, int player = 0) const; // 入力が離された瞬間かどうか

	float GetActionValue(const std::string& name, int player = 0) const; // 入力の値を取得
	Vector2 GetActionVector(const std::string& name, int player = 0) const; // 入力のベクトルを取得

	bool IsKeyDown(SDL_Scancode key) const;
	bool IsKeyUp(SDL_Scancode key) const;
	bool IsKey(SDL_Scancode key) const;

	bool IsAllKey() const;

	bool IsMouseButtonDown(SDL_MouseButton button) const;
	bool IsMouseButtonUp(SDL_MouseButton button) const;
	bool IsMouseButton(SDL_MouseButton button) const;
	//Vector2 GetMousePosition() const;
	void SetRelativeMouseMode(bool flag);

	bool IsGamepadButtonDown(SDL_GamepadButton button) const;
	bool IsGamepadButtonUp(SDL_GamepadButton button) const;
	bool IsGamepadButton(SDL_GamepadButton button) const;
	bool IsAllGamepadButton() const;

	float GetGamepadAxis(SDL_GamepadAxis axis) const;
	//Vector2 GetGamepadStickAxis(SDL_GamepadStick stick) const;

private:
    class InputManager& _inputManager;

	std::unordered_map<std::string, std::vector<InputCode>> _actionMap; // アクションマップ
	std::unordered_map<std::string, std::vector<AxisCode>> _axisMap; // アクションマップ
};
