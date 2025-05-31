#include "ReportGenerator.h"
#include "Utilities.h"

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <filesystem>
#include <sstream>
#include <iomanip>



size_t processDuplicateGroups(const std::map<std::string, std::vector<fs::path>>& duplicateGroups)
{
    const std::wstring logFileName = L"duplicate_log.txt";

    // Build the complete log content in memory first
    std::wstringstream logContent;
    size_t groupCount = 0;
    size_t totalDuplicateFiles = 0;
    uintmax_t totalDuplicateSize = 0;

    // Process groups and collect statistics
    std::wstringstream groupsBuffer;
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
            std::wstring errorMsg = L"Error getting file size for " + files[0].wstring() + L": " + utf8ToWstring(e.what());
            printUnicode(errorMsg, true);
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

    // Build the complete log content
    logContent << L"=== DUPLICATE FILES ANALYSIS ===" << std::endl;

    if (groupCount == 0)
    {
        logContent << L"No duplicate files found." << std::endl;
        printUnicode(L"No duplicate files found.", true);
    }
    else
    {
        std::string totalSizeStr = formatFileSize(totalDuplicateSize);
        std::wstring totalSizeWStr(totalSizeStr.begin(), totalSizeStr.end());

        // Write summary to log content
        logContent << L"=== SUMMARY ===" << std::endl;
        logContent << L"Total duplicate groups found: " << groupCount << std::endl;
        logContent << L"Total duplicate files: " << totalDuplicateFiles << std::endl;
        logContent << L"Total wasted space: " << totalSizeWStr << std::endl << std::endl;

        // Print summary to console
        printUnicode(L"\n=== SUMMARY ===", true);
        printUnicode(L"Total duplicate groups found: " + std::to_wstring(groupCount), true);
        printUnicode(L"Total duplicate files: " + std::to_wstring(totalDuplicateFiles), true);
        printUnicode(L"Total wasted space: " + totalSizeWStr, true);
    }

    // Append the buffered group details
    logContent << groupsBuffer.str();

    // Write everything to file using your Unicode function
    writeUnicodeToFile(logContent.str(), logFileName, false, false); // newline=false since content already has newlines, append=false to overwrite

    printUnicode(L"Duplicate analysis written to: " + logFileName, true);

    return groupCount;
}

void writeScanLog(const std::vector<fs::path>& paths, const fs::path& basePath, size_t maxEntries)
{
    const std::wstring logFileName = L"scan_results.txt";
    std::wstringstream logContent;

    logContent << L"=== DUPEFIND SCAN RESULTS ===" << std::endl;
    logContent << L"Scan Date: " << getCurrentTimestamp() << std::endl;
    logContent << L"Base Directory: " << basePath.wstring() << std::endl;
    logContent << L"Total Files/Directories Found: " << paths.size() << std::endl;
    logContent << std::endl;

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
    logContent << L"\n=== SUMMARY STATISTICS ===" << std::endl;
    logContent << L"Directories: " << dirCount << std::endl;
    logContent << L"Files: " << fileCount << std::endl;
    if (totalSize > 0)
    {
        std::string totalSizeStr = formatFileSize(totalSize);
        std::wstring totalSizeWStr(totalSizeStr.begin(), totalSizeStr.end());
        logContent << L"Total Size: " << totalSizeWStr << std::endl;
    }
    logContent << std::endl;

    // Write the file tree header
    logContent << L"\n=== FILE TREE (limited to " << maxEntries << L" entries) ===" << std::endl;

	writeUnicodeToFile(logContent.str(), logFileName);


    size_t entriesWritten = 0;
    for (const auto& path : paths)
    {
        if (entriesWritten >= maxEntries)
        {
            size_t remaining = paths.size() - entriesWritten;
			std::wstringstream truncatedMessage;
            truncatedMessage << L"... (file tree truncated, " << remaining << L" more entries not shown)";
			writeUnicodeToFile(truncatedMessage.str(), logFileName);
            break;
        }

        try
        {
            std::wstringstream entryContent;

            // Calculate relative path and indentation
            fs::path relativePath = fs::relative(path, basePath);
            auto depth = std::distance(relativePath.begin(), relativePath.end());
            std::wstring indent = (depth > 1) ? std::wstring((depth - 1) * 2, L' ') : L"";

            // Write the entry
            entryContent << indent << path.filename().wstring();

            // Add file info if regular file
            if (fs::is_regular_file(path))
            {
                try
                {
                    uintmax_t fileSize = fs::file_size(path);
                    std::string fileSizeStr = formatFileSize(fileSize);
                    std::wstring fileSizeWStr(fileSizeStr.begin(), fileSizeStr.end());
                    entryContent << L" (" << fileSizeWStr << L")";
                }
                catch (const fs::filesystem_error&)
                {
                    entryContent << L" (size unknown)";
                }
            }
            else if (fs::is_directory(path))
            {
                entryContent << L"/";  // trailing slash for directories
            }

			writeUnicodeToFile(entryContent.str(), logFileName);
        }
        catch (const fs::filesystem_error& e)
        {
            std::wstring message = L"Error processing: " + path.wstring() + L" - " + utf8ToWstring(e.what());
            printUnicodeMulti(true, message);

            writeUnicodeToFile(message, logFileName);
        }

        entriesWritten++;
    }

    printUnicode(L"Scan results written to: " + logFileName, true);
}

void writeDeletionLog(const std::vector<fs::path>& deletedFiles, const std::vector<fs::path>& keptFiles, const std::string& removalType, size_t successCount, uintmax_t totalSizeDeleted)
{
	const std::wstring logFileName = L"deletion_log.txt";
    std::wstringstream logContent;


    logContent << L"=== " << std::wstring(removalType.begin(), removalType.end()) << L" REMOVAL ===" << std::endl;
    logContent << L"Timestamp: " << getCurrentTimestamp() << std::endl;
    logContent << L"Total files processed: " << deletedFiles.size() << std::endl;
    logContent << L"Successfully deleted: " << successCount << std::endl;

    if (totalSizeDeleted > 0)
    {
        std::string spaceFreedStr = formatFileSize(totalSizeDeleted);
        logContent << L"Total space freed: " << std::wstring(spaceFreedStr.begin(), spaceFreedStr.end()) << std::endl;
    }

    logContent << std::endl;

	writeUnicodeToFile(logContent.str(), logFileName, false, true);

    if (!keptFiles.empty())
    {
        writeUnicodeToFile(L"Files kept:", logFileName, true, true);
        for (const auto& file : keptFiles)
        {
            writeUnicodeToFile(L"  KEEP: " + file.wstring(), logFileName);
        }
        writeUnicodeToFile(L"", logFileName);
    }

    writeUnicodeToFile(L"Files moved to Recycle Bin:", logFileName);
    for (const auto& file : deletedFiles)
    {
        writeUnicodeToFile(L"  DELETE: " + file.wstring(), logFileName);
    }
	writeUnicodeToFile(L"", logFileName);

    // Also write summary to console
    printUnicode(L"\n=== REMOVAL SUMMARY ===", true);
    printUnicode(L"Total files moved to Recycle Bin: " + std::to_wstring(successCount), true);
    if (totalSizeDeleted > 0)
    {
        std::string spaceFreedStr = formatFileSize(totalSizeDeleted);
        printUnicode(L"Total space freed: " + std::wstring(spaceFreedStr.begin(), spaceFreedStr.end()), true);
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

void resetLogFiles()
{
    const std::vector<std::wstring> logFiles = {
        L"scan_results.txt",
        L"duplicate_log.txt",
        L"deletion_log.txt"
    };

    for (const auto& logFile : logFiles)
    {
        // Simply delete the file if it exists
        if (fs::exists(logFile))
        {
            try
            {
                fs::remove(logFile);
            }
            catch (const fs::filesystem_error& e)
            {
                // Non-critical error, just notify
                printUnicode(L"Warning: Could not delete old log file: " + logFile + L" - " + utf8ToWstring(e.what()), true);
            }
        }
    }
}


