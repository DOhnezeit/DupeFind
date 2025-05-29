// Project: DupeFind, Find duplicate files in a directory
#include "FileScanner.h"
#include "HashCalculator.h"
#include "DuplicateManager.h"
#include "InputHandler.h"
#include "ReportGenerator.h"
#include "Utilities.h"

#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <fstream>
#include <map>


int main()
{
    std::wcout << L"DupeFind is ready!" << std::endl;

	fs::path folderPath;

	while (true) 
	{
        std::wstring input = getUserInput(L"Enter folder path to scan: ");
		folderPath = convertToPath(input);
		if (folderPath.empty()) 
		{
			std::wcout << L"Please enter a valid folder path: " << std::endl;
			continue;
		}
		break;
	}

    std::vector<fs::path> foundPaths = getAllFilesAndDirectories(folderPath);

    std::wcout << L"\nScan completed. Found " << foundPaths.size() << L" files and directories in: " << folderPath.wstring() << std::endl;

    const size_t MAX_CONSOLE_DISPLAY = 50;
    bool showInConsole = foundPaths.size() <= MAX_CONSOLE_DISPLAY;

    if (showInConsole) 
    {
        std::wcout << L"Here are the results:\n" << std::endl;
    }
    else 
    {
        std::wcout << L"Too many files to display in console (" << foundPaths.size() << L" files)." << std::endl;
        std::wcout << L"Writing all results to log file only...\n" << std::endl;
    }


    std::ofstream log("scan_results.txt");
    if (!log.is_open())
    {
        std::wcerr << L"Error: Could not open log file for writing." << std::endl;
        return 1;
	}

    size_t fileCount = 0;
    for (const auto& path : foundPaths)
    {
        fs::path relativePath = fs::relative(path, folderPath);
        auto depth = std::distance(relativePath.begin(), relativePath.end());

		// Create an indent based on the depth of the path
		// Using a wide string for the indent, then converting it to UTF-8 for the log

        std::wstring wIndent = (depth > 1) ? std::wstring((depth - 1) * 2, L' ') : L"";
        std::string indent = wstringToUtf8(wIndent);

        if (showInConsole) 
        {
            std::wcout << wIndent << path.filename() << std::endl;
        }
        else 
        {
            // Show progress every 1000 files
            if (fileCount % 1000 == 0) {
                std::wcout << L"Processed " << fileCount << L"/" << foundPaths.size() << L" files..." << std::endl;
            }
        }

        log << std::string(indent.begin(), indent.end()) << wstringToUtf8(path.filename().wstring()) << std::endl;
        fileCount++;
    }
    
	std::wcout << L"You can read about the found files in the log!" << std::endl;
    log.close();

    std::wcout << L"\nChecking for duplicate files..." << std::endl;

    std::map<std::string, std::vector<fs::path>> duplicateGroups = groupFilesByHash(foundPaths);
    size_t duplicateGroupCount = processDuplicateGroups(duplicateGroups);

    // TODO: Remove duplicates in main

    /*if (duplicateGroupCount > 0)
    {
        handleDuplicateRemoval(duplicateGroups);
    }*/
	

    return 0;
}


