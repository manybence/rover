#ifndef diverse_h
#define diverse_h

#include <algorithm>
#include <string>
#include <map>
#include "parameters.h"

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");
    return (first == std::string::npos || last == std::string::npos) 
           ? "" 
           : str.substr(first, last - first + 1);
}

// Utility function to convert string to integer
int stringToInt(const std::string& str) {
    return std::stoi(str);
}

// Utility function to convert string to float
float stringToFloat(const std::string& str) {
    return std::stof(str);
}

// Utility function to convert float to byte array
void floatToByteArray(float value, unsigned char* byteArray) {
    memcpy(byteArray, &value, sizeof(value));
}

// Utility function to convert string to boolean
bool stringToBool(const std::string& str) {

    // Trim white-space characters
    std::string lowerStr = trim(str);

    // Convert the input string to lowercase for case-insensitive comparison
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);

    // Check for common true values
    if (lowerStr == "true" || lowerStr == "1" || lowerStr == "yes" || lowerStr == "on") {
        return true;
    }

    // Check for common false values
    if (lowerStr == "false" || lowerStr == "0" || lowerStr == "no" || lowerStr == "off") {
        return false;
    }

    // Default behavior if the input doesn't match known patterns
    throw std::invalid_argument("Invalid input string for boolean conversion: " + str);
}

// Function to compare strings (case-insensitive)
bool compareStrings(const std::string& str1, const std::string& str2) {
    std::string s1 = str1;
    std::string s2 = str2;
    return (s1 == s2);
}

short int lookupString(const std::string& str, const std::map<std::string, short int>& table) {
    auto it = table.find(str);
    if (it != table.end()) {
        return it->second;
    }
    // Return a default value or handle the error appropriately
    return -1; // Assuming -1 as an invalid token
}

void replaceAll(std::string &str, const std::string &from, const std::string &to) {
    if (from.empty()) {
        return;
    }
    size_t startPos = 0;
    while ((startPos = str.find(from, startPos)) != std::string::npos) {
        str.replace(startPos, from.length(), to);
        startPos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

int findLongestRun(const std::vector<int16_t>& raw_input_data, int start_index, int stop_index) {
    if (raw_input_data.empty() || start_index > stop_index) {printf("lr error"); return 0;}

    int min_it = raw_input_data[start_index];
    int max_it = raw_input_data[start_index];

    int min_run = 0;
    int max_run = 0;
    int longest_min_run = 0;
    int longest_max_run = 0;

    for (int i = start_index; i <= stop_index; i++) {
        int x = raw_input_data[i];

        if (x > max_it) {
            max_it = x;
            max_run = 0;  // reset max run count
        }

        if (x < min_it) {
            min_it = x;
            min_run = 0;  // reset min run count
        }

        if (x == MIN_VALUE) {
            min_run++;
            if (min_run > longest_min_run) {
                longest_min_run = min_run;
            }
        } else {
            min_run = 0; // reset run
        }

        if (x == MAX_VALUE) {
            max_run++;
            if (max_run > longest_max_run) {
                longest_max_run = max_run;
            }
        } else {
            max_run = 0; // reset run
        }
    }

    return std::max(longest_min_run, longest_max_run);
}

int findPeakValue(const std::vector<int16_t>& raw_input_data, int start_index, int stop_index) {
    if (raw_input_data.empty() || start_index > stop_index) return 1;

    int16_t min_val = INT16_MAX;
    int16_t max_val = INT16_MIN;

    for (int i = start_index; i <= stop_index; i++) {
        min_val = std::min(min_val, raw_input_data[i]);
        max_val = std::max(max_val, raw_input_data[i]);
    }
    //printf("min_val: %d, max_val: %d\n", min_val, max_val);

    return max_val - min_val + 1;  // Calculate peak value
}

int mapValue(int x) {
    switch(x) {
        case 1: return    1;
        case 2: return    4;
        case 3: return    9;
        case 4: return   17;
        case 5: return   28;
        case 6: return   42;
        case 7: return   63;
        case 8: return   93;
        case 9: return  147;
        default:
            if (x < 1) return   0;
            if (x > 9) return 147;
            break;
    }
    return 0; // This line will not actually be reached due to the above logic handling all possible cases.
}


#endif
