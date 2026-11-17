#pragma once
#include <vector>
#include "Core/Component.h"
#include "Utility/Math.h"

class RigidbodyComponent : public Component {
public:

    RigidbodyComponent(GameObject& parent, int updateLayer = 30);
    ~RigidbodyComponent() override;

    void Update(float deltaTime) override;  // 新しくUpdate関数を追加
    void FixedUpdate(float deltaTime) override;
    void CalculateVelocity(float deltaTime);
    void UpdatePosition(float deltaTime);

    void Move(float forward, float strafe);
    void MoveRotate(const Vector3& axis, float angularSpeed);

    void AddForce(Vector3 force);

private:

	Vector3 _velocity;
	//Vector3 _acceleration;
	Vector3 _internalPosition; 
    float _mass;
	Vector3 _sumOfForces;
 
    bool _isGravity;
    bool _onGround; 
};