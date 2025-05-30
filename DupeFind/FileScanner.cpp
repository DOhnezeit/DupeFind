﻿#include "FileScanner.h"
#include "Utilities.h" 

#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_set>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


fs::path convertToPath(const std::wstring& input)
{
    try
    {
        fs::path pathObj(input);

        if (!fs::exists(pathObj))
        {
            printUnicodeMulti(true, L"Error: Path does not exist: ", pathObj.wstring());
            return {};
        }
        if (!fs::is_directory(pathObj))
        {
            printUnicodeMulti(true, L"Error: Path is not a directory: ", pathObj.wstring());
            return {};
        }

        // Normalize the path to its canonical form (resolving any symbolic links, relative paths, etc.)
        return fs::canonical(pathObj);
    }

    catch (const fs::filesystem_error& exception)
    {
        printUnicodeMulti(true,
            L"Filesystem error: ",
            utf8ToWstring(exception.what()),
            L"\nFailed on path: ",
            exception.path1().wstring());

		return {};
    }
    catch (const std::exception& exception)
    {
        printUnicodeMulti(true, L"Error: ", utf8ToWstring(exception.what()));
        return {};
    }
}

std::vector<fs::path> getAllFilesAndDirectories(const fs::path& folderPath)
{
    std::vector<fs::path> results;

    try {
        for (auto it = fs::recursive_directory_iterator(folderPath, fs::directory_options::skip_permission_denied); it != fs::recursive_directory_iterator(); ++it)
        {
            try
            {
                std::wstring dirName = it->path().filename().wstring();
                std::transform(dirName.begin(), dirName.end(), dirName.begin(), ::tolower);

                if (dirName == L"$recycle.bin" || dirName == L"system volume information")
                {
                    it.disable_recursion_pending(); // Skip the Recycle Bin directory and System Volume Information
                    continue;
                }

                if (shouldSkipFile(it->path()))
                {
                    // Reduces console spam
                    if (results.size() < 1000)
                    {
                        printUnicodeMulti(true, L"Skipping system file: ", it->path().filename().wstring());
                    }
                    continue;
                }

                results.push_back(it->path());
            }
            catch (const std::system_error& ex) 
            {
                printUnicodeMulti(true, L"Failed to process: ", it->path().wstring());
                printUnicodeMulti(true, L"Error processing entry: ", utf8ToWstring(ex.what()));

                continue;
            }
        }
    }
    catch (const fs::filesystem_error& ex) 
    {
        printUnicodeMulti(true, L"Error accessing directory: ", utf8ToWstring(ex.what()));
    }

    return results;
}

bool isSystemOrEncryptedFile(const fs::path& filePath)
{

    DWORD attributes = GetFileAttributesW(filePath.wstring().c_str());

    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        return false; // If we can't get attributes, we assume it's not a system file
    }

    if (attributes & FILE_ATTRIBUTE_SYSTEM || attributes & FILE_ATTRIBUTE_ENCRYPTED || attributes & FILE_ATTRIBUTE_HIDDEN || attributes & FILE_ATTRIBUTE_REPARSE_POINT) // Check if the file is a system file, encrypted, or hidden
    {
        return true; // It's a system file
    }

    return false;
}

bool shouldSkipFile(const fs::path& filePath)
{
    std::wstring fileName = filePath.filename().wstring();
    std::transform(fileName.begin(), fileName.end(), fileName.begin(), ::tolower);

    std::wstring ext = filePath.extension().wstring();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);

    static const std::unordered_set<std::wstring> skippedExtensions = 
    {
        L".sys", L".log", L".tmp", L".bak", L".swp", L".dll"
    };

	if (skippedExtensions.count(ext)) return true; // Skip files with these extensions
    if (isSystemOrEncryptedFile(filePath)) return true;

    // Skip desktop.ini files, which are used by Windows to store folder view settings
    if (fileName == L"desktop.ini")
    {
        return true;
    }

    return false;
}
