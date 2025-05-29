#pragma once

#include <string>

std::wstring getUserInput(const std::wstring& prompt);

int getUserChoiceRange(const std::wstring& prompt, int minChoice, int maxChoice, bool useDefaultValue = false, int defaultValue = -1);

bool getUserConfirmation(const std::wstring& prompt, bool defaultValue = true);