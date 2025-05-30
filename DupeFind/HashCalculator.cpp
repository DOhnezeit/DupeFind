#include "HashCalculator.h"
#include "Utilities.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wincrypt.h>

std::string calculateSHA256(const fs::path& filePath)  
{  
    if (!fs::exists(filePath)) return "empty_file";  

	printUnicodeMulti(true, L"Calculating hash for file: ", filePath.wstring());

    HANDLE hFile = CreateFileW(  
        filePath.wstring().c_str(),
        GENERIC_READ,  
        FILE_SHARE_READ,  
        NULL,  
        OPEN_EXISTING,  
        FILE_ATTRIBUTE_NORMAL || FILE_FLAG_SEQUENTIAL_SCAN,
        NULL);  

    if (hFile == INVALID_HANDLE_VALUE)  
    {  
		DWORD error = GetLastError();
		std::wstring wsError = std::to_wstring(error);

		printUnicodeMulti(true, L"Error opening file: ", filePath.wstring(), L" (Error code: ", wsError, L")");
        return {};  
    }  

    LARGE_INTEGER fileSizeLI;
    if (!GetFileSizeEx(hFile, &fileSizeLI))
    {
		printUnicodeMulti(true, L"Error getting file size: ", filePath.wstring());
        CloseHandle(hFile);
        return {};
    }

    // Check if the file is empty  
    if (fileSizeLI.QuadPart == 0)  
    {  
        CloseHandle(hFile);  
        return "empty_file";  
    }  

    if (fileSizeLI.QuadPart > 2LL * 1024 * 1024 * 1024) // 2GB limit hack  
    {  
		printUnicodeMulti(true, L"Skipping large file (>2GB): ", filePath.wstring());
        CloseHandle(hFile);  
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
        CloseHandle(hFile);  
        return {};  
    }  

    // Create a hash object for SHA-256  
    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash))  
    {  
        std::wcerr << L"Error: CryptCreateHash failed." << std::endl;  
        CryptReleaseContext(hProv, 0);  
        CloseHandle(hFile);  
        return {};  
    }  

    const size_t BUFFER_SIZE = 65536; // 64 KB buffer  
    std::vector<BYTE> buffer(BUFFER_SIZE);  
    DWORD bytesRead = 0;  

    while (ReadFile(hFile, buffer.data(), BUFFER_SIZE, &bytesRead, NULL) && bytesRead > 0)  
    {  
        if (!CryptHashData(hHash, buffer.data(), bytesRead, 0))  
        {  
            std::wcerr << L"Error: CryptHashData failed." << std::endl;  
            CryptDestroyHash(hHash);  
            CryptReleaseContext(hProv, 0);  
            CloseHandle(hFile);  
            return {};  
        }  
    }  

    if (!CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0))  
    {  
        std::wcerr << L"Error: CryptGetHashParam failed." << std::endl;  
        CryptDestroyHash(hHash);  
        CryptReleaseContext(hProv, 0);  
        CloseHandle(hFile);  
        return {};  
    }  

    CryptDestroyHash(hHash);  
    CryptReleaseContext(hProv, 0);  
    CloseHandle(hFile);  

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
    size_t processedFiles = 0;
    size_t totalFiles = 0;

    // Count regular files first
    for (const auto& file : files)
    {
        if (fs::is_regular_file(file)) totalFiles++;
    }

    std::wcout << L"Processing " << totalFiles << L" files for duplicate detection..." << std::endl;

    for (const auto& file : files)
    {
        if (!fs::is_regular_file(file)) continue; // Skip directories and non-regular files

        try
        {
            processedFiles++;

            if (totalFiles <= 100 || processedFiles % 100 == 0) // Show progress every 100 files
            {
                std::wstring wsProcessed = std::to_wstring(processedFiles);
                std::wstring wsTotal = std::to_wstring(totalFiles);

				// CHECK: This might or might not work, maybe test more
                printUnicodeMulti(true, L"Progress: ", wsProcessed, L"/", wsTotal, L" - ", file.wstring());
            }

            std::string hash = calculateSHA256(file);

            if (hash.empty()) continue; // Skip files that failed to hash

            hashGroups[hash].push_back(file);
        }
        catch (const std::exception& e)
        {
            std::wstring wsExceptionMsg = utf8ToWstring(e.what());
            printUnicodeMulti(true, L"Error processing file ", file.wstring(), wsExceptionMsg);
        }
    }
    std::wcout << L"Finished processing files." << std::endl;
    return hashGroups;
}