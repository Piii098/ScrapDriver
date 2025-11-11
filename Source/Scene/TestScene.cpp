#include "Scene/TestScene.h"
#include "Core/GameObject.h"
#include "Resource/AssetManager.h"
#include "Component/ModelComponent.h"
#include "Component/TestComponent.h"
#include "Component/PlaneComponent.h"
#include "Component/RaytracingModelComponent.h"
#include "Component/CameraComponent.h"
#include "Input/InputAction.h"
#include "GameObject/CameraObject.h"

TestScene::TestScene()
{
}

TestScene::~TestScene()
{
}

bool TestScene::Initialize(const std::string& name)
{
	_name = name;
	SDL_Log("TestScene Initialize : %s", _name.c_str());
	LoadData();    // �f�[�^�̓ǂݍ���
	LoadObjects(); // �I�u�W�F�N�g�̓ǂݍ���
	return true;
}

void TestScene::LoadData()
{
	// �����ɃV�[���ŗL�̃f�[�^�ǂݍ��ݏ���������
	SDL_Log("TestScene LoadData called");
}

void TestScene::LoadObjects()
{
	//// カメラを作成（少し上から見下ろす角度）
	//_camera = CreateGameObject<GameObject>();
	//_camera->Name("MainCamera");
	//auto camera = _camera->CreateComponent<CameraComponent>();
	//camera->SetFOV(60.0f);
	//_camera->Position(0.0f, 10.0f, -4.0f); // 高い位置から
	//_camera->Rotation(Quaternion::FromAxisAngle(Vector3::Up, Math::ToRadians(-90.0f))); // 少し下向き

	_camera = CreateGameObject<CameraObject>();
	_camera->Name("MainCamera");
	auto cameraComp = _camera->CreateComponent<CameraComponent>();
	cameraComp->SetFOV(60.0f);
	_camera->Position(0.0f, 3.0f, -8.0f); // 高い位置から
	_camera->Rotation(Quaternion::FromAxisAngle(Vector3::Up, Math::ToRadians(-10.0f))); // 少し下向き

	GameObject* gameObject = nullptr;
	RaytracingModelComponent* rayModelComp = nullptr;

	// gameObject = CreateChildObject<GameObject>(_camera);
	// rayModelComp = gameObject->CreateComponent<RaytracingModelComponent>();
	// gameObject->Position(0.0f, 0.0f, 0.0f);
	// //gameObject->Rotation(Quaternion::FromAxisAngle(Vector3::UnitX, Math::ToRadians(90.0f)));
	// gameObject->Scale(30.0f, 30.0f, 1.0f);
	// rayModelComp->SetTexture("Assets/texture.jpg");

	// 床（反射する鏡のような床）
	gameObject = CreateGameObject<GameObject>();
	gameObject->Name("Floor");
	rayModelComp = gameObject->CreateComponent<RaytracingModelComponent>();
	rayModelComp->SetTexture("Assets/texture.jpg");
	gameObject->Position(0.0f, -1.0f, 0.0f);
	gameObject->Rotation(Quaternion::FromAxisAngle(Vector3::UnitX, Math::ToRadians(90.0f))); // 水平
	gameObject->Scale(100.0f, 100.0f, 1.0f); // 大きな床

	// 左の壁（鏡）
	gameObject = CreateGameObject<GameObject>();
	gameObject->Name("LeftMirror");
	rayModelComp = gameObject->CreateComponent<RaytracingModelComponent>();
	rayModelComp->SetTexture("Assets/texture.jpg");
	gameObject->Position(-25.0f, 30.0f, 0.0f);
	gameObject->Rotation(Quaternion::FromAxisAngle(Vector3::Up, Math::ToRadians(90.0f))); // 垂直に立てる
	gameObject->Scale(50.0f, 50.0f, 1.0f); // 壁サイズ

	//// 右の壁（鏡）
	//gameObject = CreateGameObject<GameObject>();
	//gameObject->Name("RightMirror");
	//rayModelComp = gameObject->CreateComponent<RaytracingModelComponent>();
	//rayModelComp->SetTexture("Assets/texture.jpg");
	//gameObject->Position(2.5f, 0.5f, 0.0f);
	//gameObject->Rotation(Quaternion::FromAxisAngle(Vector3::Up, Math::ToRadians(-90.0f))); // 垂直に立てる
	//gameObject->Scale(3.0f, 2.0f, 1.0f); // 壁サイズ

	//// 奥の壁
	//gameObject = CreateGameObject<GameObject>();
	//gameObject->Name("BackWall");
	//rayModelComp = gameObject->CreateComponent<RaytracingModelComponent>();
	//rayModelComp->SetTexture("Assets/texture.jpg");
	//gameObject->Position(0.0f, 0.5f, 2.0f);
	//gameObject->Rotation(Quaternion::FromAxisAngle(Vector3::Right, Math::ToRadians(90.0f))); // 垂直に立てる
	//gameObject->Scale(5.0f, 2.0f, 1.0f); // 壁サイズ

	//// 中央のオブジェクト1（浮遊）
	//gameObject = CreateGameObject<GameObject>();
	//gameObject->Name("CenterObject1");
	//rayModelComp = gameObject->CreateComponent<RaytracingModelComponent>();
	//rayModelComp->SetTexture("Assets/texture.jpg");
	//gameObject->Position(-0.8f, 0.3f, -0.5f);
	//gameObject->Rotation(Quaternion::FromAxisAngle(Vector3::Up, Math::ToRadians(45.0f)));
	//gameObject->Scale(0.8f, 0.8f, 0.8f);

	//// 中央のオブジェクト2（浮遊）
	//gameObject = CreateGameObject<GameObject>();
	//gameObject->Name("CenterObject2");
	//rayModelComp = gameObject->CreateComponent<RaytracingModelComponent>();
	//rayModelComp->SetTexture("Assets/texture.jpg");
	//gameObject->Position(0.8f, 0.5f, 0.2f);
	//gameObject->Rotation(Quaternion::FromAxisAngle(Vector3::Up, Math::ToRadians(-30.0f)));
	//gameObject->Scale(0.6f, 0.6f, 0.6f);

	//// 天井の小さなオブジェクト
	//gameObject = CreateGameObject<GameObject>();
	//gameObject->Name("CeilingObject");
	//rayModelComp = gameObject->CreateComponent<RaytracingModelComponent>();
	//rayModelComp->SetTexture("Assets/texture.jpg");
	//gameObject->Position(0.0f, 1.8f, 0.0f);
	//gameObject->Rotation(Quaternion::FromAxisAngle(Vector3::Right, Math::ToRadians(180.0f)));
	//gameObject->Scale(1.2f, 1.2f, 1.2f);
}

void TestScene::InputScene(const class InputAction& input)
{
	//float moveSpeed = 0.5f;
	//
	//// カメラコンポーネントを取得
	//auto camera = _camera->GetComponent<CameraComponent>();
	//if (!camera) return;
	//
	//// カメラの方向ベクトルを取得
	//Vector3 forward = camera->GetForward();
	//Vector3 right = camera->GetRight();
	//
	//// 移動方向の計算（カメラの向きに基づく）
	//if (input.IsKey(SDL_SCANCODE_UP) || input.IsKey(SDL_SCANCODE_W)) {
	//	// 前進（カメラが向いている方向）
	//	_camera->Position() += forward * moveSpeed;
	//}

	//if (input.IsKey(SDL_SCANCODE_DOWN) || input.IsKey(SDL_SCANCODE_S)) {
	//	// 後退（カメラが向いている逆方向）
	//	_camera->Position() -= forward * moveSpeed;
	//}

	//if (input.IsKey(SDL_SCANCODE_LEFT) || input.IsKey(SDL_SCANCODE_A)) {
	//	// 左移動（カメラの左方向）
	//	_camera->Position() -= right * moveSpeed;
	//}

	//if (input.IsKey(SDL_SCANCODE_RIGHT) || input.IsKey(SDL_SCANCODE_D)) {
	//	// 右移動（カメラの右方向）
	//	_camera->Position() += right * moveSpeed;
	//}

	//// Q と E キーでカメラを回転（Y軸回転）
	//if (input.IsKey(SDL_SCANCODE_Q)) {
	//	_camera->Rotation() = Quaternion::FromAxisAngle(Vector3::Up, Math::ToRadians(-2.0f)) * _camera->Rotation();
	//}

	//if (input.IsKey(SDL_SCANCODE_E)) {
	//	_camera->Rotation() = Quaternion::FromAxisAngle(Vector3::Up, Math::ToRadians(2.0f)) * _camera->Rotation();
	//}

	//if( input.IsKey(SDL_SCANCODE_R)) {
	//	_camera->Rotation() = Quaternion::FromAxisAngle(Vector3::Right, Math::ToRadians(-2.0f)) * _camera->Rotation();
	//}

	//if (input.IsKey(SDL_SCANCODE_F)) {
	//	_camera->Rotation() = Quaternion::FromAxisAngle(Vector3::Right, Math::ToRadians(2.0f)) * _camera->Rotation();
	//}

	//// R と F キーで上下移動
	//if (input.IsKey(SDL_SCANCODE_R)) {
	//	_camera->Position().y += moveSpeed; // 上昇
	//}

	//if (input.IsKey(SDL_SCANCODE_F)) {
	//	_camera->Position().y -= moveSpeed; // 下降
	//}

}

void TestScene::UpdateScene(float deltaTime)
{
	

}