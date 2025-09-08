#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "expert-system.hpp"

struct InputOptions {
    bool isHelp = false;
    char *file = nullptr;
};


std::string getStdInput();
std::string getFileInput(char *fileName);
InputOptions parseInput(int ac, char **av);


int main(int argc, char ** argv) {
    InputOptions opts = parseInput(argc, argv);

    if (opts.isHelp) {
        std::cout << "Expert System .. in propositional calculation."
        << std::endl
        << std::endl << "run with " << argv[0] << " propositional_statments.txt"
        << std::endl
        << std::endl << "flags -h / --help    to get this help message"
        << std::endl;
        return 0;
    }

    std::string input;

    try {
        input = opts.file == nullptr ? getStdInput() : getFileInput(opts.file);
    } catch (std::exception &e) {
        std::cerr << "Startup error | " << e.what() << std::endl;
        return 1;
    }

    // MAIN ENTRY POINT



    // can throw on invalid token
    // tokenize(input) -> token[]

    // A+B<=>H # this is OK

    // G => f  # tokenizer error

    // G => H | H & B # parsing error

    // G | ( H => L ) # parsing error

    // G | L # also parsing error


    // can throw on invalid syntax (eg. see above)
    // parseRules(token[]) -> {rules[], errors[]}
    // parseFacts(token[]) -> facts[]
    // parseQueries(token[]) -> queries[]

    // can throw on contracdicting rules
    // solver(everyting) -> solved variables & written explication

    // To solve:
    // 1) check if value is solved
    // 2) if not, find a rule that solves this value
    // 3) check if this rule is solved?
    // 4) if yes, solve it and return
    // 5) if no, add the rules values to the "to solve" list

    // SOLVER

    // this is a funciton that takes as input the list of rules, 
    // the goals for this solving, and the list of facts
    // this will return the list of solved goals, and the rules used to solve this

    // solve(fact[], rules[], reasoning[], query) -> (reasoning[], fact[])
    
    



    return 0;
}


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
