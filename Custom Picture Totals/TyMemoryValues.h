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

	typedef bool (*tyFileSys_Exists_t)(char* pFilename, int* pOutLen);
	typedef char* (*tyFileSys_Load_t) (char* fileName, int* pOutLen, char* pMemoryAllocated, int spaceAllocated);
	typedef char* (*Heap_MemAlloc_t) (int size);
	typedef void (*Heap_MemFree_t) (void* ptr);
	static inline tyFileSys_Exists_t FileExists;
	static inline tyFileSys_Load_t LoadFile;
	static inline Heap_MemAlloc_t Heap_MemAlloc;
	static inline Heap_MemFree_t Heap_MemFree;
};

