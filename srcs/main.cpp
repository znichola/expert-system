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
    Digraph digraph;
    bool userWantToChangeFacts = true;
    while (userWantToChangeFacts) {
        try {
            std::vector<Token> tokens = tokenizer(input);
            auto [rules, facts, queries] = parseTokens(tokens);
            digraph = makeDigraph(facts, rules);

            // Iterative solving with Closed World Assumption
            bool changed = true;
            int max_iterations = 1; // Prevent infinite loops
            int iteration = 0;

            while (changed && iteration < max_iterations) {
                changed = false;
                iteration++;
                
                if (opts.isExplain) {
                    std::cout << "=== Iteration " << iteration << " ===" << std::endl;
                }
                
                // First: Apply Closed World Assumption to facts with no consequent rules
                for (auto &[fact_id, fact] : digraph.facts) {
                    if (fact.state == Fact::State::Undetermined && fact.consequent_rules.empty()) {
                        if (opts.isExplain) {
                            std::cout << "Applying Closed World Assumption: " << fact_id << " = False (no rules can prove it)" << std::endl;
                        }
                        fact.state = Fact::State::False;
                        changed = true;
                    }
                }

                // Second: Try to solve queries with current facts
                for (const auto &query : queries) {
                    auto it = digraph.facts.find(query.label);
                    if (it != digraph.facts.end()) {
                        auto old_state = it->second.state;
                        digraph.solveForFact(query.label, opts.isExplain);
                        if (it->second.state != old_state) {
                            changed = true;
                        }
                    } else {
                        // Query fact doesn't exist in digraph - this shouldn't happen
                        // but handle it gracefully
                        if (opts.isExplain) {
                            std::cout << "Warning: Query fact " << query.label << " not found in digraph" << std::endl;
                        }
                    }
                }
                
                // Third: Apply CWA to remaining undetermined facts that couldn't be proven
                for (auto &[fact_id, fact] : digraph.facts) {
                    if (fact.state == Fact::State::Undetermined) {
                        // Check if this fact is not in an ambiguous conclusion
                        if (!digraph.isFactInAmbiguousConclusion(fact_id)) {
                            if (opts.isExplain) {
                                std::cout << "Applying Closed World Assumption: " << fact_id << " = False (couldn't be proven)" << std::endl;
                            }
                            fact.state = Fact::State::False;
                            changed = true;
                        }
                    }
                }
            }
            // Display final results
            for (const auto &query : queries) {
                auto it = digraph.facts.find(query.label);
                if (it != digraph.facts.end()) {
                    std::cout << query.label << " is " << it->second.state << std::endl;
                } else {
                    std::cout << query.label << " is False" << std::endl;
                }
            }
        } catch (std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
            // TODO remove this print
            // Print all facts
            // for (const auto &[fact_id, fact] : digraph.facts) {
            //     std::cout << fact_id << " is " << fact.state << std::endl;
            // }
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
    launchGraphvisServer();
    return true;
}
