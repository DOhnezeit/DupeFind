// Project: DupeFind, Find duplicate files in a directory

#include <iostream>
#include <filesystem>
#include <string>

std::string getPath();


int main()
{
    std::cout << "DupeFind is ready!" << std::endl;
    std::cout << "Please enter a folder path to scan: " << std::endl;

	std::string folderPath = getPath();

	// Check for correct path format

	try
	{
		if (!std::filesystem::exists(folderPath))
		{
			std::cout << "Error: Path does not exist." << std::endl;
			return 1;
		}

		if (!std::filesystem::is_directory(folderPath))
		{
			std::cout << "Error: Path is not a directory." << std::endl;
		}

		std::cout << "Scanning directory: " << folderPath << std::endl;

		// TODO: Scan here

	}

	catch (const std::filesystem::filesystem_error& exception)
	{
		std::cout << "Filesystem error: " << exception.what() << std::endl;
		return 1;
	}
	catch (const std::exception& exception)
	{
		std::cout << "Error: " << exception.what() << std::endl;
		return 1;
	}


    return 0;
}

std::string getPath()
{
	std::string path;
	std::getline(std::cin, path);
	return path;
}