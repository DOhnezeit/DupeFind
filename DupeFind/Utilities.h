#pragma once

#include <string>



std::string wstringToUtf8(const std::wstring& wstr);

std::string formatFileSize(uintmax_t bytes);
