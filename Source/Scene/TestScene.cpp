#include "Scene/TestScene.h"
#include "Core/GameObject.h"
#include "Resource/AssetManager.h"
#include "Component/ModelComponent.h"
#include "Component/TestComponent.h"
#include "Component/PlaneComponent.h"
#include "Component/RaytracingModelComponent.h"
#include "Component/CameraComponent.h"
#include "Component/LightComponent.h"
#include "Input/InputAction.h"
#include "GameObject/CameraObject.h"
#include "LevelLoader/LevelLoader.h"
#include "Core/Game.h"
#include "Renderer/RendererManager.h"

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
	// カメラ（CameraComponentが自動的にRendererManagerに認識される）
	_camera = CreateGameObject<CameraObject>();
	_camera->Name("MainCamera");
	
	// CameraObjectで既に作成されたCameraComponentを取得して設定
	auto cameraComp = _camera->GetComponent<CameraComponent>();
	if (cameraComp) {
		cameraComp->SetFOV(60.0f);
		cameraComp->SetAsActiveCamera(); // アクティブカメラに設定
		SDL_Log("Camera control mode: %d", static_cast<int>(cameraComp->GetControlMode()));
	}
	_camera->Position(0.0f, 3.0f, -8.0f);

	GameObject* object = nullptr;
	ModelComponent* modelComp = nullptr;
	PerMeshMaterialData material = {};

	// 床オブジェクト
	object = CreateGameObject<GameObject>();
	object->Name("Floor");
	object->Position(0.0f, -1.0f, 0.0f);
	object->Rotation(Quaternion::FromAxisAngle(Vector3::UnitX, Math::ToRadians(90.0f)));
	object->Scale(100.0f, 100.0f, 1.0f);

	modelComp = object->CreateComponent<ModelComponent>();
	modelComp->LoadQuad();
	modelComp->SetTexture("Assets/texture.jpg");

	material.baseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	material.metallic = 0.0f;
	material.roughness = 0.0f;
	material.reflectance = 1.0f;
	material.materialType = 0;
	material.uvScale = Vector2(1.0f, 1.0f);
	modelComp->SetMaterial(material);

	// 壁オブジェクト
	object = CreateGameObject<GameObject>();
	object->Name("Wall");
	object->Position(0.0f, 50.0f, 50.0f);
	object->Rotation(Quaternion::FromAxisAngle(Vector3::UnitX, Math::ToRadians(0.0f)));
	object->Scale(100.0f, 100.0f, 1.0f);
	modelComp = object->CreateComponent<ModelComponent>();
	modelComp->LoadQuad();
	modelComp->SetTexture("Assets/Metal.jpg");

	material.baseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	material.metallic = 1.0f;
	material.roughness = 0.9f;
	material.reflectance = 0.2f;
	material.materialType = 0;
	material.uvScale = Vector2(1.0f, 1.0f);
	modelComp->SetMaterial(material);

	// 壁オブジェクト
	object = CreateGameObject<GameObject>();
	object->Name("Wall");
	object->Position(0.0f, 50.0f, -50.0f);
	object->Rotation(Quaternion::FromAxisAngle(Vector3::UnitX, Math::ToRadians(0.0f)));
	object->Scale(100.0f, 100.0f, 1.0f);
	modelComp = object->CreateComponent<ModelComponent>();
	modelComp->LoadQuad();
	modelComp->SetTexture("Assets/texture.jpg");

	material.baseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	material.metallic = 1.0f;
	material.roughness = 0.9f;
	material.reflectance = 0.2f;
	material.materialType = 0;
	material.uvScale = Vector2(1.0f, 1.0f);
	modelComp->SetMaterial(material);

	//// 反射テスト用の高メタリック球体
	//GameObject* reflectiveSphere = CreateGameObject<GameObject>();
	//reflectiveSphere->Name("ReflectiveSphere");
	//reflectiveSphere->Position(2.0f, 1.0f, 2.0f);
	//reflectiveSphere->Scale(1.0f, 1.0f, 1.0f);
	//auto* sphereModel = reflectiveSphere->CreateComponent<ModelComponent>();
	//sphereModel->LoadQuad(); // 球体メッシュがない場合はQuadで代用
	//sphereModel->SetTexture("Assets/texture.jpg");
	//PerMeshMaterialData sphereMaterial = {};
	//sphereMaterial.baseColor = Vector4(0.9f, 0.9f, 1.0f, 1.0f); // 薄い青
	//sphereMaterial.metallic = 0.95f;  // 高メタリック
	//sphereMaterial.roughness = 0.05f; // 低ラフネス（鏡面）
	//sphereMaterial.reflectance = 1.0f;
	//sphereMaterial.materialType = 1; // Metal
	//sphereMaterial.uvScale = Vector2(1.0f, 1.0f);
	//sphereModel->SetMaterial(sphereMaterial);

	//// 反射確認用の追加オブジェクト
	//GameObject* coloredCube = CreateGameObject<GameObject>();
	//coloredCube->Name("ColoredCube");
	//coloredCube->Position(-2.0f, 1.0f, 3.0f);
	//coloredCube->Scale(1.5f, 1.5f, 1.5f);
	//auto* cubeModel = coloredCube->CreateComponent<ModelComponent>();
	//cubeModel->LoadQuad();
	//cubeModel->SetTexture("Assets/texture.jpg");
	//PerMeshMaterialData cubeMaterial = {};
	//cubeMaterial.baseColor = Vector4(1.0f, 0.2f, 0.2f, 1.0f); // 赤色
	//cubeMaterial.metallic = 0.1f;
	//cubeMaterial.roughness = 0.7f;
	//cubeMaterial.reflectance = 0.3f;
	//cubeMaterial.materialType = 0;
	//cubeMaterial.uvScale = Vector2(1.0f, 1.0f);
	//cubeModel->SetMaterial(cubeMaterial);

	//// ガラス材質テスト用オブジェクト
	//GameObject* glassObject = CreateGameObject<GameObject>();
	//glassObject->Name("GlassObject");
	//glassObject->Position(0.0f, 1.5f, 1.0f);
	//glassObject->Scale(1.0f, 2.0f, 1.0f);
	//auto* glassModel = glassObject->CreateComponent<ModelComponent>();
	//glassModel->LoadQuad();
	//glassModel->SetTexture("Assets/texture.jpg");
	//PerMeshMaterialData glassMaterial = {};
	//glassMaterial.baseColor = Vector4(0.9f, 1.0f, 0.9f, 0.8f); // 半透明の緑
	//glassMaterial.metallic = 0.0f;
	//glassMaterial.roughness = 0.0f; // 完全に滑らか
	//glassMaterial.reflectance = 0.9f;
	//glassMaterial.materialType = 2; // Glass
	//glassMaterial.uvScale = Vector2(1.0f, 1.0f);
	//glassModel->SetMaterial(glassMaterial);

}

void TestScene::InputScene(const class InputAction& input)
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