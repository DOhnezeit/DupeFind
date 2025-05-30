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

void writeUtf8ToFile(const std::wstring& text, const std::wstring& filePath, bool newline)
{
    std::wofstream log(filePath, std::ios::app | std::ios::binary);
    if (log.is_open())
    {
        // HACK: Write BOM once if needed, might change later
        static bool wroteBOM = false;
        if (!wroteBOM)
        {
            wchar_t bom = 0xFEFF;
            log.write(&bom, 1);
            wroteBOM = true;
        }

        log << text;

        if (newline)
        {
            log << L"\n";
		}
    }
}





