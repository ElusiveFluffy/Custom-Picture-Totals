// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "PictureFrames.h"
#include "TygerFrameworkAPI.hpp"
#include "TyMemoryValues.h"

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

    API::Initialize(param);

    TyMemoryValues::TyBaseAddress = (DWORD)param->TyHModule;
    if (TyMemoryValues::TyBaseAddress)
        API::LogPluginMessage("Got Ty Base Address");
    else
    {
        API::LogPluginMessage("Failed to Get Ty Base Address", Error);
        param->initErrorMessage = "Failed to Get Ty Base Address";
        return false;
    }

    //if (!fs::exists(API::GetPluginDirectory() / "Custom Picture IDs.cfg"))
    //{
    //    TyMemoryValues::FileExists = (TyMemoryValues::tyFileSys_Exists_t)(TyMemoryValues::TyBaseAddress + 0x1B8540);
    //    char fileName[] = "Custom Picture IDs.cfg";
    //    if (!TyMemoryValues::FileExists(fileName, nullptr))
    //    {
    //        API::LogPluginMessage("Custom Picture IDs cfg file is missing", Error);
    //        param->initErrorMessage = "Custom Picture IDs cfg file is missing";
    //        return false;
    //    }
    //    else
    //        TyMemoryValues::LoadFile = (TyMemoryValues::tyFileSys_Load_t)(TyMemoryValues::TyBaseAddress + 0x1B87C0);
    //}
    API::AddOnTyInitialized(PictureFrames::SetPictureIDs);

    std::vector<TygerFrameworkImGuiParam> TygerFrameworkImguiElements = { {CollapsingHeader, "Assigned Totals Picture IDs"},
                                                                          {Text, "Waiting for the Game to Initialize"} };
    API::SetTygerFrameworkImGuiElements(TygerFrameworkImguiElements);

    return true;
}

