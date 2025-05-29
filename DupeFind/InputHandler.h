#pragma once

#include <string>

std::wstring getUserInput(const std::wstring& prompt);

int getUserChoice(const std::wstring& prompt, int minChoice, int maxChoice);

std::wstring getUserInput(const std::wstring& prompt);