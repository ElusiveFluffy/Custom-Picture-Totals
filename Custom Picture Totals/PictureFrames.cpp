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

    //Run the game's shutdom function
    return Original_TyShutdown();
}

void PictureFrames::CheckIfGameFinishInit()
{
    TyMemoryValues::TyBaseAddress = (DWORD)API::Get()->param()->TyHModule;
    API::LogPluginMessage("Got Ty Base Address");

    std::vector<TygerFrameworkImGuiParam> TygerFrameworkImguiElements = { {CollapsingHeader, "Assigned Totals Picture IDs"},
                                                                          {Text, "Waiting for the Game to Initialize"} };
    API::SetTygerFrameworkImGuiElements(TygerFrameworkImguiElements);

    //Just waiting for the game to startup, values below 5 are all uses before fully initialized
    while (TyMemoryValues::GetTyGameState() < 5) {
        Sleep(100);
    }

    if (!HookShutdown())
    {
        //Return early if the hook fails
        API::LogPluginMessage("Failed to Hook the Ty Shutdown Function", Error);
        //Update the TygerFramework ImGui menu text
        TygerFrameworkImguiElements[1] = {TextWrapped, "Error Failed to Hook the Ty Shutdown Function to Avoid a Crash When Closing the Game (Check the Log for More Details)"};
        API::SetTygerFrameworkImGuiElements(TygerFrameworkImguiElements);
        return;
    }

    SetPictureIDs();
}

void PictureFrames::SetPictureIDs()
{
    //Temporarily open it here to check the first line to see what method is used
    std::ifstream pictureIDs("Plugins/Custom Picture IDs.txt");
    if (pictureIDs.is_open()) {
        std::string line;
        std::getline(pictureIDs, line);
        pictureIDs.close();

        //Chose the method to load the IDs
        if (line != "Z1:")
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

    std::vector<TygerFrameworkImGuiParam> TygerFrameworkImguiElements = { {CollapsingHeader, "Assigned Totals Picture IDs"} };

    int count = 0;
    int* picturePointer = TyMemoryValues::GetStartPicturePointer();
    //Loop through each line, splitting it at the = sign
    while (std::getline(pictureIDs, level, '=') &&
        std::getline(pictureIDs, numOfPictures)) {

        level.pop_back();
        int amount = std::stoi(numOfPictures);
        //Store the original pointers to avoid a crash when closing the game
        OriginalPointers.emplace(picturePointer, std::initializer_list<int>{ *picturePointer, * (picturePointer + 1) });
        //If its 512 set the rest to 0 picture frames
        if (amount > 0 && count != 512)
        {
            std::string idList = "";
            API::LogPluginMessage(level + ":" + numOfPictures);
            //Set the array start pointer
            *picturePointer = (int)&PictureFrameIDs[count];
            //Assign all the IDs
            for (int i = 0; i < amount; i++) {
                if (count == 511) {
                    API::LogPluginMessage("Too Many Picture Frames Assigned! Can Have a Max of 512!", Error);
                }
                idList += std::to_string(count);
                //Don't add the comma to the last one
                if (i != amount - 1)
                    idList += ",";

                PictureFrameIDs[count] = count;
                count++;
            }
            //Set the array ending pointer
            *(picturePointer + 1) = (int)&PictureFrameIDs[count];
            *(picturePointer + 2) = (int)&PictureFrameIDs[count];

            TygerFrameworkImguiElements.push_back({ Text, level + ":" });
            //Make it so that when it wraps the next line is still to the right of the level ID text
            TygerFrameworkImguiElements.push_back({ SameLine });
            TygerFrameworkImguiElements.push_back({ TextWrapped, idList });
        }
        else
        {
            TygerFrameworkImguiElements.push_back({ TextWrapped, level + ":   -" });
            API::LogPluginMessage(level + ": 0");
            //Set everything to 0 if a level is set to have no picture frames
            *picturePointer = 0;
            *(picturePointer + 1) = 0;
            *(picturePointer + 2) = 0;
        }
        //Increment the pointer by 3 ints (C++ auto spaces it by 4 bytes)
        picturePointer = picturePointer + 3;
    }
    //Insert the total amount at the top, just after the collapsing header element
    TygerFrameworkImguiElements.insert(TygerFrameworkImguiElements.begin() + 1, { Text, "Total Picture Frames: " + std::to_string(count) });
    API::SetTygerFrameworkImGuiElements(TygerFrameworkImguiElements);
    API::LogPluginMessage("Total Picture Frames: " + std::to_string(count));
}

void PictureFrames::AdvancedPictureLoading()
{
    std::ifstream pictureIDs("Plugins/Custom Picture IDs.txt");
    std::string level;
    std::string line;
    int countForLevel = 0;

    std::vector<TygerFrameworkImGuiParam> TygerFrameworkImguiElements = { {CollapsingHeader, "Assigned Totals Picture IDs"} };

    int count = 0;
    int* picturePointer = TyMemoryValues::GetStartPicturePointer();
    //Loop through all the lines
    while (std::getline(pictureIDs, line)) {
        //Check if its a level line
        if (line.find(":") != std::string::npos) {
            level = line;
            TygerFrameworkImguiElements.push_back({ Text, level });
            continue;
        }
        TygerFrameworkImguiElements.push_back({ SameLine });

        //Store the original pointers to avoid a crash when closing the game
        OriginalPointers.emplace(picturePointer, std::initializer_list<int>{ *picturePointer, * (picturePointer + 1) });
        //If its 512 set the rest to 0 picture frames
        if (line != "-" && count != 512)
        {
            TygerFrameworkImguiElements.push_back({ TextWrapped, line });

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
            TygerFrameworkImguiElements.push_back({ TextWrapped, line });
            API::LogPluginMessage(level + " 0");
            //Set everything to 0 if a level is set to have no picture frames
            *picturePointer = 0;
            *(picturePointer + 1) = 0;
            *(picturePointer + 2) = 0;
        }
        //Increment the pointer by 3 ints (C++ auto spaces it by 4 bytes)
        picturePointer = picturePointer + 3;
    }
    //Insert the total amount at the top, just after the collapsing header element
    TygerFrameworkImguiElements.insert(TygerFrameworkImguiElements.begin() + 1, {Text, "Total Picture Frames: " + std::to_string(count)});
    API::SetTygerFrameworkImGuiElements(TygerFrameworkImguiElements);
    API::LogPluginMessage("Total Picture Frames: " + std::to_string(count));
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
