#include <SDL3/SDL.h>
#include <algorithm>
#include "Input/InputAction.h"
#include "Input/InputManager.h"
#include "Core/Game.h"

InputAction::InputAction(InputManager& inputManager)
	: _inputManager(inputManager) {

}

InputAction::~InputAction()
{
}
//
//void InputAction::MapAction(const std::string& name, InputCode code, int playerIndex) {
//	_actionMap[name].push_back(code);
//}
//
//bool InputAction::IsAction(const std::string& name, int player) const {
//	auto iter = _actionMap.find(name);
//	if (iter == _actionMap.end()) {
//		return false;
//	}
//
//	const std::vector<InputCode>& codes = iter->second;
//
//	// キーボード入力のチェック
//	for (const auto& code : codes) {
//		if (std::holds_alternative<SDL_Scancode>(code) &&
//			IsKey(std::get<SDL_Scancode>(code))) {
//			return true;
//		}
//	}
//
//	// マウス入力のチェック
//	for (const auto& code : codes) {
//		if (std::holds_alternative<SDL_MouseButton>(code) &&
//			IsMouseButton(std::get<SDL_MouseButton>(code))) {
//			return true;
//		}
//	}
//
//	// ゲームパッド入力のチェック
//	for (const auto& code : codes) {
//		if (std::holds_alternative<SDL_GamepadButton>(code) &&
//			IsGamepadButton(std::get<SDL_GamepadButton>(code))) {
//			return true;
//		}
//	}
//
//	return false;
//}
//
//void InputAction::MapActionAxis(const std::string& name, InputCode negative, InputCode positive) {
//	_inputManager.SetAxisMapping(name, negative, positive);
//	_axisMap[name].push_back(name);
//}
//
//void InputAction::MapActionAxis(const std::string& name, SDL_GamepadAxis axis) {
//	_axisMap[name].push_back(axis);
//}
//
//bool InputAction::IsActionDown(const std::string& name, int player) const {
//	auto iter = _actionMap.find(name);
//	if (iter == _actionMap.end()) {
//		return false;
//	}
//
//	const std::vector<InputCode>& codes = iter->second;
//
//	// キーボード入力のチェック
//	for (const auto& code : codes) {
//		if (std::holds_alternative<SDL_Scancode>(code) &&
//			IsKeyDown(std::get<SDL_Scancode>(code))) {
//			return true;
//		}
//	}
//
//	// マウス入力のチェック
//	for (const auto& code : codes) {
//		if (std::holds_alternative<SDL_MouseButton>(code) &&
//			IsMouseButtonDown(std::get<SDL_MouseButton>(code))) {
//			return true;
//		}
//	}
//
//	// ゲームパッド入力のチェック
//	for (const auto& code : codes) {
//		if (std::holds_alternative<SDL_GamepadButton>(code) &&
//			IsGamepadButtonDown(std::get<SDL_GamepadButton>(code))) {
//			return true;
//		}
//	}
//
//	return false;
//}
//
//bool InputAction::IsActionUp(const std::string& name, int player) const {
//	// 実装を追加
//	return false;
//}
//
//float InputAction::GetActionValue(const std::string& name, int player) const {
//	auto iter = _axisMap.find(name);
//	if (iter == _axisMap.end()) {
//		return 0.0f;
//	}
//
//	const std::vector<AxisCode>& codes = iter->second;
//
//	float value = 0.0f;
//
//	for (const auto& code : codes) {
//		if (std::holds_alternative<std::string>(code)) {
//			value = Math::Abs(value) > Math::Abs(_inputManager.GetAxis(std::get<std::string>(code))) ? value : _inputManager.GetAxis(std::get<std::string>(code));
//		}
//	}
//
//	for (const auto& code : codes) {
//		if (std::holds_alternative<SDL_GamepadAxis>(code)) {
//			value = Math::Abs(value) > Math::Abs(GetGamepadAxis(std::get<SDL_GamepadAxis>(code))) ? value : GetGamepadAxis(std::get<SDL_GamepadAxis>(code));
//		}
//	}
//
//	return value;
//}
//
//Vector2 InputAction::GetActionVector(const std::string& name, int player) const {
//	// 実装を追加
//	return Vector2::Zero;
//}
//
//// キーボード入力
//bool InputAction::IsKeyDown(SDL_Scancode key) const {
//	return _inputManager.GetState().keyboard.GetKeyState(key) == BUTTON_STATE::DOWN;
//
//}
//
//bool InputAction::IsKeyUp(SDL_Scancode key) const {
//	return _inputManager.GetState().keyboard.GetKeyState(key) == BUTTON_STATE::UP;
//}
//
//bool InputAction::IsKey(SDL_Scancode key) const {
//
//	return _inputManager.GetState().keyboard.GetKeyState(key) == BUTTON_STATE::PRESS;
//}
//
//bool InputAction::IsAllKey() const {
//	return _inputManager.GetState().keyboard.IsAllKey();
//}
//
//// マウス入力 
//bool InputAction::IsMouseButtonDown(SDL_MouseButton button) const {
//	return _inputManager.GetState().mouse.GetButtonState(button) == BUTTON_STATE::DOWN;
//}
//
//bool InputAction::IsMouseButtonUp(SDL_MouseButton button) const {
//
//	return _inputManager.GetState().mouse.GetButtonState(button) == BUTTON_STATE::UP;
//}
//
//bool InputAction::IsMouseButton(SDL_MouseButton button) const {
//
//	return _inputManager.GetState().mouse.GetButtonState(button) == BUTTON_STATE::PRESS;
//}
//
//Vector2 InputAction::GetMousePosition() const {
//	return _inputManager.GetState().mouse.GetPosition();
//}
//
//void InputAction::SetRelativeMouseMode(bool isRelative) {
//	SDL_SetWindowRelativeMouseMode(_inputManager.GetWindow(), isRelative);
//	_inputManager.SetRelativeMouseMode(isRelative);
//}
//
//// ゲームパッド入力 
//bool InputAction::IsGamepadButtonDown(SDL_GamepadButton button) const {
//	return _inputManager.GetState().gamepad.GetButtonState(button) == BUTTON_STATE::DOWN;
//}
//
//bool InputAction::IsGamepadButtonUp(SDL_GamepadButton button) const {
//
//	return _inputManager.GetState().gamepad.GetButtonState(button) == BUTTON_STATE::UP;
//}
//
//bool InputAction::IsGamepadButton(SDL_GamepadButton button) const {
//
//	return _inputManager.GetState().gamepad.GetButtonState(button) == BUTTON_STATE::PRESS;
//
//}
//
//bool InputAction::IsAllGamepadButton() const {
//	return _inputManager.GetState().gamepad.IsAllButton();
//}
//
//float InputAction::GetGamepadAxis(SDL_GamepadAxis axis) const {
//	return _inputManager.GetState().gamepad.GetAxis(axis);
//}
//
//Vector2 InputAction::GetGamepadStickAxis(SDL_GamepadStick stick) const {
//	return _inputManager.GetState().gamepad.GetStickAxis(stick);
//}
