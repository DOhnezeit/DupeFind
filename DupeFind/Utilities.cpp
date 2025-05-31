#include "Utilities.h"

#include <iomanip>
#include <sstream>
#include <string>
#include <filesystem>
#include <fstream>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>




namespace fs = std::filesystem;

std::string formatFileSize(uintmax_t bytes)
{
    const char* units[] = { "B", "KB", "MB", "GB", "TB" };
    int unit = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024.0 && unit < 4) 
    {
        size /= 1024.0;
        unit++;
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << size << " " << units[unit];
    return oss.str();
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
        utf8str.data(),
        size_needed,
        nullptr,
        nullptr
    );

    if (converted_chars <= 0)
        return std::string();

    return utf8str;
}

std::wstring utf8ToWstring(const std::string& str)
{
    if (str.empty()) return std::wstring();

    // Calculate the size needed for the wide string buffer
    int size_needed = MultiByteToWideChar(
        CP_UTF8,
        0,
        str.data(),
        (int)str.size(),
        nullptr,
        0
    );

    if (size_needed <= 0)
        return std::wstring();

    std::wstring wstr(size_needed, 0);

    // Perform the conversion from UTF-8 to wide char
    int converted_chars = MultiByteToWideChar(
        CP_UTF8,
        0,
        str.data(),
        (int)str.size(),
        &wstr[0],
        size_needed
    );

    if (converted_chars <= 0)
        return std::wstring();

    return wstr;
}

void printUnicode(const std::wstring& text, bool newline)
{
	static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD written;

    WriteConsoleW(hConsole, text.c_str(), (DWORD)text.length(), &written, nullptr);

    if (newline)
    {
        WriteConsoleW(hConsole, L"\n", 1, &written, nullptr);
    }
}

void printUnicode(const wchar_t* text, bool newline)
{
    printUnicode(std::wstring(text), newline);
}

void writeUnicodeToFile(const std::wstring& text, const std::wstring& filePath, bool newline, bool append)
{
    HANDLE hFile = CreateFileW(
        filePath.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        nullptr,
        append ? OPEN_ALWAYS : CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        printUnicode(L"Error: Could not open file for writing: " + filePath, true);
        return;
    }

    // If appending, move to end of file
    if (append)
    {
        SetFilePointer(hFile, 0, nullptr, FILE_END);
    }

    // Write UTF-16 BOM if file is new/empty
    DWORD fileSize = GetFileSize(hFile, nullptr);
    if (fileSize == 0)
    {
        WORD bom = 0xFEFF;
        DWORD written;
        WriteFile(hFile, &bom, sizeof(bom), &written, nullptr);
    }

    // Write the text
    DWORD written;
    WriteFile(hFile, text.c_str(), (DWORD)(text.length() * sizeof(wchar_t)), &written, nullptr);

    if (newline)
    {
        WriteFile(hFile, L"\r\n", 2 * sizeof(wchar_t), &written, nullptr);
    }

    CloseHandle(hFile);
}





