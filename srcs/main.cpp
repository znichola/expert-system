#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "expert-system.hpp"
#include "parser.hpp"
#include "server.hpp"


InputOptions parseInput(int ac, char **av);
std::string getInputOrErrorExit(const InputOptions &opts);
std::string getNewFactsLineFromUser(std::string input);
bool isHelpPrint(const InputOptions &opts, char *argv0);
bool isServerLaunch(const InputOptions &opts);


int main(int argc, char ** argv) {
    InputOptions opts = parseInput(argc, argv);

    if (isHelpPrint(opts, argv[0])) 
        return 0;

    if (isServerLaunch(opts))
        return 0;

    std::string input = getInputOrErrorExit(opts);

    // MAIN ENTRY POINT

    while (true) {
        Digraph digraph;
        digraph.isExplain = opts.isExplain;

        try {
            std::vector<Token> tokens = tokenizer(input);
            auto [rules, facts, queries] = parseTokens(tokens);
            digraph = makeDigraph(facts, rules);
            if (!opts.isOpenWorldAssumption) {
                digraph.applyClosedWorldAssumption();
            }

            auto [conclusion, explanation, isError] = digraph.solveEverythingNoThrow(queries);

            if (opts.isDot) {
                std::cout << digraph.toDot();
                break;
            } else if(opts.isExplain) {
                std::cout << "CONCLUSION\n"  << conclusion << std::endl
                          << "EXPLANATION\n" << explanation << std::endl;
            } else {
                std::cout << conclusion;
            }

            if (isError) {
                return 1;
            }
        } catch (std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
        if (opts.isInteractive) {
            std::cout << "Do you want to change the facts and re-evaluate? (y/n): ";
            std::string answer;
            std::getline(std::cin, answer);
            if (answer != "y" && answer != "Y") {
                break;
            } else {
                input = getNewFactsLineFromUser(input);
            }
        } else {
            break;
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
        else if (s == "--server" || s == "-s")
            res.isServer = true;
        else if (s == "--explain" || s == "-e")
            res.isExplain = true;
        else if (s == "--dot" || s == "-d")
            res.isDot = true;
        else if (s == "--interactive" || s == "-i")
            res.isInteractive = true;
        else if (s == "--openWorldAssumption")
            res.isOpenWorldAssumption = true;
        else if (s.starts_with("--port="))
            res.port = std::stoi(s.substr(7));
        else
            res.file = av[i];
    }
    return res;
}


std::string getNewFactsLineFromUser(std::string input) {
    std::cout << "Enter new facts line (e.g., '=AB'): ";
    std::string newFactsLine;
    std::getline(std::cin, newFactsLine);
    // Replace the facts line in the input
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


std::string getFileInput(char *fileName) {
    std::ifstream file(fileName);
    if (!file) throw std::runtime_error("Cannot open file \"" 
            + std::string(fileName) + "\"");
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
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


std::string getInputOrErrorExit(const InputOptions &opts) {
    try {
        return opts.file == nullptr ? getStdInput() : getFileInput(opts.file);
    } catch (std::exception &e) {
        std::cerr << "Startup error | " << e.what() << std::endl;
        exit(1);
    }
}


bool isHelpPrint(const InputOptions &opts , char *argv0) {
    if (!opts.isHelp)
        return false;

    std::cout << "Expert System .. in propositional calculation."
    << std::endl
    << std::endl << "Usage: " << argv0 << " [options] propositional_statements.txt"
    << std::endl
    << std::endl << "Options:"
    << std::endl << "  -h, --help                 Show this help message and exit"
    << std::endl << "  -s, --server               Launch a webserver for an interactive interface"
    << std::endl << "      --port=NUMBER          Specify port for server mode (default: 7711)"
    << std::endl << "  -e, --explain              Print explanation of the reasoning process"
    << std::endl << "  -d, --dot                  Output reasoning as a Graphviz DOT file"
    << std::endl << "  -i, --interactive          Enable interactive mode (modify facts after evaluation)"
    << std::endl << "      --openWorldAssumption  Use Open World Assumption (default is Closed World)"
    << std::endl;

    return true;
}

bool isServerLaunch(const InputOptions &opts) {
    if (!opts.isServer)
        return false;
    WebServer webServer(opts);
    webServer.start(); // blocks on the server
    return true;
}
