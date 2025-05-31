#pragma once

#include <filesystem>
#include <vector>
#include <string>


namespace fs = std::filesystem;

fs::path convertToPath(const std::wstring& input);

std::vector<fs::path> getAllFilesAndDirectories(const fs::path& folderPath);

bool shouldSkipFile(const fs::path& filePath);

bool isSystemOrEncryptedFile(const fs::path& filePath);