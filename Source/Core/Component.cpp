#include "Core/Component.h"
#include "Core/GameObject.h"

Component::Component(GameObject& parent, int updateLayer)
	: _parent(parent)
	, _state(STATE::ACTIVE)
	, _updateLayer(updateLayer) {

}

Component::~Component() {
	//_parent.RemoveComponent(this);
}

#pragma region パブリック関数


#pragma endregion