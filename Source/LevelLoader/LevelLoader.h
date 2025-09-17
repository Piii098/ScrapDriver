#pragma once
#include <rapidjson/document.h>
#include <string>
#include "Utility/Math.h"

class LevelLoader {
public:

	static bool LoadJson(const std::string& fileName, rapidjson::Document& outDoc);
	static bool LoadLevel(class Game& game, const std::string& fileName);
private:
	static void LoadGlobalProperties(class Game& game, const rapidjson::Value& inObject);
};

class JsonHelper {
public:

	static bool GetInt(const rapidjson::Value& inObject, const char* inProperty, int& outInt);
	static bool GetFloat(const rapidjson::Value& inObject, const char* inProperty, float& outFloat);
	static bool GetString(const rapidjson::Value& inObject, const char* inProperty, std::string& outString);
	static bool GetBool(const rapidjson::Value& inObject, const char* inProperty, bool& outBool);
	static bool GetVector3(const rapidjson::Value& inObject, const char* inProperty, Vector3& outVector);
	//static bool GetQuaternion(const rapidjson::Value& inObject, const char* inProperty, Quaternion& outQuaternion);

};