// Project: DupeFind, Find duplicate files in a directory

#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <fstream>

namespace fs = std::filesystem; 

fs::path getPath();
std::vector<std::string> getAllFilesAndDirectories(const fs::path& folderPath);


int main()
{
    std::cout << "DupeFind is ready!" << std::endl;
    std::cout << "Please enter a folder path to scan: " << std::endl;

	fs::path folderPath;

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

    std::ofstream log("scan_results.txt", std::ios::out | std::ios::binary);

    for (const auto& path : foundPaths)
    {
		std::cout << path << std::endl;
        log << path << std::endl;
    }
	
	std::cout << "You can read the results in the log!" << std::endl;

    return 0;
}

fs::path getPath()
{
    std::string folderPath;
    std::getline(std::cin, folderPath);

	// This removes leading and trailing whitespace from the input path
    folderPath.erase(0, folderPath.find_first_not_of(" \t\r\n"));
	folderPath.erase(folderPath.find_last_not_of(" \t\r\n") + 1);


    try
    {
        fs::path pathObj(std::move(folderPath));

        if (!fs::exists(pathObj))
        {
            std::cout << "Error: Path does not exist." << std::endl;
            return {};
        }
        if (!fs::is_directory(pathObj))
        {
            std::cout << "Error: Path is not a directory." << std::endl;
            return {};
        }

        return fs::canonical(pathObj);
    }

    catch (const fs::filesystem_error& exception)
    {
        std::cout << "Filesystem error: " << exception.what() << std::endl;
        std::cout << "Failed on path: " << exception.path1() << std::endl;

        return {};
    }
    catch (const std::exception& exception)
    {
        std::cout << "Error: " << exception.what() << std::endl;
        return {};
    }
}

std::vector<std::string> getAllFilesAndDirectories(const fs::path& folderPath)
{
    std::vector<std::string> results;

    try {
        for (auto it = fs::recursive_directory_iterator(folderPath, fs::directory_options::skip_permission_denied); it != fs::recursive_directory_iterator(); ++it)
        {
            try 
            {
				std::string dirName = it->path().filename().string();
                std::transform(dirName.begin(), dirName.end(), dirName.begin(), ::tolower);

                if (dirName == "$recycle.bin")
                {
                    it.disable_recursion_pending(); // Skip the Recycle Bin directory
					continue;
                }

                results.push_back(it->path().string());
            }
            catch (const std::system_error& ex) {
                std::wcerr << "Failed to process: " << it->path().wstring() << std::endl;
                std::cerr << "Error processing entry: " << ex.what() << std::endl;
            }
        }
    }
    catch (const fs::filesystem_error& ex) {
        std::cerr << "Error accessing directory: " << ex.what() << std::endl;
    }

    return results;
}