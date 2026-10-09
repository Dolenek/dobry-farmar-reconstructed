#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
// MSVC compatibility for the unmodified Wine-origin MinGW declarations.
#ifndef __MSABI_LONG
#define __MSABI_LONG(value) value##L
#endif
typedef BOOL WINBOOL;
#include <d3d8.h>
static_assert(sizeof(D3DPRESENT_PARAMETERS)==52,"Original 13-DWORD parameters");
static_assert(sizeof(D3DSURFACE_DESC)==32,"Original texture level description");
static_assert(sizeof(D3DLOCKED_RECT)==8,"Original texture pitch and pointer");
