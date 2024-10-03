#pragma once
#include <vector>
#include <map>
#include <string>
#include <functional>
class TyMemoryValues
{
public:
	static inline DWORD TyBaseAddress;
	static int GetTyGameState() { return *(int*)(TyBaseAddress + 0x288A6C); };
	static LPVOID* GetTyShutdownFunc() { return (LPVOID*)(TyBaseAddress + 0x19DAB0); };

	static int* GetStartPicturePointer() { return (int*)(TyBaseAddress + 0x288610); };
};

