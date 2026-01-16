#pragma once

#include <string>
#include <fstream>
#include <iostream>

namespace uvc2gl {

struct AppConfig {
    std::string videoDevice = "/dev/video0";
    std::string audioDevice = "default";
    int width = 1920;
    int height = 1080;
    int fps = 30;
    float volume = 1.0f;
    
    bool LoadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "No config file found, using defaults" << std::endl;
            return false;
        }
        
        std::string line;
        while (std::getline(file, line)) {
            size_t pos = line.find('=');
            if (pos == std::string::npos) continue;
            
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            if (key == "videoDevice") videoDevice = value;
            else if (key == "audioDevice") audioDevice = value;
            else if (key == "width") width = std::stoi(value);
            else if (key == "height") height = std::stoi(value);
            else if (key == "fps") fps = std::stoi(value);
            else if (key == "volume") volume = std::stof(value);
        }
        
        file.close();
        std::cout << "Loaded config: " << videoDevice << ", " << audioDevice 
                  << ", " << width << "x" << height << "@" << fps 
                  << ", volume=" << volume << std::endl;
        return true;
    }
    
    bool SaveToFile(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to save config to " << filename << std::endl;
            return false;
        }
        
        file << "videoDevice=" << videoDevice << "\n";
        file << "audioDevice=" << audioDevice << "\n";
        file << "width=" << width << "\n";
        file << "height=" << height << "\n";
        file << "fps=" << fps << "\n";
        file << "volume=" << volume << "\n";
        
        file.close();
        return true;
    }
};

} // namespace uvc2gl
