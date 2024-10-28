#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <stdexcept>

std::string readFile(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::map<std::string, std::string> parseSerializedString(const std::string &data) {
    std::map<std::string, std::string> settings;
    size_t pos = 0;

    auto parseString = [&](size_t &pos) -> std::string {
        if (data[pos] != 's') throw std::runtime_error("Expected 's'");
        pos += 2;  // Skip 's:'
        size_t lenEnd = data.find(':', pos);
        size_t length = std::stoul(data.substr(pos, lenEnd - pos));
        pos = lenEnd + 2;  // Skip ':'
        std::string str = data.substr(pos, length);
        pos += length + 2;  // Skip ':"'
        return str;
    };

    auto parseBoolean = [&](size_t &pos) -> std::string {
        if (data[pos] != 'b') throw std::runtime_error("Expected 'b'");
        pos += 2;  // Skip 'b:'
        std::string value = (data[pos] == '1') ? "true" : "false";
        pos += 2;  // Skip 'value;'
        return value;
    };

    if (data[pos] != 'a') throw std::runtime_error("Expected 'a'");
    pos += 2;  // Skip 'a:'
    size_t itemCountEnd = data.find(':', pos);
    size_t itemCount = std::stoul(data.substr(pos, itemCountEnd - pos));
    pos = itemCountEnd + 2;  // Skip ':'

    for (size_t i = 0; i < itemCount; ++i) {
        std::string key = parseString(pos);
        if (data[pos] == 's') {
            std::string value = parseString(pos);
            settings[key] = value;
        } else if (data[pos] == 'b') {
            std::string value = parseBoolean(pos);
            settings[key] = value;
        } else {
            throw std::runtime_error("Unsupported data type");
        }
    }

    return settings;
}

int main() {
    try {
        //std::string data = readFile("..\\..\\..\\..\\var\\www\\html\\settings.txt");
        std::string data = readFile("settings.txt");
        std::map<std::string, std::string> settings = parseSerializedString(data);

        for (const auto &pair : settings) {
            std::cout << pair.first << " : " << pair.second << std::endl;
        }
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
