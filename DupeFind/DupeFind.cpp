// Project: DupeFind, Find duplicate files in a directory

#include <iostream>
#include <filesystem>
#include <string>
#include <vector>


std::string getPath();
std::vector<std::string> getAllFilesAndDirectories(const std::string& folderPath);


int main()
{
    std::cout << "DupeFind is ready!" << std::endl;
    std::cout << "Please enter a folder path to scan: " << std::endl;

	std::string folderPath;

	while (true) 
	{
		folderPath = getPath();
		if (folderPath.empty()) 
		{
			std::cout << "Please enter a valid folder path: " << std::endl;
			continue;
		}
		break;
	}

    std::vector<std::string> foundPaths = getAllFilesAndDirectories(folderPath);

    std::cout << "\nScan completed. Found " << foundPaths.size() << " files and directories in: " << folderPath << std::endl;
	std::cout << "Here are the results:\n" << std::endl;
    for (const auto& path : foundPaths)
    {
		std::cout << path << std::endl;
    }
	


    return 0;
}

std::string getPath()
{
    std::string folderPath;
    std::getline(std::cin, folderPath);

    try
    {
        if (!std::filesystem::exists(folderPath))
        {
            std::cout << "Error: Path does not exist." << std::endl;
            return "";
        }
        if (!std::filesystem::is_directory(folderPath))
        {
            std::cout << "Error: Path is not a directory." << std::endl;
            return "";
        }
    }
    catch (const std::filesystem::filesystem_error& exception)
    {
        std::cout << "Filesystem error: " << exception.what() << std::endl;
        return ""; 
    }
    catch (const std::exception& exception)
    {
        std::cout << "Error: " << exception.what() << std::endl;
        return ""; 
    }



    return folderPath;
}

std::vector<std::string> getAllFilesAndDirectories(const std::string& folderPath) 
{
	std::vector<std::string> results;

	try {
		for (const auto& entry : std::filesystem::recursive_directory_iterator(folderPath)) 
		{
			results.push_back(entry.path().string());
		}
	}
	catch (const std::filesystem::filesystem_error& ex) {
		std::cerr << "Error accessing directory: " << ex.what() << std::endl;
	}

	return results;
}