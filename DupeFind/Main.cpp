﻿// Project: DupeFind, Find duplicate files in a directory
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
#include <map>

#include <io.h>
#include <fcntl.h>

namespace fs = std::filesystem;

int main()
{
	(void)_setmode(_fileno(stdin), _O_U16TEXT);
	(void)_setmode(_fileno(stdout), _O_U16TEXT);
	(void)_setmode(_fileno(stderr), _O_U16TEXT);

	resetLogFiles();

	std::wcout << L"DupeFind is ready!" << std::endl;
	fs::path folderPath;
	while (true) 
	{
        std::wstring input = getUserInput(L"Enter folder path to scan: ");
		printUnicodeMulti(true, L"DEBUG Input Path: ", input); // TODO: Remove this line after debugging
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

	writeScanLog(foundPaths, folderPath, 1000);
    
	std::wcout << L"You can read about the found files in the log!" << std::endl;

    std::wcout << L"\nChecking for duplicate files..." << std::endl;
    std::map<std::string, std::vector<fs::path>> duplicateGroups = groupFilesByHash(foundPaths);
    size_t duplicateGroupCount = processDuplicateGroups(duplicateGroups);
    if (duplicateGroupCount > 0)
    {
        handleDuplicateRemoval(duplicateGroups);
    }
	
	std::wcout << L"\nPress enter to exit...";
	std::wstring quitInput;
	std::getline(std::wcin, quitInput);

    return 0;
}


