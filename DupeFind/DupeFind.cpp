// Project: DupeFind, Find duplicate files in a directory

#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <Windows.h>
#include <wincrypt.h>
#include <map>
#include <iomanip>


namespace fs = std::filesystem; 

fs::path getPath();
std::vector<fs::path> getAllFilesAndDirectories(const fs::path& folderPath);
std::string wstringToUtf8(const std::wstring& wstr);
std::string calculateSHA256(const fs::path& filePath);
std::map<std::string, std::vector<fs::path>> groupFilesByHash(const std::vector<fs::path>& files);
void processDuplicateGroups(const std::map<std::string, std::vector<fs::path>>& duplicateGroups);

bool shouldSkipFile(const fs::path& filePath);
bool isSystemOrEncryptedFile(const fs::path& filePath);



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

    std::vector<fs::path> foundPaths = getAllFilesAndDirectories(folderPath);

    std::wcout << L"\nScan completed. Found " << foundPaths.size() << L" files and directories in: " << folderPath.wstring() << std::endl;
	std::wcout << L"Here are the results:\n" << std::endl;

    const size_t MAX_CONSOLE_DISPLAY = 50;
    bool showInConsole = foundPaths.size() <= MAX_CONSOLE_DISPLAY;

    if (showInConsole) {
        std::wcout << L"Here are the results:\n" << std::endl;
    }
    else {
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

        if (showInConsole) {
            std::wcout << wIndent << path.filename() << std::endl;
        }
        else {
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
    processDuplicateGroups(duplicateGroups);

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
		// Convert the input string to a filesystem path object
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
                    std::wcout << L"Skipping system file: " << it->path().filename().wstring() << std::endl;
                    continue;
                }

                results.push_back(it->path());
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

	// Just calculating the size needed for the UTF-8 string, conversion will be done in the next step
    int size_needed = WideCharToMultiByte(
        CP_UTF8,            
        0,                  
        wstr.data(),        
        (int)wstr.size(),   
        nullptr,            
        0,                  
        nullptr, nullptr    
    );

    if (size_needed <= 0)
        return std::string(); // conversion failed, return empty string

    std::string utf8str(size_needed, 0);

	// Actually converting the wide string to UTF-8
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

std::string calculateSHA256(const fs::path& filePath)
{
    std::wcout << L"Hashing: " << filePath.filename().wstring() << std::endl;

	std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        std::wcerr << L"Error: Could not open file " << filePath.wstring() << std::endl;
		return {};
    }

	HCRYPTPROV hProv = NULL; // Cryptographic provider handle
	HCRYPTHASH hHash = NULL; // Hash handle
    const DWORD HASH_LENGTH = 32; // 256 bits = 32 bytes
	BYTE rgbHash[HASH_LENGTH]; 
	DWORD cbHash = HASH_LENGTH; 

	// Acquire a cryptographic provider context
    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) 
    {
        std::wcerr << L"Error: CryptAcquireContext failed." << std::endl;
        file.close();
        return {};
	}

	// Create a hash object for SHA-256
    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash))
    {
        std::wcerr << L"Error: CryptCreateHash failed." << std::endl;
        CryptReleaseContext(hProv, 0);
        file.close();
        return {};
    }
    
	const size_t BUFFER_SIZE = 65536; // 64 KB buffer
	std::vector<char> buffer(BUFFER_SIZE);

	while (file.read(buffer.data(), BUFFER_SIZE) || file.gcount() > 0) 
    {
		size_t bytesRead = file.gcount();
		
        if (!CryptHashData(hHash, reinterpret_cast<BYTE*>(buffer.data()), static_cast<DWORD>(bytesRead), 0)) 
        {
            std::wcerr << L"Error: CryptHashData failed." << std::endl;
            CryptDestroyHash(hHash);
            CryptReleaseContext(hProv, 0);
            file.close();
            return {};
        }
    }

    if (!CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0)) 
    {
        std::wcerr << L"Error: CryptGetHashParam failed." << std::endl;
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        file.close();
        return {};
	}

	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);
    file.close();

	std::ostringstream oss;
    for (DWORD i = 0; i < cbHash; ++i) 
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rgbHash[i]);
	}

    return oss.str();
}

std::map<std::string, std::vector<fs::path>> groupFilesByHash(const std::vector<fs::path>& files)
{
    std::map<std::string, std::vector<fs::path>> hashGroups;

    for (const auto& file : files)
    {
        if (!fs::is_regular_file(file)) continue; // Skip directories and non-regular files

        try
        {
            std::string hash = calculateSHA256(file);
            hashGroups[hash].push_back(file);
        }
        catch (const std::exception& e)
        {
            std::wcerr << L"Error processing file " << file.wstring() << L": " << e.what() << std::endl;
        }
    }
    return hashGroups;
}

void processDuplicateGroups(const std::map<std::string, std::vector<fs::path>>& duplicateGroups)
{
    std::wofstream log("scan_results.txt", std::ios::app);

    if (!log.is_open())
    {
        std::wcerr << L"Error: Could not open duplicates log file for writing." << std::endl;
		return;
    }

	size_t groupCount = 0;
    for (const auto& [hash, files] : duplicateGroups)
    {
		if (files.size() <= 1) continue; // Skip unique files

        ++groupCount;
        std::wcout << L"Duplicate group #" << groupCount << L" (SHA-256: " << std::wstring(hash.begin(), hash.end()) << L")" << std::endl;
        log << L"Duplicate group #" << groupCount << L" (SHA-256: " << std::wstring(hash.begin(), hash.end()) << L")" << std::endl;

        for (const auto& file : files)
        {
            std::wcout << L"  " << file.wstring() << std::endl;
            log << L"  " << file.wstring() << std::endl;
		}
    }

    if (groupCount == 0)
    {
        std::wcout << L"No duplicate files found." << std::endl;
        log << L"No duplicate files found." << std::endl;
    }
    else
    {
        std::wcout << L"Total duplicate groups found: " << groupCount << std::endl;
        log << L"Total duplicate groups found: " << groupCount << std::endl;
	}

    log.close();
}

bool shouldSkipFile(const fs::path& filePath)
{
    std::wstring fileName = filePath.filename().wstring();
    std::transform(fileName.begin(), fileName.end(), fileName.begin(), ::tolower);

	// Just a few common files that we want to skip
    if (fileName == L"desktop.ini" ||
        fileName == L"thumbs.db" ||
        fileName == L"folder.jpg" ||
        fileName == L"albumartsmall.jpg" ||
        fileName == L"albumart_{*.jpg")
    {
        return true;
    }

    // Check if it's a system/hidden file using Windows attributes
    return isSystemOrEncryptedFile(filePath);
}

bool isSystemOrEncryptedFile(const fs::path& filePath)
{

	DWORD attributes = GetFileAttributesW(filePath.wstring().c_str());

    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        return false; // If we can't get attributes, we assume it's not a system file
	}

	if (attributes & FILE_ATTRIBUTE_SYSTEM || attributes & FILE_ATTRIBUTE_ENCRYPTED || attributes & FILE_ATTRIBUTE_HIDDEN) // Check if the file is a system file, encrypted, or hidden
    {
        return true; // It's a system file
	}
  
    return false;
}
