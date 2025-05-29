#include "DuplicateManager.h"
#include "InputHandler.h"
#include "Utilities.h" 

#include <iostream>
#include <filesystem>
#include <map>
#include <vector>
#include <string>



void handleDuplicateRemoval(const std::map<std::string, std::vector<fs::path>>& duplicateGroups)
{
    std::wcout << L"\n=== DUPLICATE REMOVAL OPTIONS ===" << std::endl;
    std::wcout << L"Would you like to remove duplicate files?" << std::endl;
    std::wcout << L"[1] Keep all files" << std::endl;
    std::wcout << L"[2] Interactive removal" << std::endl;
    std::wcout << L"[3] Automatic removal (keeps the file with the shortest path)" << std::endl;

    int choice = getUserChoiceRange(L"Pleas enter your choice (1-3): ", 1, 3);

    switch (choice)
    {
    case 1:
        std::wcout << L"Keeping all files. No duplicates will be removed." << std::endl;
        break;
    case 2:
        interactiveRemoval(duplicateGroups);
        break;
    case 3:
        automaticRemoval(duplicateGroups);
        break;
    }
}

void interactiveRemoval(const std::map<std::string, std::vector<fs::path>>& duplicateGroups)
{
	std::wcout << L"\n=== INTERACTIVE DUPLICATE REMOVAL ===" << std::endl;
    std::wcout << L"For each duplicate group, you can choose which files to keep/delete." << std::endl;
    std::wcout << L"Files will be moved to recycle bin for safety.\n" << std::endl;

    size_t groupNumber = 1;
    size_t totalDeleted = 0;
    uintmax_t totalSizeDeleted = 0;

    for (const auto& [hash, files] : duplicateGroups)
    {
        if (files.size() <= 1) continue;

        std::wcout << L"--- Duplicate Group #" << groupNumber << L" ---" << std::endl;

        uintmax_t fileSize = 0;
        try
        {
            fileSize = fs::file_size(files[0]);
            std::string fileSizeStr = formatFileSize(fileSize);
            std::wcout << L"File size: " << std::wstring(fileSizeStr.begin(), fileSizeStr.end()) << std::endl;
        }
        catch (const fs::filesystem_error& e)
        {
            std::wcerr << L"Error getting file size: " << e.what() << std::endl;
        }

        for (size_t i = 0; i < files.size(); ++i)
        {
            std::wcout << L"  " << (i + 1) << L". " << files[i].wstring() << std::endl;
        }

        std::wcout << L"\nOptions:" << std::endl;
        std::wcout << L"[0] Keep all files" << std::endl;
        std::wcout << L"[1-" << files.size() << L"] Keep only the selected file (delete all others)" << std::endl;
        std::wcout << L"Press enter or enter -1 to auto-select (keep shortest path)" << std::endl;

        int choice = getUserChoiceRange(L"Please enter your choice: ", 1, static_cast<int>(files.size()), true);

        if (choice == 0)
        {
            std::wcout << L"Keeping all files in this group.\n" << std::endl;
            groupNumber++;
            continue;
        }

        fs::path fileToKeep;
        if (choice == -1)
        {
            fileToKeep = selectBestFileToKeep(files);
            std::wcout << L"Auto-selected: " << fileToKeep.wstring() << std::endl;
        }
        else
        {
            fileToKeep = files[choice - 1];
            std::wcout << L"Keeping file: " << fileToKeep.wstring() << std::endl;
        }

        // Collect files to delete
		std::vector<fs::path> filesToDelete;
        for (const auto& file : files)
        {
            if (file != fileToKeep)
            {
				filesToDelete.push_back(file);
            }
        }

		std::wcout << L"\nFiles to be moved to Recycle Bin:" << std::endl;
        for (const auto& file : filesToDelete)
        {
            std::wcout << L"  " << file.wstring() << std::endl;
        }
	

        if (getUserConfirmation(L"Are you sure you want to continue? "))
        {
			size_t deletedCount = 0;
            for (const auto& file : filesToDelete)
            {
                if (safeDeleteFile(file, true)) // Moves it to recycle bin
                {
                    deletedCount++;
                    totalSizeDeleted += fileSize;
                }
			}
            totalDeleted += deletedCount;
			std::wcout << L"Successfully moved " << deletedCount << L" files to Recycle Bin." << std::endl;
        }
        else
        {
			std::wcout << L"Skipping deletion for this group." << std::endl;
        }

        std::wcout << std::endl;
		groupNumber++;
    }

    std::wcout << L"=== REMOVAL SUMMARY ===" << std::endl;
	std::wcout << L"Total files moved to Recycle Bin: " << totalDeleted << std::endl;
    if (totalSizeDeleted > 0)
    {
        std::string spaceFreedStr = formatFileSize(totalSizeDeleted);
        std::wcout << L"Total space freed: " << std::wstring(spaceFreedStr.begin(), spaceFreedStr.end()) << std::endl;
    }
}

void automaticRemoval(const std::map<std::string, std::vector<fs::path>>& duplicateGroups)
{
	// TODO: Implement automatic removal logic

}

fs::path selectBestFileToKeep(const std::vector<fs::path>& files)
{
	// TODO: Implement logic to select the best file to keep based on shortest path or other criteria
    return fs::path();
}

bool safeDeleteFile(const fs::path& filePath, bool useRecycleBin)
{
	// TODO: Implement safe deletion logic

    /*if (!fs::exists(file))
    {
        std::wcerr << L"Warning: File does not exist: " << file.wstring() << std::endl;
        continue;
    }*/

    return false;
}
