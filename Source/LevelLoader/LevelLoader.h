#pragma once
#include <rapidjson/document.h>
#include <string>
#include "Utility/Math.h"

/// @brief JSON読み込み用ヘルパークラス
class JsonHelper {
public:
	/// @brief int値の取得
	static bool GetInt(const rapidjson::Value& inObject, const char* inProperty, int& outInt);
	
	/// @brief float値の取得
	static bool GetFloat(const rapidjson::Value& inObject, const char* inProperty, float& outFloat);
	
	/// @brief string値の取得
	static bool GetString(const rapidjson::Value& inObject, const char* inProperty, std::string& outString);
	
	/// @brief bool値の取得
	static bool GetBool(const rapidjson::Value& inObject, const char* inProperty, bool& outBool);
	
	/// @brief Vector3の取得
	static bool GetVector3(const rapidjson::Value& inObject, const char* inProperty, Vector3& outVector);
	
	/// @brief Vector4の取得
	static bool GetVector4(const rapidjson::Value& inObject, const char* inProperty, Vector4& outVector);
	
	/// @brief Quaternionの取得
	static bool GetQuaternion(const rapidjson::Value& inObject, const char* inProperty, Quaternion& outQuaternion);
};

/// @brief レベルデータの読み込みを行うクラス
class LevelLoader {
public:
	/// @brief レベルファイルの読み込み
	/// @param scene 読み込み先のシーン
	/// @param fileName JSONファイルのパス
	/// @return 成功時true
	static bool LoadLevel(class Scene& scene, const std::string& fileName);

private:
	/// @brief JSONファイルの読み込み
	static bool LoadJson(const std::string& fileName, rapidjson::Document& outDoc);
	
	/// @brief グローバルプロパティの読み込み
	static void LoadGlobalProperties(class Scene& scene, const rapidjson::Value& inObject);
	
	/// @brief ゲームオブジェクトの読み込み
	static void LoadGameObjects(class Scene& scene, const rapidjson::Value& inObject);
	
	/// @brief 単一のゲームオブジェクトの読み込み
	static class GameObject* LoadGameObject(class Scene& scene, const rapidjson::Value& inObject, class GameObject* parent = nullptr);
};
