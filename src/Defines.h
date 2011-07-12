#pragma once

// Namespace usings
using namespace std;
#include <map>
#include <vector>

#include "DXUT.h"
#include "SDKmisc.h"

// XNA Math and collision
#include <xnamath.h>
#include "xnaCollision.h"

// Some utility functions
#include "Utility.h"

// Static Lib Imports
#pragma comment(lib, "dxerr.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "D3D9.lib")
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "D3DX10.lib")
#pragma comment(lib, "D3DX11.lib")
#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "gdiplus.lib")

#ifdef _DEBUG
#pragma comment(lib, "comsuppwd.lib")
#pragma comment(lib, "d3dx9d.lib")
#else
#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "d3dx9.lib")
#endif

// Warning disables
#pragma warning(disable: 4324) // Padding added from aligned members