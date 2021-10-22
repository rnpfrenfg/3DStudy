#pragma once

#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

#include <DirectXPackedVector.h>
#include <Windows.h>
#include <windowsx.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <DirectXColors.h>
#include <wrl.h>
#include <DirectXCollision.h>

namespace DX = DirectX;
namespace MS = Microsoft;
using MS::WRL::ComPtr;

#include <stdio.h>
#include <tchar.h>

#include <algorithm>
#include <array>
#include <list>
#include <string>
#include <vector>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <set>
#include <comdef.h>
#include <thread>

#define CDEBUG DEBUG|_DEBUG |1

#if CDEBUG
#define DxThrowIfFailed(x) if(FAILED(x)){std::cout<<"[Error::"<<std::hex<<x<<std::dec<<"] At " <<__FILE__<<" :: "<<__LINE__ <<'\n';while(true);}
#else
#define DxThrowIfFailed(x) if(FAILED(x)){exit(0);}
#endif