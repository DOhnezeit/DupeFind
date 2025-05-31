#pragma once

#include <string>
#include <vector>
#include <map>
#include <filesystem>

namespace fs = std::filesystem;

size_t processDuplicateGroups(const std::map<std::string, std::vector<fs::path>>& duplicateGroups);

void writeScanLog(const std::vector<fs::path>& paths, const fs::path& basePath, size_t maxEntries = 1000);

void writeDeletionLog(const std::vector<fs::path>& deletedFiles, const std::vector<fs::path>& keptFiles, const std::string& removalType, size_t successCount, uintmax_t totalSizeDeleted);

std::wstring getCurrentTimestamp();

void resetLogFiles();