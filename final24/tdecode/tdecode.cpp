#include <iostream>
#include <unordered_map>
#include <string>
#include <sstream>
#include <cctype>

void decode(std::istream &input, std::ostream &output, std::ostream &error) {
    std::unordered_map<unsigned char, std::string> code_map;
    std::string line;

    if (!std::getline(input, line)) {
        error << "Error: Empty input file." << std::endl;
        exit(EXIT_FAILURE);
    }

    size_t pos = 0;
    while (pos < line.size() && static_cast<unsigned char>(line[pos]) >= 128) {
        unsigned char key = line[pos++];
        std::string value;

        while (pos < line.size() && std::isalpha(line[pos])) {
            value += line[pos++];
        }

        if (value.empty()) {
            error << "Error: Invalid encoding map." << std::endl;
            exit(EXIT_FAILURE);
        }

        code_map[key] = value;
    }

    std::string decoded_text;
    decoded_text += line.substr(pos);

    while (std::getline(input, line)) {
        for (size_t i = 0; i < line.size(); ++i) {
            unsigned char ch = static_cast<unsigned char>(line[i]);
            if (ch >= 128) {
                if (code_map.find(ch) == code_map.end()) {
                    error << "Error: Undefined encoding for byte " << (int)ch << std::endl;
                    exit(EXIT_FAILURE);
                }
                decoded_text += code_map[ch];
            } else {
                decoded_text += ch;
            }
        }
        decoded_text += '\n';
    }

    output << decoded_text;
}

int main() {
    decode(std::cin, std::cout, std::cerr);
    return 0;
}