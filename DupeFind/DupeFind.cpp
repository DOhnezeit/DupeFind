// Project: DupeFind, Find duplicate files in a directory

#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <locale>
#include <cwctype>
#include <Windows.h>


namespace fs = std::filesystem; 

fs::path getPath();
std::vector<std::wstring> getAllFilesAndDirectories(const fs::path& folderPath);
std::string wstringToUtf8(const std::wstring& wstr);


int main()
{
    std::wcout << L"DupeFind is ready!" << std::endl;
    std::wcout << L"Please enter a folder path to scan: " << std::endl;

	fs::path folderPath;

	while (true) 
	{
		folderPath = getPath();
		if (folderPath.empty()) 
		{
			std::wcout << L"Please enter a valid folder path: " << std::endl;
			continue;
		}
		break;
	}

    std::vector<std::wstring> foundPaths = getAllFilesAndDirectories(folderPath);

    std::wcout << L"\nScan completed. Found " << foundPaths.size() << L" files and directories in: " << folderPath.wstring() << std::endl;
	std::wcout << L"Here are the results:\n" << std::endl;

    std::ofstream log("scan_results.txt");
    if (!log.is_open())
    {
        std::wcerr << L"Error: Could not open log file for writing." << std::endl;
        return 1;
	}


    for (const auto& path : foundPaths)
    {
		std::wcout << path << std::endl;
        log << wstringToUtf8(path) << std::endl;
    }
	
	std::wcout << L"You can read the results in the log!" << std::endl;

    return 0;
}

fs::path getPath()
{
    std::wstring folderPath;
    std::getline(std::wcin, folderPath);

	// This removes leading and trailing whitespace from the input path
    folderPath.erase(0, folderPath.find_first_not_of(L" \t\r\n"));
	folderPath.erase(folderPath.find_last_not_of(L" \t\r\n") + 1);


    try
    {
        fs::path pathObj(std::move(folderPath));

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

std::vector<std::wstring> getAllFilesAndDirectories(const fs::path& folderPath)
{
    std::vector<std::wstring> results;

    try {
        for (auto it = fs::recursive_directory_iterator(folderPath, fs::directory_options::skip_permission_denied); it != fs::recursive_directory_iterator(); ++it)
        {
            try 
            {
				std::wstring dirName = it->path().filename().wstring();
                std::transform(dirName.begin(), dirName.end(), dirName.begin(), ::tolower);

                if (dirName == L"$recycle.bin")
                {
                    it.disable_recursion_pending(); // Skip the Recycle Bin directory
					continue;
                }

                results.push_back(it->path().wstring());
            }
            catch (const std::system_error& ex) {
                std::wcerr << L"Failed to process: " << it->path().wstring() << std::endl;
                std::wcerr << L"Error processing entry: " << ex.what() << std::endl;
            }
        }
    }
    catch (const fs::filesystem_error& ex) {
        std::wcerr << L"Error accessing directory: " << ex.what() << std::endl;
    }

    return results;
}

std::string wstringToUtf8(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();

    int size_needed = WideCharToMultiByte(
        CP_UTF8,            // Convert to UTF-8
        0,                  // Flags
        wstr.data(),        // Source wide string
        (int)wstr.size(),   // Length of wide string
        nullptr,            // No output buffer yet
        0,                  // Request size only
        nullptr, nullptr    // Default character and flag
    );

    if (size_needed <= 0)
        return std::string(); // conversion failed, return empty string

    std::string utf8str(size_needed, 0);

    // Perform the conversion
    int converted_chars = WideCharToMultiByte(
        CP_UTF8,
        0,
        wstr.data(),
        (int)wstr.size(),
        &utf8str[0],
        size_needed,
        nullptr,
        nullptr
    );

    if (converted_chars <= 0)
        return std::string();

    return utf8str;
}