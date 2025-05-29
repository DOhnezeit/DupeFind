#include "DuplicateManager.h"
#include "InputHandler.h"

#include <iostream>


void handleDuplicateRemoval(const std::map<std::string, std::vector<fs::path>>& duplicateGroups)
{
    std::wcout << L"\n=== DUPLICATE REMOVAL OPTIONS ===" << std::endl;
    std::wcout << L"Would you like to remove duplicate files?" << std::endl;
    std::wcout << L"[1] Keep all files" << std::endl;
    std::wcout << L"[2] Interactive removal" << std::endl;
    std::wcout << L"[3] Automatic removal (keeps the file with the shortest path)" << std::endl;

    int choice = getUserChoice(L"Pleas enter your choice (1-3): ", 1, 3);

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
	// TODO: Implement interactive removal logic

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
    return false;
}
