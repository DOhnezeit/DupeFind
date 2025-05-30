#include "DuplicateManager.h"
#include "InputHandler.h"
#include "Utilities.h" 
#include "ReportGenerator.h"

#include <iostream>
#include <filesystem>
#include <map>
#include <vector>
#include <string>

#define WiN32_LEAN_AND_MEAN
#include <Windows.h>
#include <ShellAPI.h>



void handleDuplicateRemoval(const std::map<std::string, std::vector<fs::path>>& duplicateGroups)
{
    std::wcout << L"\n=== DUPLICATE REMOVAL OPTIONS ===" << std::endl;
    std::wcout << L"Would you like to remove duplicate files?" << std::endl;
    std::wcout << L"[1] Keep all files" << std::endl;
    std::wcout << L"[2] Interactive removal" << std::endl;
    std::wcout << L"[3] Automatic removal (keeps the file with the shortest path)" << std::endl;

    int choice = getUserChoiceRange(L"Please enter your choice (1-3): ", 1, 3);

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

    // Collect all deletion operations for logging
    std::vector<fs::path> allDeletedFiles;
    std::vector<fs::path> allKeptFiles;

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
        std::wcout << L"[0] Keep all files in this group" << std::endl;
        std::wcout << L"[1-" << files.size() << L"] Keep only the selected file (delete all others)" << std::endl;
        std::wcout << L"Press enter or enter -1 to auto-select (keep shortest path)" << std::endl;

        int choice = getUserChoiceRange(L"Please enter your choice: ", 0, static_cast<int>(files.size()), true);

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
                    allDeletedFiles.push_back(file);
                }
            }
            totalDeleted += deletedCount;
            allKeptFiles.push_back(fileToKeep);
            std::wcout << L"Successfully moved " << deletedCount << L" files to Recycle Bin." << std::endl;
        }
        else
        {
            std::wcout << L"Skipping deletion for this group." << std::endl;
        }

        std::wcout << std::endl;
        groupNumber++;
    }

    if (!allDeletedFiles.empty() || !allKeptFiles.empty())
    {
        writeDeletionLog(allDeletedFiles, allKeptFiles, "INTERACTIVE", totalDeleted, totalSizeDeleted);
    }
}

void automaticRemoval(const std::map<std::string, std::vector<fs::path>>& duplicateGroups)
{
	std::wcout << L"\n=== AUTOMATIC DUPLICATE REMOVAL ===" << std::endl;
	std::wcout << L"This will automatically keep the file with the shortest path in each duplicate group." << std::endl;
	std::wcout << L"All other duplicates will be moved to the Recycle Bin." << std::endl;

	std::vector<fs::path> filesToDelete;
	std::vector<fs::path> filesToKeep;

    for (const auto& [hash, files] : duplicateGroups)
    {
        if (files.size() <= 1) continue;

        fs::path fileToKeep = selectBestFileToKeep(files);
        filesToKeep.push_back(fileToKeep);
        for (const auto& file : files)
        {
            if (file != fileToKeep)
            {
                filesToDelete.push_back(file);
            }
		}
    }

    if (filesToDelete.empty())
    {
        std::wcout << L"No files to delete." << std::endl;
        return;
	}

	std::wcout << L"\nFiles to be kept (shortest paths):" << std::endl;
    for (const auto& file : filesToKeep)
    {
        std::wcout << L"  KEEP: " << file.wstring() << std::endl;
	}

    std::wcout << L"\nFiles to be moved to Recycle Bin:" << std::endl;
    for (const auto& file : filesToDelete)
    {
        std::wcout << L"  DELETE: " << file.wstring() << std::endl;
    }

	std::wcout << L"\nTotal files to delete: " << filesToDelete.size() << std::endl;

    if (!getUserConfirmation(L"Are you sure you want to continue? (Y/n): "))
    {
        std::wcout << L"Aborting automatic removal." << std::endl;
        return;
	}

	// Perform deletion
	size_t successCount = 0;
	uintmax_t totalSizeDeleted = 0;

    for (const auto& file : filesToDelete)
    {
        try
        {
			uintmax_t fileSize = fs::file_size(file);
            if (safeDeleteFile(file, true)) // Moves it to recycle bin
            {
                successCount++;
				totalSizeDeleted += fileSize;
				std::wcout << L"Moved to Recycle Bin: " << file.wstring() << std::endl;
            }
        }
        catch (const fs::filesystem_error& e)
        {
            std::wcerr << L"Error deleting " << file.wstring() << L": " << e.what() << std::endl;
		}
    }
    writeDeletionLog(filesToDelete, filesToKeep, "AUTOMATIC", successCount, totalSizeDeleted);
}

fs::path selectBestFileToKeep(const std::vector<fs::path>& files)
{
	fs::path bestFile = files[0];

    for (const auto& file : files)
    {
        if (file.string().length() < bestFile.string().length())
        {
            bestFile = file;
        }
	}

	return bestFile;
}

bool safeDeleteFile(const fs::path& filePath, bool useRecycleBin)
{
    if (!fs::exists(filePath))
    {
        std::wcerr << L"Can't delete file. File does not exist: " << filePath.wstring() << std::endl;
        return false;
    }

    if (useRecycleBin)
    {
		std::wstring path = filePath.wstring();
		path.push_back(L'\0'); // SHFileOperation requires double null-termination

        SHFILEOPSTRUCTW fileOp = {};
		fileOp.wFunc = FO_DELETE;
		fileOp.pFrom = path.c_str();
		fileOp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT; // Move to Recycle Bin, no confirmation, silent (no display)

        int result = SHFileOperationW(&fileOp);
        if (result != 0 || fileOp.fAnyOperationsAborted)
        {
            std::wcerr << L"Failed to move file to Recycle Bin: " << filePath.wstring() << std::endl;
			return false;
        }

		return true;
    }
    else
    {
        try
        {
            fs::remove(filePath);
            return true;
        }
        catch (const fs::filesystem_error& e)
        {
            std::string errorMsg = e.what();
            std::wstring wErrorMsg(errorMsg.begin(), errorMsg.end());
            std::wcerr << L"Error deleting file: " << wErrorMsg << std::endl;
            return false;
		}
    }
}
