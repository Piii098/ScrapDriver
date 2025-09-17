#pragma once

// C++17非推奨警告の抑制（RapidJSON対応）
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

#include <string>
#include <vector>
#include <memory>
#include <locale.h>
#include <SDL3/SDL.h>
#include "Utility/Math.h"

// 前方宣言
class GameObject;
class Component;
class Scene;
class SceneManager;
