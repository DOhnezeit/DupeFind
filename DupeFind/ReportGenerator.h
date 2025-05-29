#pragma once

#include <string>
#include <vector>
#include <map>
#include <filesystem>

namespace fs = std::filesystem;

size_t processDuplicateGroups(const std::map<std::string, std::vector<fs::path>>& duplicateGroups);
void writeLogFile(const std::vector<fs::path>& paths, const fs::path& basePath);