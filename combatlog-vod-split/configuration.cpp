#include <windows.h>
#include <iostream>
#include <shlobj.h>
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream> 
#include <sstream>
#include <vector>

#include "configuration.h"

void configuration::SetConfiguration()
{
    //gathers document folder location
    TCHAR szPath[MAX_PATH];
    HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, szPath);
    std::wstring arr_w(szPath);
    std::string tmp(arr_w.begin(), arr_w.end());
    this->my_documents = tmp;
};

bool configuration::CheckForConfigFile()
{
    //checks if combat_log_vod_split directory exists and create it does not exist
    std::string configDirectory = my_documents + "\\combat_log_vod_split";
    std::filesystem::path tmp = configDirectory;
    CreateDirectory(tmp.c_str(), NULL);

    //checks to see if configuration file exists and if it does not creates a config file
    if (!std::filesystem::exists(configDirectory + "\\config.conf"))
    {
        std::ofstream outfile(configDirectory + "\\config.conf");
        outfile << "video_directory=\nlog_directory=\nmode=";
        outfile.close();
        std::cout << "Configuration file not found please configure" << std::endl;
        return false;
    }
    else
    {
        std::string line;
        std::ifstream file(configDirectory + "\\config.conf");

        //reads config file and sets values locally in the program
        while (std::getline(file, line))
        {
            try
            {
                int found = line.find("=");
                if (found > 1 && found != line.size())
                {
                    std::string setting = line.substr(0, found);
                    if (setting == "video_directory")
                    {
                        this->video_directory = line.substr(found + 1, line.size() - 1);
                    }
                    else if (setting == "log_directory")
                    {
                        this->log_directory = line.substr(found + 1, line.size() - 1);
                    }
                    else if (setting == "mode")
                    {
                        this->mode = line.substr(found + 1, line.size() - 1);
                    }
                    else if (setting == "threads")
                    {
                        this->threads = stoi(line.substr(found + 1, line.size() - 1));
                    }
                }
            }
            catch (const std::runtime_error& error)
            {
                std::cout << error.what() << '\n';
            }
        }

        //checks for missing settings
        if (video_directory == "")
        {
            std::cout << "Error setting video directory - please check config file" << std::endl;
            return false;
        }
        if (log_directory == "")
        {
            std::cout << "Error setting log directory - please check config file" << std::endl;
            return false;
        }
        if (mode == "")
        {
            std::cout << "Error setting mode - please check config file" << std::endl;
            return false;
        }
    }

    return true;
};