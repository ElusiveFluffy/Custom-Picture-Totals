#include "pch.h"
#include "PictureFrames.h"
#include "TyMemoryValues.h"
#include "TygerFrameworkAPI.hpp"

#include <string>
#include <sstream>

void PictureFrames::Shutdown() {

    API::LogPluginMessage("Deinitializing");
    API::LogPluginMessage("Resetting Picture Frame Pointers to Avoid a Crash");
    //Reset all the pointers back to their original values to avoid a crash
    for (auto [startPointer, pointerValue] : PictureFrames::OriginalPointers) {
        *startPointer = pointerValue[0];
        *(startPointer + 1) = pointerValue[1];
        *(startPointer + 2) = pointerValue[1];
    }
}

void PictureFrames::SetPictureIDs()
{
    std::string line;

    TyMemoryValues::FileExists = (TyMemoryValues::tyFileSys_Exists_t)(TyMemoryValues::TyBaseAddress + 0x1B8540);
    char fileName[] = "Custom Picture IDs.cfg";
    int fileSize;
    //Check if it exists in a rkv or PC_External. Needs to be done now because usually in plugin initialize the game hasn't initialized the rkvs yet, so it'll always say the file doesn't exist
    if (TyMemoryValues::FileExists(fileName, &fileSize) && fileSize != 0) {
        //Allocate memory manually to avoid a error with loading from PC_External, if the PC_External Loader plugin isn't installed (the game does it some other way that causes a heap error)
        TyMemoryValues::Heap_MemAlloc = (TyMemoryValues::Heap_MemAlloc_t)(TyMemoryValues::TyBaseAddress + 0x196840);
        //+ 1 for the null terminator
        char* charPictureFile = TyMemoryValues::Heap_MemAlloc(fileSize + 1);
        charPictureFile[fileSize] = NULL;
        
        TyMemoryValues::LoadFile = (TyMemoryValues::tyFileSys_Load_t)(TyMemoryValues::TyBaseAddress + 0x1B87C0);
        TyMemoryValues::LoadFile(fileName, &fileSize, charPictureFile, fileSize + 1);

        std::string pictureFile(charPictureFile);
        //No longer needed, clear out the memory. Need to use the function from the game
        TyMemoryValues::Heap_MemFree = (TyMemoryValues::Heap_MemFree_t)(TyMemoryValues::TyBaseAddress + 0x196860);
        TyMemoryValues::Heap_MemFree(charPictureFile);

        //This method reads \r which messes up the getline thing, as it doesn't remove it, so just remove it here
        std::erase(pictureFile, '\r');
        std::stringstream pictureIDs(pictureFile);
        std::getline(pictureIDs, line);

        //Move back to the first line
        pictureIDs.clear();
        pictureIDs.seekg(0, std::ios::beg);

        //Chose the method to load the IDs
        if (line != "Z1:")
            SimplePictureLoading(pictureIDs);
        else
            AdvancedPictureLoading(pictureIDs);
    }
    else {
        std::ifstream pictureIDs(API::GetPluginDirectory() / fileName);
        if (pictureIDs.is_open()) {
            std::getline(pictureIDs, line);

            //Move back to the first line
            pictureIDs.clear();
            pictureIDs.seekg(0, std::ios::beg);

            //Chose the method to load the IDs
            if (line != "Z1:")
                SimplePictureLoading(pictureIDs);
            else
                AdvancedPictureLoading(pictureIDs);

            pictureIDs.close();
        }
        else
        {
            API::LogPluginMessage("Custom Picture IDs cfg file is missing", Error);
            std::vector<TygerFrameworkImGuiParam> TygerFrameworkImguiElements = { {CollapsingHeader, "Assigned Totals Picture IDs"}, {Text, "Custom Picture IDs cfg file is missing"} };
            API::SetTygerFrameworkImGuiElements(TygerFrameworkImguiElements);
            return;
        }
    }
    
    //Add the shutdown function only if the cfg file exists
    API::AddOnTyBeginShutdown(PictureFrames::Shutdown);
}

void SetGameInfoTotalCount(int count) {
    API::LogPluginMessage("Setting Game Info Total Picture Frame Count");

    DWORD oldProtection;
    int* pictureFrameCount = (int*)(TyMemoryValues::TyBaseAddress + 0xE7733);
    //Change the memory access to ReadWrite to be able to change the hardcoded value (usually its read only)
    VirtualProtect(pictureFrameCount, 4, PAGE_EXECUTE_READWRITE, &oldProtection);

    *pictureFrameCount = count;

    //Set it back to the old access protection
    VirtualProtect(pictureFrameCount, 4, oldProtection, &oldProtection);
}

void PictureFrames::SimplePictureLoading(std::istream &pictureIDs)
{
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
            API::LogPluginMessage(level + ": 0");
            //Set everything to 0 if a level is set to have no picture frames
            *picturePointer = 0;
            *(picturePointer + 1) = 0;
            *(picturePointer + 2) = 0;
        }
        //Increment the pointer by 3 ints (C++ auto spaces it by 4 bytes)
        picturePointer = picturePointer + 3;
    }
    //Set the full game totals picture count
    SetGameInfoTotalCount(count);
    //Insert the total amount at the top, just after the collapsing header element
    TygerFrameworkImguiElements.insert(TygerFrameworkImguiElements.begin() + 1, { Text, "Total Picture Frames: " + std::to_string(count) });
    API::SetTygerFrameworkImGuiElements(TygerFrameworkImguiElements);
    API::LogPluginMessage("Total Picture Frames: " + std::to_string(count));
}

void PictureFrames::AdvancedPictureLoading(std::istream& pictureIDs)
{
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
            //Remove the level and same line elements
            TygerFrameworkImguiElements.pop_back();
            TygerFrameworkImguiElements.pop_back();
            API::LogPluginMessage(level + " 0");
            //Set everything to 0 if a level is set to have no picture frames
            *picturePointer = 0;
            *(picturePointer + 1) = 0;
            *(picturePointer + 2) = 0;
        }
        //Increment the pointer by 3 ints (C++ auto spaces it by 4 bytes)
        picturePointer = picturePointer + 3;
    }
    //Set the full game totals picture count
    SetGameInfoTotalCount(count);
    //Insert the total amount at the top, just after the collapsing header element
    TygerFrameworkImguiElements.insert(TygerFrameworkImguiElements.begin() + 1, { Text, "Total Picture Frames: " + std::to_string(count) });
    API::SetTygerFrameworkImGuiElements(TygerFrameworkImguiElements);
    API::LogPluginMessage("Total Picture Frames: " + std::to_string(count));
}
