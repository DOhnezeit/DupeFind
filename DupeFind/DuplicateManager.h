#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace fs = std::filesystem;

void handleDuplicateRemoval(const std::map<std::string, std::vector<fs::path>>& duplicateGroups);

void interactiveRemoval(const std::map<std::string, std::vector<fs::path>>& duplicateGroups);

void automaticRemoval(const std::map<std::string, std::vector<fs::path>>& duplicateGroups);

fs::path selectBestFileToKeep(const std::vector<fs::path>& files);

bool safeDeleteFile(const fs::path& filePath, bool useRecycleBin = true);