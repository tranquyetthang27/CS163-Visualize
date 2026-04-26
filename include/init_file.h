#ifndef INIT_FILE_H
#define INIT_FILE_H

#include <string>
#include <vector>

class InitFile {
public:
    static std::vector<std::string> loadLines(const std::string& filePath);
    static std::vector<std::string> loadWords(const std::string& filePath);
    static std::vector<int> loadNumbers(const std::string& filePath);
};

#endif
