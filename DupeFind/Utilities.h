#pragma once

#include <string>
#include <cstdint>
#include <filesystem>

namespace fs = std::filesystem;


std::string wstringToUtf8(const std::wstring& wstr);

std::string formatFileSize(uintmax_t bytes);

std::string safeFilenameDisplay(const fs::path& filePath);