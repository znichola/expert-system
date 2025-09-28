#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "expert-system.hpp"
#include "parser.hpp"
#include "server.hpp"

struct InputOptions {
    bool isHelp = false;
    char *file = nullptr;
    bool isServer = false;
    bool isExplain = false;
    bool isDot = false;
    bool isFacts = false;
};


std::string getStdInput();
std::string getFileInput(char *fileName);
InputOptions parseInput(int ac, char **av);

bool isHelpPrint(const InputOptions &opts, char *argv0);
std::string getInputOrErrorExit(const InputOptions &opts);
bool isServerLaunch(const InputOptions &opts);

std::string getNewFactsLineFromUser(std::string input)
{
    std::cout << "Enter new facts line (e.g., '=AB'): ";
    std::string newFactsLine;
    std::getline(std::cin, newFactsLine);
    // Replace or add the facts line in the input
    std::istringstream iss(input);
    std::string line;
    std::string updatedInput;
    bool foundFactsLine = false;
    while (std::getline(iss, line)) {
        if (!line.empty() && line[0] == '=') {
            updatedInput += newFactsLine + "\n";
            foundFactsLine = true;
        } else {
            updatedInput += line + "\n";
        }
    }
    if (!foundFactsLine) {
        updatedInput += newFactsLine + "\n";
    }
    return updatedInput;
}

int main(int argc, char ** argv) {
    InputOptions opts = parseInput(argc, argv);

    if (isHelpPrint(opts, argv[0])) 
        return 0;

    if (isServerLaunch(opts))
        return 0;
    std::string input;
    try {
        input = getInputOrErrorExit(opts);
        if (input.empty())
        {
            std::cerr << "No input provided, exiting." << std::endl;
            return 1;
        }
    } catch (std::exception &e) {
        std::cerr << "Startup error | " << e.what() << std::endl;
        return 1;
    }
    // MAIN ENTRY POINT
    bool userWantToChangeFacts = true;
    while (userWantToChangeFacts) {
        try {
            std::vector<Token> tokens = tokenizer(input);
            auto [rules, facts, queries] = parseTokens(tokens);
            Digraph digraph = makeDigraph(facts, rules);

            for (const auto &query : queries) {
                auto res = digraph.solveForFact(query.label, opts.isExplain);
                std::cout << query.label << " is " << res << std::endl;
            }
        } catch (std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
        if (!opts.isFacts)
            break;
        std::cout << "Do you want to change the facts and re-evaluate? (y/n): ";
        std::string answer;
        std::getline(std::cin, answer);
        if (answer != "y" && answer != "Y") {
            userWantToChangeFacts = false;
        } else {
            input = getNewFactsLineFromUser(input);
        }
    }
    return 0;
}


InputOptions parseInput(int ac, char **av) {
    InputOptions res;

    for (int i = 1; i < ac; i++) {
        std::string s(av[i]);
        if (s == "--help" || s == "-h")
            res.isHelp = true;
        if (s == "--server" || s == "-s")
            res.isServer = true;
        if (s == "--explain" || s == "-e")
            res.isExplain = true;
        if (s == "--dot" || s == "-d")
            res.isDot = true;
        if (s == "--facts" || s == "-f")
            res.isFacts = true;
        else
            res.file = av[i];
    }
    return res;
}


std::string getStdInput() {
    std::string ret;
    std::string buff;
    std::cout << "Enter your input (end with ';;' on a new line):" << std::endl;
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

bool isHelpPrint(const InputOptions &opts , char *argv0) {
    if (!opts.isHelp)
        return false;

    std::cout << "Expert System .. in propositional calculation."
    << std::endl
    << std::endl << "run with " << argv0 << " propositional_statments.txt"
    << std::endl
    << std::endl << "flags -h / --help    to get this help message"
    << std::endl << "      -s / --server  luanches a webserver for an interactive interface"
    << std::endl;

    return true;
}

std::string getInputOrErrorExit(const InputOptions &opts) {
    try {
        return opts.file == nullptr ? getStdInput() : getFileInput(opts.file);
    } catch (std::exception &e) {
        std::cerr << "Startup error | " << e.what() << std::endl;
        exit(1);
    }
}

bool isServerLaunch(const InputOptions &opts) {
    if (!opts.isServer)
        return false;
    WebServer webServer(7711);
    webServer.start(); // blocks on the server
    return true;
}
