#pragma once
#include <vector>
#include <map>
#include <fstream>
class PictureFrames
{
public:
	static void CheckIfGameFinishInit();

	//First value is the pointer to the first pointer, second is the value for the pointer for the game
	static inline std::map<int*, std::vector<int>> OriginalPointers{};
	static inline int PictureFrameIDs[512]{ 0 };
	static inline bool PictureFramesInitialized = false;
	static void SetPictureIDs();
	static void SimplePictureLoading();
	static void AdvancedPictureLoading();
	static bool HookShutdown();
};

