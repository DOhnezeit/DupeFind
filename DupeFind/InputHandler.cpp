#include "InputHandler.h"

#include <algorithm>
#include <iostream>


std::wstring getUserInput(const std::wstring& prompt)
{
    if (!prompt.empty())
    {
        std::wcout << prompt;
    }

    std::wstring input;
    std::getline(std::wcin, input);

    input.erase(0, input.find_first_not_of(L" \t\r\n"));
    input.erase(input.find_last_not_of(L" \t\r\n") + 1);

    return input;
}

int getUserChoiceRange(const std::wstring& prompt, int minChoice, int maxChoice, bool useDefaultValue, int defaultValue)
{
    while (true)
    {
        std::wstring input = getUserInput(prompt);

        if (useDefaultValue && input.empty())
        {
            return defaultValue;
		}

        try
        {
            int choice = std::stoi(input);
            if (choice >= minChoice && choice <= maxChoice)
            {
                return choice;
            }
            else
            {
                std::wcout << L"Please enter a number between " << minChoice << L" and " << maxChoice << L": ";
            }
        }
        catch (const std::exception&)
        {
            std::wcout << L"Please enter a number between " << minChoice << L" and " << maxChoice << L": ";
        }
    }
}

bool getUserConfirmation(const std::wstring& prompt, bool defaultValue)
{
    std::wstring input = getUserInput(prompt);

	if (input.empty())
	{
		return defaultValue;
	}

	std::transform(input.begin(), input.end(), input.begin(), ::tolower);

    if (input == L"y" || input == L"yes" || input == L"1" || input == L"true") return true;
    if (input == L"n" || input == L"no" || input == L"0" || input == L"false") return false;

	std::wcout << L"Invalid input. Please enter 'y' for yes or 'n' for no: " << std::endl;
	return getUserConfirmation(L"", defaultValue);
}
