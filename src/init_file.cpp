#include "init_file.h"
#include <fstream>
#include <sstream>
#include <algorithm>

std::vector<std::string> InitFile::loadWords(const std::string& filePath) {
    std::vector<std::string> result;
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        return result;
    }

    std::string word;
    while (file >> word) {
        std::string cleanWord = "";
        for (char c : word) {
            if (isalpha(c)) {
                cleanWord += (char)tolower(c);
            }
        }
        if (!cleanWord.empty()) {
            result.push_back(cleanWord);
        }
    }

    file.close();
    return result;
}

std::vector<int> InitFile::loadNumbers(const std::string& filePath) {
    std::vector<int> result;
    std::ifstream file(filePath);

    if (!file.is_open()) {
        return result;
    }

    std::string token;
    while (file >> token) {
        try {
            result.push_back(std::stoi(token));
        } catch (...) {
            continue;
        }
    }

    file.close();
    return result;
}
