#include "LevelLoader/LevelLoader.h"
#include "Core/Scene.h"
#include "Core/GameObject.h"
#include <fstream>
#include <vector>

const int LevelVersion = 1;

// ===== JsonHelper実装 ===== //

bool JsonHelper::GetInt(const rapidjson::Value& inObject, const char* inProperty, int& outInt) {
	auto iter = inObject.FindMember(inProperty);
	if (iter == inObject.MemberEnd() || !iter->value.IsInt()) {
		return false;
	}
	outInt = iter->value.GetInt();
	return true;
}

bool JsonHelper::GetFloat(const rapidjson::Value& inObject, const char* inProperty, float& outFloat) {
	auto iter = inObject.FindMember(inProperty);
	if (iter == inObject.MemberEnd()) {
		return false;
	}
	
	if (iter->value.IsDouble()) {
		outFloat = iter->value.GetDouble();
		return true;
	}
	else if (iter->value.IsDouble()) {
		outFloat = static_cast<float>(iter->value.GetDouble());
		return true;
	}
	else if (iter->value.IsInt()) {
		outFloat = static_cast<float>(iter->value.GetInt());
		return true;
	}
	
	return false;
}

bool JsonHelper::GetString(const rapidjson::Value& inObject, const char* inProperty, std::string& outString) {
	auto iter = inObject.FindMember(inProperty);
	if (iter == inObject.MemberEnd() || !iter->value.IsString()) {
		return false;
	}
	outString = iter->value.GetString();
	return true;
}

bool JsonHelper::GetBool(const rapidjson::Value& inObject, const char* inProperty, bool& outBool) {
	auto iter = inObject.FindMember(inProperty);
	if (iter == inObject.MemberEnd() || !iter->value.IsBool()) {
		return false;
	}
	outBool = iter->value.GetBool();
	return true;
}

bool JsonHelper::GetVector3(const rapidjson::Value& inObject, const char* inProperty, Vector3& outVector) {
	auto itr = inObject.FindMember(inProperty);
	if (itr == inObject.MemberEnd()) {
		SDL_Log("GetVector3: Property '%s' not found", inProperty);
		return false;
	}

	auto& property = itr->value;
	if (!property.IsArray() || property.Size() != 3) {
		SDL_Log("GetVector3: Property '%s' is not a valid 3-element array", inProperty);
		return false;
	}

	for (rapidjson::SizeType i = 0; i < 3; i++) {
		if (!property[i].IsNumber()) {
			SDL_Log("GetVector3: Element %d in property '%s' is not a number", i, inProperty);
			return false;
		}
	}

	outVector.x = static_cast<float>(property[0].GetDouble());
	outVector.y = static_cast<float>(property[1].GetDouble());
	outVector.z = static_cast<float>(property[2].GetDouble());

	return true;
}

bool JsonHelper::GetVector4(const rapidjson::Value& inObject, const char* inProperty, Vector4& outVector) {
	auto itr = inObject.FindMember(inProperty);
	if (itr == inObject.MemberEnd()) {
		return false;
	}

	auto& property = itr->value;
	if (!property.IsArray() || property.Size() != 4) {
		return false;
	}

	for (rapidjson::SizeType i = 0; i < 4; i++) {
		if (!property[i].IsNumber()) {
			return false;
		}
	}

	outVector.x = static_cast<float>(property[0].GetDouble());
	outVector.y = static_cast<float>(property[1].GetDouble());
	outVector.z = static_cast<float>(property[2].GetDouble());
	outVector.w = static_cast<float>(property[3].GetDouble());

	return true;
}

bool JsonHelper::GetQuaternion(const rapidjson::Value& inObject, const char* inProperty, Quaternion& outQuaternion) {
	auto iter = inObject.FindMember(inProperty);
	if (iter == inObject.MemberEnd() || !iter->value.IsArray() || iter->value.Size() != 4) {
		return false;
	}
	
	for (rapidjson::SizeType i = 0; i < 4; i++) {
		if (!iter->value[i].IsNumber()) {
			return false;
		}
	}
	
	outQuaternion.x = static_cast<float>(iter->value[0].GetDouble());
	outQuaternion.y = static_cast<float>(iter->value[1].GetDouble());
	outQuaternion.z = static_cast<float>(iter->value[2].GetDouble());
	outQuaternion.w = static_cast<float>(iter->value[3].GetDouble());
	
	return true;
}

// ===== LevelLoader実装 ===== //

bool LevelLoader::LoadLevel(Scene& scene, const std::string& fileName) {
	rapidjson::Document doc;
	if (!LoadJson(fileName, doc)) {
		SDL_Log("Failed to load level file: %s", fileName.c_str());
		return false;
	}

	int version = 0;
	if (!JsonHelper::GetInt(doc, "version", version) || version != LevelVersion) {
		SDL_Log("Unsupported level version: %d, expected: %d", version, LevelVersion);
		return false;
	}

	// グローバルプロパティの読み込み
	if (doc.HasMember("globalProperties") && doc["globalProperties"].IsObject()) {
		LoadGlobalProperties(scene, doc["globalProperties"]);
	}

	// ゲームオブジェクトの読み込み
	if (doc.HasMember("gameObjects") && doc["gameObjects"].IsArray()) {
		LoadGameObjects(scene, doc["gameObjects"]);
	}
    

	SDL_Log("Level loaded successfully: %s", fileName.c_str());
	return true;
}

bool LevelLoader::LoadJson(const std::string& fileName, rapidjson::Document& outDoc) {
	std::ifstream file(fileName, std::ios::in | std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		SDL_Log("Failed to open file: %s", fileName.c_str());
		return false;
	}

	std::ifstream::pos_type fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<char> bytes(static_cast<size_t>(fileSize) + 1);
	file.read(bytes.data(), static_cast<size_t>(fileSize));
	bytes[static_cast<size_t>(fileSize)] = '\0';

	outDoc.Parse(bytes.data());
	if (!outDoc.IsObject()) {
		SDL_Log("JSON parse error in file: %s", fileName.c_str());
		return false;
	}

	return true;
}

void LevelLoader::LoadGlobalProperties(Scene& scene, const rapidjson::Value& inObject) {
	// シーン名の読み込み
	std::string sceneName;
	if (JsonHelper::GetString(inObject, "sceneName", sceneName)) {
		SDL_Log("Scene name: %s", sceneName.c_str());
	}

	// カメラ設定などの読み込み（必要に応じて実装）
	// 例: ambientLight, backgroundColor など
}

void LevelLoader::LoadGameObjects(Scene& scene, const rapidjson::Value& inArray) {
	if (!inArray.IsArray()) {
		return;
	}

	for (rapidjson::SizeType i = 0; i < inArray.Size(); i++) {
		LoadGameObject(scene, inArray[i], nullptr);
	}
}

GameObject* LevelLoader::LoadGameObject(Scene& scene, const rapidjson::Value& inObject, GameObject* parent) {
	if (!inObject.IsObject()) {
		return nullptr;
	}

	// ゲームオブジェクトの生成
	GameObject* obj = nullptr;
	if (parent) {
		obj = scene.CreateChildObject<GameObject>(parent);
	}
	else {
		obj = scene.CreateGameObject<GameObject>();
	}

	if (!obj) {
		SDL_Log("Failed to create game object");
		return nullptr;
	}

	// 名前の読み込み
	std::string name;
	if (JsonHelper::GetString(inObject, "name", name)) {
		obj->Name(name);
		SDL_Log("Loading GameObject: %s", name.c_str());
	}

	// トランスフォームの読み込み
	Vector3 position;
	if (JsonHelper::GetVector3(inObject, "position", position)) {
		obj->Position(position);
		SDL_Log("  Position: [%f, %f, %f]", position.x, position.y, position.z);
	}

	Quaternion rotation;
	if (JsonHelper::GetQuaternion(inObject, "rotation", rotation)) {
		obj->Rotation(rotation);
		SDL_Log("  Rotation: [%f, %f, %f, %f]", rotation.x, rotation.y, rotation.z, rotation.w);
	}

	Vector3 scale;
	if (JsonHelper::GetVector3(inObject, "scale", scale)) {
		obj->Scale(scale);
		SDL_Log("  Scale: [%f, %f, %f]", scale.x, scale.y, scale.z);
	}

	// GameObject独自のプロパティ読み込み
	obj->LoadProperties(inObject);

	// 子オブジェクトの読み込み
	if (inObject.HasMember("children") && inObject["children"].IsArray()) {
		const rapidjson::Value& children = inObject["children"];
		for (rapidjson::SizeType i = 0; i < children.Size(); i++) {
			LoadGameObject(scene, children[i], obj);
		}
	}

	return obj;
}
