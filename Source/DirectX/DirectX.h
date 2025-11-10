#pragma once

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "DirectXTex.lib")
#pragma comment(lib, "ole32.lib")      
#pragma comment(lib, "oleaut32.lib")  


#include <d3d12.h>
#include <d3dx12.h>
#include <d3dcompiler.h>
#include <dxgi1_4.h>
#include <dxgi.h>
#include <dxcapi.h>

#include <DirectXTex.h>
#include "Direct3D.h"

#include <wrl/client.h>
using Microsoft::WRL::ComPtr;
