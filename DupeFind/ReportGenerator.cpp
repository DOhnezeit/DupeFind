#include "ReportGenerator.h"
#include "Utilities.h"

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <filesystem>



size_t processDuplicateGroups(const std::map<std::string, std::vector<fs::path>>& duplicateGroups)
{
    std::wofstream log("scan_results.txt", std::ios::app);

    if (!log.is_open())
    {
        std::wcerr << L"Error: Could not open duplicates log file for writing." << std::endl;
        return 0;
    }

    size_t groupCount = 0;
    size_t totalDuplicateFiles = 0;
    uintmax_t totalDuplicateSize = 0;

    log << L"\n\n=== DUPLICATE FILES ANALYSIS ===" << std::endl;
    std::wcout << L"\n=== DUPLICATE FILES ANALYSIS ===" << std::endl;

    for (const auto& [hash, files] : duplicateGroups)
    {
        if (files.size() <= 1) continue; // Skip unique files

        ++groupCount;
        totalDuplicateFiles += files.size() - 1; // count duplicates excluding the original

        uintmax_t fileSize = 0;
        try
        {
            fileSize = fs::file_size(files[0]);
            totalDuplicateSize += fileSize * (files.size() - 1);  // accumulate wasted space
        }
        catch (const fs::filesystem_error& e)
        {
            std::wcerr << L"Error getting file size for " << files[0].wstring() << L": " << e.what() << std::endl;
            continue;
        }

        std::string fileSizeStr = formatFileSize(fileSize);
        std::wstring fileSizeWStr = std::wstring(fileSizeStr.begin(), fileSizeStr.end());

        std::wstring hashWStr = std::wstring(hash.begin(), hash.end());

        std::wcout << L"Duplicate group #" << groupCount << L" (" << files.size() << L" files, "
            << fileSizeWStr << L" each)" << std::endl;
        std::wcout << L"SHA-256: " << hashWStr << std::endl;

        log << L"Duplicate group #" << groupCount << L" (" << files.size() << L" files, "
            << fileSizeWStr << L" each)" << std::endl;
        log << L"SHA-256: " << hashWStr << std::endl;

        for (const auto& file : files)
        {
            std::wcout << L"  " << file.wstring() << std::endl;
            log << L"  " << file.wstring() << std::endl;
        }
        std::wcout << std::endl; // Add blank line between groups
        log << std::endl;
    }

    if (groupCount == 0)
    {
        std::wcout << L"No duplicate files found." << std::endl;
        log << L"No duplicate files found." << std::endl;
    }
    else
    {
        std::string totalSizeStr = formatFileSize(totalDuplicateSize);
        std::wstring totalSizeWStr(totalSizeStr.begin(), totalSizeStr.end());

        std::wcout << L"=== SUMMARY ===" << std::endl;
        std::wcout << L"Total duplicate groups found: " << groupCount << std::endl;
        std::wcout << L"Total duplicate files: " << totalDuplicateFiles << std::endl;
        std::wcout << L"Total wasted space: " << totalSizeWStr << std::endl;

        log << L"=== SUMMARY ===" << std::endl;
        log << L"Total duplicate groups found: " << groupCount << std::endl;
        log << L"Total duplicate files: " << totalDuplicateFiles << std::endl;
        log << L"Total wasted space: " << totalSizeWStr << std::endl;
    }

    log.close();
    return groupCount;
}

void writeLogFile(const std::vector<fs::path>& paths, const fs::path& basePath)
{
	// TODO: Implement this function to write the log file with the paths and base path
}
