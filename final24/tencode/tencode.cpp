#include <iostream>
#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <cstdlib>

struct WordGain {
    std::string word;
    unsigned gain;
};

bool wg_less_than(const WordGain &a, const WordGain &b) {
    if (a.gain != b.gain) {
        return a.gain > b.gain; 
    }
    return a.word < b.word;    
}

void encode(std::istream &input, std::ostream &output, std::ostream &error) {

    std::ostringstream buffer;
    buffer << input.rdbuf();
    std::string content = buffer.str();


    std::unordered_map<std::string,int> word_count;
    std::vector<std::string> words;
    std::string w;
    for (size_t i = 0; i < content.size(); ++i) {
        if (std::isalpha(static_cast<unsigned char>(content[i]))) {
            w.push_back(content[i]);
        } else {
            if (!w.empty()) {
                words.push_back(w);
                word_count[w]++;
                w.clear();
            }
            if (!std::isspace(static_cast<unsigned char>(content[i]))) {
                words.push_back(std::string(1, content[i]));
            }
        }
    }
    if (!w.empty()) {
        words.push_back(w);
        word_count[w]++;
    }

    std::vector<WordGain> gains;
    gains.reserve(word_count.size());
    for (std::unordered_map<std::string,int>::iterator it = word_count.begin(); it != word_count.end(); ++it) {
        const std::string &wd = it->first;
        int c = it->second;
        unsigned lw = (unsigned)wd.size();
        int g = c * (int)lw - ((int)lw + 1 + c);
        if (g > 0) {
            WordGain tmp;
            tmp.word = wd;
            tmp.gain = (unsigned)g;
            gains.push_back(tmp);
        }
    }

    std::sort(gains.begin(), gains.end(), wg_less_than);

    std::unordered_map<std::string, unsigned char> code_map;
    int code = 128;
    for (size_t i = 0; i < gains.size() && code < 256; ++i, ++code) {
        code_map[gains[i].word] = (unsigned char)code;
    }

    if (code_map.empty()) {
        output << content;
        return;
    }

    for (std::unordered_map<std::string,unsigned char>::iterator it = code_map.begin(); it != code_map.end(); ++it) {
        output << it->second << it->first;
    }
    output << '\n';

    for (size_t i = 0; i < words.size(); ++i) {
        if (code_map.find(words[i]) != code_map.end()) {
            output << code_map[words[i]];
        } else {
            output << words[i];
        }
    }
}

int main() {
    encode(std::cin, std::cout, std::cerr);
    return 0;
}
