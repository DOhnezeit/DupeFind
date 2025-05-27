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


namespace fs = std::filesystem; 

fs::path getPath();
std::vector<fs::path> getAllFilesAndDirectories(const fs::path& folderPath);
std::string wstringToUtf8(const std::wstring& wstr);
std::string calculateSHA256(const fs::path& filePath);
std::map<std::string, std::vector<fs::path>> groupFilesByHash(const std::vector<fs::path>& files);
void processDuplicateGroups(const std::map<std::string, std::vector<fs::path>>& duplicateGroups);



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

    std::ofstream log("scan_results.txt");
    if (!log.is_open())
    {
        std::wcerr << L"Error: Could not open log file for writing." << std::endl;
        return 1;
	}


    for (const auto& path : foundPaths)
    {
        fs::path relativePath = fs::relative(path, folderPath);
        auto depth = std::distance(relativePath.begin(), relativePath.end());

		// Create an indent based on the depth of the path
		// Using a wide string for the indent, then converting it to UTF-8 for the log

        std::wstring wIndent = (depth > 1) ? std::wstring((depth - 1) * 2, L' ') : L"";
        std::string indent = wstringToUtf8(wIndent);

        std::wcout << wIndent << path.filename() << std::endl;
        log << std::string(indent.begin(), indent.end()) << wstringToUtf8(path.filename().wstring()) << std::endl;
    }
    log.close();
	std::wcout << L"You can read about the found files in the log!" << std::endl;

	// Calculate SHA-256 for the first file found in the directory (testing purposes)
    for (const auto& entry : fs::recursive_directory_iterator(folderPath))
    {
        if (fs::is_regular_file(entry.path()))
        {
            std::string hexHash = calculateSHA256(entry.path());
            std::wcout << L"First file: " << entry.path().wstring() << std::endl;
            std::wcout << L"SHA-256: " << std::wstring(hexHash.begin(), hexHash.end()) << std::endl;
            break;
        }
    }

	// TODO: Continue here with the MD5 calculation and duplicate detection

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

                if (dirName == L"$recycle.bin")
                {
                    it.disable_recursion_pending(); // Skip the Recycle Bin directory
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

	// Create a hash object for MD5
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
	// TODO: Implement the logic to group files by their MD5 hash
    return {};
}

void processDuplicateGroups(const std::map<std::string, std::vector<fs::path>>& duplicateGroups)
{
	// TODO: Implement the logic to process duplicate groups
    return;
}