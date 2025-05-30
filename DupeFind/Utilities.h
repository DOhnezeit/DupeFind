#pragma once

#include <string>
#include <cstdint>


std::string wstringToUtf8(const std::wstring& wstr);
std::wstring utf8ToWstring(const std::string& str);

std::string formatFileSize(uintmax_t bytes);

void printUnicode(const std::wstring& text, bool newline = false);
void printUnicode(const wchar_t* text, bool newline = false);

template <typename... Args>
void printUnicodeMulti(bool newline, Args&&... args)
{
    std::wstring combinedText;
    ((combinedText += std::forward<Args>(args)), ...);
    printUnicode(combinedText, newline);
}

void writeUtf8ToFile(const std::wstring& text, const std::wstring& filePath, bool newline = false);


