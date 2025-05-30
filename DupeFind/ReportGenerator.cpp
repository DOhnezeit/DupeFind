#include "ReportGenerator.h"
#include "Utilities.h"

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <filesystem>
#include <time.h>
#include <sstream>
#include <iomanip>


size_t processDuplicateGroups(const std::map<std::string, std::vector<fs::path>>& duplicateGroups)
{
	// HACK: Currently writing the log here but should be moved to a separate function

    std::wstringstream groupsBuffer;

    size_t groupCount = 0;
    size_t totalDuplicateFiles = 0;
    uintmax_t totalDuplicateSize = 0;

    // Process groups and write group info into buffer
    for (const auto& [hash, files] : duplicateGroups)
    {
        if (files.size() <= 1) continue; // Skip unique files

        ++groupCount;
        totalDuplicateFiles += files.size() - 1;

        uintmax_t fileSize = 0;
        try
        {
            fileSize = fs::file_size(files[0]);
            totalDuplicateSize += fileSize * (files.size() - 1);
        }
        catch (const fs::filesystem_error& e)
        {
            std::wcerr << L"Error getting file size for " << files[0].wstring() << L": " << e.what() << std::endl;
            continue;
        }

        std::string fileSizeStr = formatFileSize(fileSize);
        std::wstring fileSizeWStr(fileSizeStr.begin(), fileSizeStr.end());

        std::wstring hashWStr = std::wstring(hash.begin(), hash.end());

        groupsBuffer << L"Duplicate group #" << groupCount << L" (" << files.size() << L" files, " << fileSizeWStr << L" each)" << std::endl;
        groupsBuffer << L"SHA-256: " << hashWStr << std::endl;

        for (const auto& file : files)
        {
            groupsBuffer << L"  " << file.wstring() << std::endl;
        }
        groupsBuffer << std::endl;
    }

    // Open log file for appending
    std::wofstream log("duplicate_log.txt");
    if (!log.is_open())
    {
        std::wcerr << L"Error: Could not open duplicates log file for writing." << std::endl;
        return 0;
    }

    // Write summary first
    log << L"=== DUPLICATE FILES ANALYSIS ===" << std::endl;

    if (groupCount == 0)
    {
        log << L"No duplicate files found." << std::endl;
        std::wcout << L"No duplicate files found." << std::endl;
    }
    else
    {
        std::string totalSizeStr = formatFileSize(totalDuplicateSize);
        std::wstring totalSizeWStr(totalSizeStr.begin(), totalSizeStr.end());

        log << L"\n=== SUMMARY ===" << std::endl;
        log << L"Total duplicate groups found: " << groupCount << std::endl;
        log << L"Total duplicate files: " << totalDuplicateFiles << std::endl;
        log << L"Total wasted space: " << totalSizeWStr << std::endl << std::endl;

        std::wcout << L"\n=== SUMMARY ===" << std::endl;
        std::wcout << L"Total duplicate groups found: " << groupCount << std::endl;
        std::wcout << L"Total duplicate files: " << totalDuplicateFiles << std::endl;
        std::wcout << L"Total wasted space: " << totalSizeWStr << std::endl;
    }

    // Write the buffered group details
    log << groupsBuffer.str();

    log.close();
    return groupCount;
}

void writeScanLog(const std::vector<fs::path>& paths, const fs::path& basePath, size_t maxEntries)
{
    const std::string logFileName = "scan_results.txt";

    std::wofstream log(logFileName);
    if (!log.is_open())
    {
        std::wcerr << L"Error: Could not open log file '"
            << std::wstring(logFileName.begin(), logFileName.end())
            << L"' for writing." << std::endl;
        return;
    }

    log << L"=== DUPEFIND SCAN RESULTS ===" << std::endl;
    log << L"Scan Date: " << getCurrentTimestamp() << std::endl;
    log << L"Base Directory: " << basePath.wstring() << std::endl;
    log << L"Total Files/Directories Found: " << paths.size() << std::endl;
    log << std::endl;

    size_t fileCount = 0;
    size_t dirCount = 0;
    uintmax_t totalSize = 0;

    // Precompute counts & sizes before writing file tree for accurate summary
    for (const auto& path : paths)
    {
        try
        {
            if (fs::is_regular_file(path))
            {
                fileCount++;
                totalSize += fs::file_size(path);
            }
            else if (fs::is_directory(path))
            {
                dirCount++;
            }
        }
        catch (const fs::filesystem_error&)
        {
            // Ignore size errors
        }
    }

    // Write summary statistics first
    log << L"\n=== SUMMARY STATISTICS ===" << std::endl;
    log << L"Directories: " << dirCount << std::endl;
    log << L"Files: " << fileCount << std::endl;
    if (totalSize > 0)
    {
        std::string totalSizeStr = formatFileSize(totalSize);
        std::wstring totalSizeWStr(totalSizeStr.begin(), totalSizeStr.end());
        log << L"Total Size: " << totalSizeWStr << std::endl;
    }
    log << std::endl;

    // Write the file tree header
    log << L"\n=== FILE TREE (limited to " << maxEntries << L" entries) ===" << std::endl;

    size_t entriesWritten = 0;
    for (const auto& path : paths)
    {
        if (entriesWritten >= maxEntries)
        {
            size_t remaining = paths.size() - entriesWritten;
            log << L"... (file tree truncated, " << remaining << L" more entries not shown)" << std::endl;
            break;
        }

        try
        {
            // Calculate relative path and indentation
            fs::path relativePath = fs::relative(path, basePath);
            auto depth = std::distance(relativePath.begin(), relativePath.end());
            std::wstring indent = (depth > 1) ? std::wstring((depth - 1) * 2, L' ') : L"";

            // Write the entry
            log << indent << path.filename().wstring();

            // Add file info if regular file
            if (fs::is_regular_file(path))
            {
                try
                {
                    uintmax_t fileSize = fs::file_size(path);
                    std::string fileSizeStr = formatFileSize(fileSize);
                    std::wstring fileSizeWStr(fileSizeStr.begin(), fileSizeStr.end());
                    log << L" (" << fileSizeWStr << L")";
                }
                catch (const fs::filesystem_error&)
                {
                    log << L" (size unknown)";
                }
            }
            else if (fs::is_directory(path))
            {
                log << L"/";  // trailing slash for directories
            }

            log << std::endl;
        }
        catch (const fs::filesystem_error& e)
        {
            log << L"Error processing: " << path.wstring() << L" - " << e.what() << std::endl;
        }

        entriesWritten++;
    }

    log.close();

    std::wcout << L"Scan results written to: " << std::wstring(logFileName.begin(), logFileName.end()) << std::endl;
}

void writeDeletionLog(const std::vector<fs::path>& deletedFiles, const std::vector<fs::path>& keptFiles, const std::string& removalType, size_t successCount, uintmax_t totalSizeDeleted)
{
    std::wofstream log("duplicate_log.txt", std::ios::app);
    if (!log.is_open())
    {
        std::wcerr << L"Error: Could not open deletion log file for writing." << std::endl;
        return;
    }

    log << L"\n=== " << std::wstring(removalType.begin(), removalType.end()) << L" REMOVAL ===" << std::endl;
    log << L"Timestamp: " << getCurrentTimestamp() << std::endl;
    log << L"Total files processed: " << deletedFiles.size() << std::endl;
    log << L"Successfully deleted: " << successCount << std::endl;

    if (totalSizeDeleted > 0)
    {
        std::string spaceFreedStr = formatFileSize(totalSizeDeleted);
        log << L"Total space freed: " << std::wstring(spaceFreedStr.begin(), spaceFreedStr.end()) << std::endl;
    }

    log << std::endl;

    if (!keptFiles.empty())
    {
        log << L"Files kept:" << std::endl;
        for (const auto& file : keptFiles)
        {
            log << L"  KEEP: " << file.wstring() << std::endl;
        }
        log << std::endl;
    }

    log << L"Files moved to Recycle Bin:" << std::endl;
    for (const auto& file : deletedFiles)
    {
        log << L"  DELETE: " << file.wstring() << std::endl;
    }
    log << std::endl;

    log.close();

    // Also write summary to console
    std::wcout << L"=== REMOVAL SUMMARY ===" << std::endl;
    std::wcout << L"Total files moved to Recycle Bin: " << successCount << std::endl;
    if (totalSizeDeleted > 0)
    {
        std::string spaceFreedStr = formatFileSize(totalSizeDeleted);
        std::wcout << L"Total space freed: " << std::wstring(spaceFreedStr.begin(), spaceFreedStr.end()) << std::endl;
    }
}

std::wstring getCurrentTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);

    struct tm localTime;
    localtime_s(&localTime, &time_t_now);

    std::wstringstream wss;
    wss << std::put_time(&localTime, L"%Y-%m-%d %H:%M:%S");

    return wss.str();
}



