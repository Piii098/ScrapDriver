#include "Component/RigidbodyComponent.h"
#include "Core/GameObject.h"
#include "Utility/Math.h"
#include "Core/Game.h"
#include "Core/Scene.h"
#include "Core/SceneManager.h"

RigidbodyComponent::RigidbodyComponent(GameObject& parent, int updateLayer)
    : Component(parent, updateLayer)
    , _mass(1.f) // ���ʂ��[���ɂ��Ȃ�
    , _sumOfForces(Vector3::Zero)
    , _velocity(Vector3::Zero)
    , _internalPosition(Vector3::Zero){

}

RigidbodyComponent::~RigidbodyComponent() {
    
}

void RigidbodyComponent::Update(float deltaTime) {
	UpdatePosition(deltaTime);
}

void RigidbodyComponent::FixedUpdate(float deltaTime) {

}

void RigidbodyComponent::CalculateVelocity(float deltaTime) {
   
    _internalPosition = _parent.Position();

    Vector3 totalForce = _sumOfForces;
    Vector3 acceleration = totalForce / _mass;

    _velocity += acceleration * deltaTime;

    _internalPosition += _velocity * deltaTime;

    _sumOfForces = Vector3::Zero;

}

void RigidbodyComponent::UpdatePosition(float deltaTime) {
	CalculateVelocity(deltaTime);

    _parent.Position(_internalPosition);
	//SDL_Log("Rigidbody Position : (%.2f, %.2f, %.2f)", _internalPosition.x, _internalPosition.y, _internalPosition.z);

}

void RigidbodyComponent::Move(float forward, float strafe) {
    Vector3 forardDir = _parent.GetForward();
    Vector3 rightDir = _parent.GetRight();
    _velocity = (forardDir * forward) + (rightDir * strafe);
}

void RigidbodyComponent::MoveRotate(const Vector3& axis, float angularSpeed) {

}

void RigidbodyComponent::AddForce(Vector3 force) {
    _sumOfForces += force;
}

#pragma endregion