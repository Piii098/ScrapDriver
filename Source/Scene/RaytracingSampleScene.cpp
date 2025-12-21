#include "Scene/RaytracingSampleScene.h"
#include "Core/GameObject.h"
#include "Component/ModelComponent.h"
#include "Component/CameraComponent.h"
#include "Input/InputAction.h"
#include "GameObject/CameraObject.h"
#include "Core/Game.h"
#include "Renderer/RendererManager.h"

#include <cmath>

namespace {
	Vector3 LerpVector3(const Vector3& a, const Vector3& b, float t)
	{
		return Vector3(XMVectorLerp(XMVECTOR(a), XMVECTOR(b), t));
	}

	Quaternion LookAtRotationLH(const Vector3& from, const Vector3& to)
	{
		Vector3 forward = Vector3(XMVectorSubtract(XMVECTOR(to), XMVECTOR(from)));
		float lenSq = forward.x * forward.x + forward.y * forward.y + forward.z * forward.z;
		if (lenSq < 1e-6f) {
			return Quaternion::Identity;
		}
		forward.Normalize();

		float yaw = atan2f(forward.x, forward.z);
		float xzLen = sqrtf(forward.x * forward.x + forward.z * forward.z);
		float pitch = atan2f(-forward.y, xzLen);

		return Quaternion::FromEuler(pitch, yaw, 0.0f);
	}
} // namespace

RaytracingSampleScene::RaytracingSampleScene() : _camera(nullptr)
{
}

RaytracingSampleScene::~RaytracingSampleScene()
{
}

bool RaytracingSampleScene::Initialize(const std::string& name)
{
	_name = name;
	SDL_Log("RaytracingSampleScene Initialize : %s", _name.c_str());
	LoadData();
	LoadObjects();
	return true;
}

void RaytracingSampleScene::LoadData()
{
	SDL_Log("RaytracingSampleScene LoadData called");
}

static PerMeshMaterialData MakeMaterial(const Vector4& baseColor, float metallic, float roughness, float reflectance)
{
	PerMeshMaterialData material = {};
	material.baseColor = baseColor;
	material.metallic = metallic;
	material.roughness = roughness;
	material.reflectance = reflectance;
	material.materialType = 0;
	material.uvScale = Vector2(1.0f, 1.0f);
	material.uvOffset = Vector2(0.0f, 0.0f);
	material.textureIndex = static_cast<unsigned int>(-1);
	return material;
}

void RaytracingSampleScene::LoadObjects()
{
	// カメラ
	_camera = CreateGameObject<CameraObject>();
	_camera->Name("MainCamera");

	auto cameraComp = _camera->GetComponent<CameraComponent>();
	if (cameraComp) {
		cameraComp->SetFOV(60.0f);
		cameraComp->SetControlMode(CameraComponent::ControlMode::NONE);
		cameraComp->SetAsActiveCamera();
		SDL_Log("Camera control mode: %d", static_cast<int>(cameraComp->GetControlMode()));
	}
	_camera->Position(0.0f, 80.0f, -320.0f);

	// カメラ巡回（ウェイポイント）
	_cameraTourPoints = {
		Vector3(0.0f, 80.0f, -320.0f),
		Vector3(0.0f, 80.0f, -320.0f),
		Vector3(220.0f, 95.0f, -260.0f),
		Vector3(220.0f, 95.0f, -260.0f),
		Vector3(220.0f, 95.0f, 200.0f),
		Vector3(220.0f, 95.0f, 200.0f),
		Vector3(-220.0f, 95.0f, -260.0f),
		Vector3(-220.0f, 95.0f, -260.0f),
		Vector3(-220.0f, 95.0f, 200.0f),
		Vector3(-220.0f, 95.0f, 200.0f),
	};
	_cameraTourLookAt = Vector3(0.0f, 60.0f, 50.0f);
	_cameraTourSegmentDurationSec = 50.0f;
	_cameraTourSegmentTimeSec = 1.0f;
	_cameraTourIndex = 0;

	GameObject* object = nullptr;
	ModelComponent* modelComp = nullptr;

	// ===== 簡易Cornell Box（反射が見える背景） =====
	// Quad のデフォルト法線は -Z。
	// -90度(X)で +Y（床の上向き）にする。

	// 床
	object = CreateGameObject<GameObject>();
	object->Name("Floor");
	object->Position(0.0f, 0.0f, 0.0f);
	object->Rotation(Quaternion::FromAxisAngle(Vector3::UnitX, Math::ToRadians(-90.0f)));
	object->Scale(500.0f, 500.0f, 1.0f);
	modelComp = object->CreateComponent<ModelComponent>();
	modelComp->LoadQuad();
	modelComp->SetMaterial(MakeMaterial(Vector4(0.75f, 0.75f, 0.75f, 1.0f), 0.0f, 1.0f, 0.02f));

	// 天井
	object = CreateGameObject<GameObject>();
	object->Name("Ceiling");
	object->Position(0.0f, 200.0f, 0.0f);
	object->Rotation(Quaternion::FromAxisAngle(Vector3::UnitX, Math::ToRadians(90.0f)));
	object->Scale(500.0f, 500.0f, 1.0f);
	modelComp = object->CreateComponent<ModelComponent>();
	modelComp->LoadQuad();
	modelComp->SetMaterial(MakeMaterial(Vector4(0.70f, 0.70f, 0.70f, 1.0f), 0.0f, 1.0f, 0.02f));

	// 背面壁（カメラの前方）
	object = CreateGameObject<GameObject>();
	object->Name("BackWall");
	object->Position(0.0f, 100.0f, 250.0f);
	object->Scale(500.0f, 200.0f, 1.0f);
	modelComp = object->CreateComponent<ModelComponent>();
	modelComp->LoadQuad();
	modelComp->SetMaterial(MakeMaterial(Vector4(0.65f, 0.70f, 0.95f, 1.0f), 0.0f, 1.0f, 0.02f));

	// 左壁（赤）: Y -90 で法線 +X
	object = CreateGameObject<GameObject>();
	object->Name("LeftWall");
	object->Position(-250.0f, 100.0f, 0.0f);
	object->Rotation(Quaternion::FromAxisAngle(Vector3::UnitY, Math::ToRadians(-90.0f)));
	object->Scale(500.0f, 200.0f, 1.0f);
	modelComp = object->CreateComponent<ModelComponent>();
	modelComp->LoadQuad();
	modelComp->SetMaterial(MakeMaterial(Vector4(0.95f, 0.35f, 0.35f, 1.0f), 0.0f, 1.0f, 0.02f));

	// 右壁（緑）: Y +90 で法線 -X
	object = CreateGameObject<GameObject>();
	object->Name("RightWall");
	object->Position(250.0f, 100.0f, 0.0f);
	object->Rotation(Quaternion::FromAxisAngle(Vector3::UnitY, Math::ToRadians(90.0f)));
	object->Scale(500.0f, 200.0f, 1.0f);
	modelComp = object->CreateComponent<ModelComponent>();
	modelComp->LoadQuad();
	modelComp->SetMaterial(MakeMaterial(Vector4(0.35f, 0.95f, 0.45f, 1.0f), 0.0f, 1.0f, 0.02f));

	// ===== 反射/粗さの比較オブジェクト =====
	// ミラー球
	object = CreateGameObject<GameObject>();
	object->Name("MirrorSphere");
	object->Position(-120.0f, 60.0f, 50.0f);
	object->Scale(40.0f, 40.0f, 40.0f);
	modelComp = object->CreateComponent<ModelComponent>();
	modelComp->LoadSphere(1.0f, 32, 24);
	modelComp->SetMaterial(MakeMaterial(Vector4(0.55f, 0.56f, 0.58f, 1.0f), 1.0f, 0.25f, 0.9f));

	// ラフ金属球
	object = CreateGameObject<GameObject>();
	object->Name("RoughMetalSphere");
	object->Position(0.0f, 60.0f, 50.0f);
	object->Scale(40.0f, 40.0f, 40.0f);
	modelComp = object->CreateComponent<ModelComponent>();
	modelComp->LoadSphere(1.0f, 32, 24);
	modelComp->SetMaterial(MakeMaterial(Vector4(0.9f, 0.9f, 0.9f, 1.0f), 1.0f, 1.0f, 0.1f));

	// 非金属（プラスチック寄り）
	object = CreateGameObject<GameObject>();
	object->Name("PlasticSphere");
	object->Position(120.0f, 60.0f, 50.0f);
	object->Scale(40.0f, 40.0f, 40.0f);
	modelComp = object->CreateComponent<ModelComponent>();
	modelComp->LoadSphere(1.0f, 32, 24);
	modelComp->SetMaterial(MakeMaterial(Vector4(0.90f, 0.90f, 0.95f, 1.0f), 0.0f, 0.25f, 0.15f));

	// グロッシーキューブ
	object = CreateGameObject<GameObject>();
	object->Name("GlossyCube");
	object->Position(120.0f, 35.0f, -90.0f);
	object->Rotation(Quaternion::FromAxisAngle(Vector3::UnitY, Math::ToRadians(20.0f)));
	object->Scale(70.0f, 70.0f, 70.0f);
	modelComp = object->CreateComponent<ModelComponent>();
	modelComp->LoadCube(1.0f);
	modelComp->SetMaterial(MakeMaterial(Vector4(0.25f, 0.50f, 1.0f, 1.0f), 0.0f, 1.0f, 0.0f));


	// グロッシーキューブ
	object = CreateGameObject<GameObject>();
	object->Name("MetalCube");
	object->Position(-120.0f, 35.0f, -90.0f);
	object->Rotation(Quaternion::FromAxisAngle(Vector3::UnitY, Math::ToRadians(20.0f)));
	object->Scale(70.0f, 70.0f, 70.0f);
	modelComp = object->CreateComponent<ModelComponent>();
	modelComp->LoadCube(1.0f);
	modelComp->SetMaterial(MakeMaterial(Vector4(0.25f, 0.50f, 1.0f, 1.0f), 1.0f, 0.10f, 1.0f));


}

void RaytracingSampleScene::InputScene(const class InputAction& input)
{
	if (input.IsKeyDown(SDL_SCANCODE_1)) {
		RenderingMode currentMode = GAME.GetRendererManager().GetRenderingMode();
		bool isRaytracing = (currentMode == RenderingMode::RAYTRACING);
		if (isRaytracing) {
			GAME.GetRendererManager().SetRenderingMode(RenderingMode::RASTERIZATION);
			SDL_Log("Switched to RASTERIZATION mode");
		}
		else {
			GAME.GetRendererManager().SetRenderingMode(RenderingMode::RAYTRACING);
			SDL_Log("Switched to RAYTRACING mode");
		}
	}
}

void RaytracingSampleScene::UpdateScene(float deltaTime)
{
	if (!_cameraTourEnabled || _camera == nullptr) {
		return;
	}
	if (_cameraTourPoints.size() < 2 || _cameraTourSegmentDurationSec <= 0.0f) {
		return;
	}

	_cameraTourSegmentTimeSec += deltaTime;
	while (_cameraTourSegmentTimeSec >= _cameraTourSegmentDurationSec) {
		_cameraTourSegmentTimeSec -= _cameraTourSegmentDurationSec;
		_cameraTourIndex = (_cameraTourIndex + 1) % static_cast<int>(_cameraTourPoints.size());
	}

	float t = _cameraTourSegmentTimeSec / _cameraTourSegmentDurationSec;
	const int nextIndex = (_cameraTourIndex + 1) % static_cast<int>(_cameraTourPoints.size());
	const Vector3 pos = LerpVector3(_cameraTourPoints[_cameraTourIndex], _cameraTourPoints[nextIndex], t);
	const Quaternion rot = LookAtRotationLH(pos, _cameraTourLookAt);

	_camera->Position(pos);
	_camera->Rotation(rot);

}
