#include "Utilities.h"

#include <iomanip>
#include <sstream>
#include <string>
#include <filesystem>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


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

std::string safeFilenameDisplay(const fs::path& filePath)
{
    try
    {
        std::wstring filename_wstr = filePath.filename().wstring();

        std::string utf8_filename = wstringToUtf8(filename_wstr);

		// If the conversion to UTF-8 was successful, return the UTF-8 string
        if (!utf8_filename.empty()) return utf8_filename;
        
		// If conversion failed, return the raw filename as a fallback
        std::string generic_str = filePath.filename().generic_string();

		// If the generic string is not empty, it means we can display it safely
        if (!generic_str.empty()) return generic_str + " [encoding issue]";

		// If both conversions failed, return a placeholder indicating an invalid filename
        return "[Invalid filename - encoding error]";
    }

    catch (const std::exception& e)
    {
        return std::string("[Error reading filename: ") + e.what() + "]";
	}

    catch (...)
    {
        return "[Unknown error reading filename]";
	}
}