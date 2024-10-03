#include "pch.h"
#include "PictureFrames.h"
#include "TyMemoryValues.h"
#include "TygerFrameworkAPI.hpp"
#include "MinHook.h"

#include <string>
#include <sstream>

typedef int32_t(WINAPI* TyShutdown_t) ();
TyShutdown_t Original_TyShutdown;

int32_t WINAPI ShutDown() {

    API::LogPluginMessage("Deinitializing");
    API::LogPluginMessage("Resetting Picture Frame Pointers to Avoid a Crash");
    //Reset all the pointers back to their original values to avoid a crash
    for (auto [startPointer, pointerValue] : PictureFrames::OriginalPointers) {
        *startPointer = pointerValue[0];
        *(startPointer + 1) = pointerValue[1];
        *(startPointer + 2) = pointerValue[1];
    }

    return Original_TyShutdown();
}

void PictureFrames::CheckIfGameFinishInit()
{
    TyMemoryValues::TyBaseAddress = (DWORD)API::Get()->param()->TyHModule;
    API::LogPluginMessage("Got Ty Base Address");
    //Just waiting for the game to startup, values below 5 are all uses before fully initialized
    while (TyMemoryValues::GetTyGameState() < 5) {
        Sleep(100);
    }

    if (!HookShutdown())
    {
        API::LogPluginMessage("Failed to Hook the Ty Shutdown Function", Error);
        return;
    }

    SetPictureIDs();
}

void PictureFrames::SetPictureIDs()
{
    std::ifstream pictureIDs("Plugins/Custom Picture IDs.txt");

    if (pictureIDs.is_open()) {
        std::string line;
        std::getline(pictureIDs, line);
        pictureIDs.close();

        if (line.find("Z1:") == std::string::npos)
            SimplePictureLoading();
        else
            AdvancedPictureLoading();
    }
}

void PictureFrames::SimplePictureLoading()
{
    std::ifstream pictureIDs("Plugins/Custom Picture IDs.txt");
    std::string level;
    std::string numOfPictures;

    int count = 0;
    int* picturePointer = TyMemoryValues::GetStartPicturePointer();
    while (std::getline(pictureIDs, level, '=') &&
        std::getline(pictureIDs, numOfPictures)) {

        level.pop_back();
        int amount = std::stoi(numOfPictures);
        //Store the original pointers to avoid a crash when closing the game
        OriginalPointers.emplace(picturePointer, std::initializer_list<int>{ *picturePointer, * (picturePointer + 1) });
        //If its 512 set the rest to 0 picture frames
        if (amount > 0 && count != 512)
        {
            API::LogPluginMessage(level + ":" + numOfPictures);
            //Set the array start pointer
            *picturePointer = (int)&PictureFrameIDs[count];
            //Assign all the IDs
            for (int i = 0; i < amount; i++) {
                if (count == 511) {
                    API::LogPluginMessage("Too Many Picture Frames Assigned! Can Have a Max of 512!", Error);
                }

                PictureFrameIDs[count] = count;
                count++;
            }
            //Set the array ending pointer
            *(picturePointer + 1) = (int)&PictureFrameIDs[count];
            *(picturePointer + 2) = (int)&PictureFrameIDs[count];
        }
        else
        {
            API::LogPluginMessage(level + ": 0");
            //Set everything to 0 if a level is set to have no picture frames
            *picturePointer = 0;
            *(picturePointer + 1) = 0;
            *(picturePointer + 2) = 0;
        }
        //Increment the pointer by 3 ints (C++ auto spaces it by 4 bytes)
        picturePointer = picturePointer + 3;
    }
    API::LogPluginMessage("Total Amount of Pictures: " + std::to_string(count));
}

void PictureFrames::AdvancedPictureLoading()
{
    std::ifstream pictureIDs("Plugins/Custom Picture IDs.txt");
    std::string level;
    std::string line;
    int countForLevel = 0;

    int count = 0;
    int* picturePointer = TyMemoryValues::GetStartPicturePointer();
    while (std::getline(pictureIDs, line)) {
        //Check if its a level line
        if (line.find(":") != std::string::npos) {
            level = line;
            continue;
        }

        //Store the original pointers to avoid a crash when closing the game
        OriginalPointers.emplace(picturePointer, std::initializer_list<int>{ *picturePointer, * (picturePointer + 1) });
        //If its 512 set the rest to 0 picture frames
        if (line != "-" && count != 512)
        {
            //Set the array start pointer
            *picturePointer = (int)&PictureFrameIDs[count];

            std::stringstream IDStream(line);
            std::string ID;
            //Loop through all the IDs and assign them
            while (std::getline(IDStream, ID, ',') && count != 512)
            {
                int idInt = std::stoi(ID);
                if (count == 511) {
                    API::LogPluginMessage("Too Many Picture Frames Assigned! Can Have a Max of 512!", Error);
                }

                PictureFrameIDs[count] = idInt;
                count++;
                countForLevel++;
            }
            //Set the array ending pointer
            *(picturePointer + 1) = (int)&PictureFrameIDs[count];
            *(picturePointer + 2) = (int)&PictureFrameIDs[count];

            API::LogPluginMessage(level + " " + std::to_string(countForLevel));
            countForLevel = 0;
        }
        else
        {
            API::LogPluginMessage(level + " 0");
            //Set everything to 0 if a level is set to have no picture frames
            *picturePointer = 0;
            *(picturePointer + 1) = 0;
            *(picturePointer + 2) = 0;
        }
        //Increment the pointer by 3 ints (C++ auto spaces it by 4 bytes)
        picturePointer = picturePointer + 3;
    }
    API::LogPluginMessage("Total Amount of Pictures: " + std::to_string(count));
}

bool PictureFrames::HookShutdown()
{
    MH_STATUS minhookStatus = MH_Initialize();
    if (minhookStatus != MH_OK) {
        std::string error = MH_StatusToString(minhookStatus);
        API::LogPluginMessage("Failed to Initialize Minhook, With the Error: " + error, Error);
        return false;
    }

    //Hook Ty Shutdown Function
    MH_STATUS minHookStatus = MH_CreateHook(TyMemoryValues::GetTyShutdownFunc(), &ShutDown, reinterpret_cast<LPVOID*>(&Original_TyShutdown));
    if (minHookStatus != MH_OK) {
        std::string error = MH_StatusToString(minHookStatus);
        API::LogPluginMessage("Failed to Create the Ty Shutdown Function Hook, With the Error: " + error, Error);
        return false;
    }

    //Enable both hooks
    minHookStatus = MH_EnableHook(MH_ALL_HOOKS);
    if (minHookStatus != MH_OK) {
        std::string error = MH_StatusToString(minHookStatus);
        API::LogPluginMessage("Failed to Hook Ty Shutdown Function, With the Error: " + error, Error);
        return false;
    }

    API::LogPluginMessage("Sucessfully Hooked the Ty Shutdown Function");
    return true;
}
