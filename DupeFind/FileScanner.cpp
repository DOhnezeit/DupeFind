#include "FileScanner.h"

#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <unordered_set>

fs::path convertToPath(std::wstring input)
{
    try
    {
        // Convert the input string to a filesystem path object
        fs::path pathObj(std::move(input));

        if (!fs::exists(pathObj))
        {
            std::wcout << L"Error: Path does not exist." << std::endl;
            return {};
        }
        if (!fs::is_directory(pathObj))
        {
            std::wcout << L"Error: Path is not a directory." << std::endl;
            return {};
        }

        // Normalize the path to its canonical form (resolving any symbolic links, relative paths, etc.)
        return fs::canonical(pathObj);
    }

    catch (const fs::filesystem_error& exception)
    {
        std::wcout << L"Filesystem error: " << exception.what() << std::endl;
        std::wcout << L"Failed on path: " << exception.path1().wstring() << std::endl;

        return {};
    }
    catch (const std::exception& exception)
    {
        std::wcout << L"Error: " << exception.what() << std::endl;
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
                        std::wcout << L"Skipping system file: " << it->path().filename().wstring() << std::endl;
                    }
                    continue;
                }

                results.push_back(it->path());
            }
            catch (const std::system_error& ex) {
                std::wcerr << L"Failed to process: " << it->path().wstring() << std::endl;
                std::wcerr << L"Error processing entry: " << ex.what() << std::endl;
                continue;
            }
        }
    }
    catch (const fs::filesystem_error& ex) {
        std::wcerr << L"Error accessing directory: " << ex.what() << std::endl;
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
