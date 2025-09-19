#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "expert-system.hpp"
#include "server.hpp"

struct InputOptions {
    bool isHelp = false;
    char *file = nullptr;
    bool isServer = false;
};


std::string getStdInput();
std::string getFileInput(char *fileName);
InputOptions parseInput(int ac, char **av);

bool isHelpPrint(const InputOptions &opts, char *argv0);
std::string getInputOrErrorExit(const InputOptions &opts);
bool isServerLaunch(const InputOptions &opts);

int main(int argc, char ** argv) {
    InputOptions opts = parseInput(argc, argv);

    if (isHelpPrint(opts, argv[0])) 
        return 0;
    
    if (isServerLaunch(opts))
        return 0;

    std::string input = getInputOrErrorExit(opts);

    // MAIN ENTRY POINT

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
    launchGraphvisServer();
    return true;
}