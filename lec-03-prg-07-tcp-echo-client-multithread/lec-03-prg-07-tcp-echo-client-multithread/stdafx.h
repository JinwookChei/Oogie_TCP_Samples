#pragma once

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#endif  // _DEBUG

#include <iostream>
#include <stdio.h>
#include <stdlib.h>



#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <Windows.h>
#include <type_traits>
#include <functional>

#pragma comment(lib, "ws2_32.lib")


#ifdef _DEBUG
#define DEBUG_BREAK() __debugbreak()
#else
#define DEBUG_BREAK() ((void)0)
#endif

