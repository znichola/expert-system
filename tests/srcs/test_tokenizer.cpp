#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "expert-system.hpp"

std::string getStdInput();
std::string getFileInput(char *fileName);
struct InputOptions {
    bool isHelp = false;
    char *file = nullptr;
};

InputOptions parseInput(int ac, char **av) {
    InputOptions res;

    for (int i = 1; i < ac; i++) {
        std::string s(av[i]);
        if (s == "--help" || s == "-h")
            res.isHelp = true;
        else
            res.file = av[i];
    }
    return res;
}


std::string getStdInput() {
    std::string ret;
    std::string buff;
    while (std::getline(std::cin, buff)) {
        auto pos = buff.find(";;");
        if (pos == std::string::npos)
            ret += buff + "\n";
        else {
            ret += buff.substr(0, pos);
            break ;
        }
    }
    return ret;
}


std::string getFileInput(char *fileName) {
    std::ifstream file(fileName);
    if (!file) throw std::runtime_error("Cannot open file \"" 
            + std::string(fileName) + "\"");
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

int main(int argc, char ** argv) {
    std::cout << "Tokenizing tests" << std::endl; 
    std::string input;
    InputOptions opts = parseInput(argc, argv);
    if (argc == 1)
    {
        try {
            std::string t = "./tokenizerTest/test.txt";
            input = getFileInput(const_cast<char*>(t.c_str()));
        } catch (std::exception &e) {
            std::cerr << "Error reading default file | " << e.what() << std::endl;
            return 1;
        }
    }
    else
    {
        try {
            input = opts.file == nullptr ? getStdInput() : getFileInput(opts.file);
        } catch (std::exception &e) {
            std::cerr << "Startup error | " << e.what() << std::endl;
            return 1;
        }
    }
    // printf("Input:\n%s\n", input.c_str());
    try {
        vector<Token> tokens = tokenizer(input);
        vector<Fact> facts = parseFacts(tokens);
        // for (const auto &t : tokens.token_list) {
        //     std::cout << "[" << t.second << "] '" << t.first << "'" << std::endl;
        // }
        for (const auto &f : facts) {
            std::cout << f.toString() << " in line "<< f.line_number << " with comment " << f.comment <<std::endl;
        }
    } catch (std::exception &e) {
        std::cerr << "Tokenizing error | " << e.what() << std::endl;
        return 1;
    }
}
