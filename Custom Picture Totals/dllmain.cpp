// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "PictureFrames.h"
#include "TygerFrameworkAPI.hpp"
#include <filesystem>
namespace fs = std::filesystem;

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

EXTERN_C void TygerFrameworkPluginRequiredVersion(TygerFrameworkPluginVersion* version) {
    //Specifiy the version number defined in the API
    version->Major = TygerFrameworkPluginVersion_Major;
    version->Minor = TygerFrameworkPluginVersion_Minor;
    version->Patch = TygerFrameworkPluginVersion_Patch;

    version->CompatibleGames = { 1 };
}

EXTERN_C bool TygerFrameworkPluginInitialize(TygerFrameworkPluginInitializeParam* param) {

    if (!fs::exists("Plugins/Custom Picture IDs Simple.txt") && !fs::exists("Plugins/Custom Picture IDs Advanced.txt"))
    {
        param->initErrorMessage = "Custom Picture IDs txt file is missing";
        return false;
    }

    API::Initialize(param);


    CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)PictureFrames::CheckIfGameFinishInit, NULL, 0, nullptr);

    return true;
}

