#pragma once

#include <string>
#include <vector>
#include <map>
#include <filesystem>


namespace fs = std::filesystem;

std::string calculateSHA256(const fs::path& filePath);

std::map<std::string, std::vector<fs::path>> groupFilesByHash(const std::vector<fs::path>& files);