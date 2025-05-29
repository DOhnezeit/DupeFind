#include "InputHandler.h"

#include <iostream>


std::wstring getUserInput(const std::wstring& prompt)
{
    if (!prompt.empty())
    {
        std::wcout << prompt << std::endl;
    }

    std::wstring input;
    std::getline(std::wcin, input);

    input.erase(0, input.find_first_not_of(L" \t\r\n"));
    input.erase(input.find_last_not_of(L" \t\r\n") + 1);

    return input;
}

int getUserChoice(const std::wstring& prompt, int minChoice, int maxChoice)
{
    while (true)
    {
        std::wstring input = getUserInput(prompt);

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