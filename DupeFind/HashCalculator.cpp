#include "HashCalculator.h"

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
    std::wcout << L"Hashing: " << filePath.filename().wstring() << std::endl;

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        std::wcerr << L"Error: Could not open file " << filePath.wstring() << std::endl;
        return {};
    }

    // Check file size before proceeding, don't need to hash empty files
    try
    {
        file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        // HACK: For now, skip very large files (>2GB) that might cause issues, think of a different way to handle this later
        if (fileSize > 2LL * 1024 * 1024 * 1024)
        {
            std::wcout << L"Skipping large file (>2GB): " << filePath.filename().wstring() << std::endl;
            file.close();
            return {};
        }

        // Skip empty files
        if (fileSize == 0)
        {
            file.close();
            return "empty_file"; // Special hash for empty files
        }
    }

    catch (const std::exception& e)
    {
        std::wcerr << L"Exception caught: " << e.what() << std::endl;
        file.close();
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
                std::wcout << L"Progress: " << processedFiles << L"/" << totalFiles << L" - " << file.filename().wstring() << std::endl;
            }

            std::string hash = calculateSHA256(file);

            if (hash.empty()) continue; // Skip files that failed to hash

            hashGroups[hash].push_back(file);
        }
        catch (const std::exception& e)
        {
            std::wcerr << L"Error processing file " << file.wstring() << L": " << e.what() << std::endl;
        }
    }
    return hashGroups;
}