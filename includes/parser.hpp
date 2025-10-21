#include "expert-system.hpp"


std::tuple<vector<Rule>, vector<Fact>, vector<Query>>
    parseTokens(const vector<Token> &input);


struct Parser {
    size_t index;
    vector<Token> tokens; 
    std::optional<Token> current(){
        if (index < tokens.size())
            return tokens[index];
        return {};
    }

    Expr parse() {
        auto expr = parseExpr();
        if (index != tokens.size()) {
            throw std::runtime_error("Unexpected tokens remaining after parsing");
        }
        if (!expr) {
            throw std::runtime_error("Failed to parse expression");
        }
        return expr.value();
    }
    

    /* createNode implementation
    ** ----------------------------
    ** This function creates a new expression node based on the given token and its operands.
    ** It handles different types of binary and unary operators.
    ** It returns an Expr representing the new node.
    ** It throws an error message and returns an Empty expression for unknown operators.
    */
    Expr createNode(const Token &token, const Expr &lhs, const Expr &rhs)
    {
        switch (token.token_list[0]){
            case '+': return And(lhs, rhs);
            case '|': return Or(lhs, rhs);
            case '^': return Xor(lhs, rhs);
            case '=': return Imply(lhs, rhs);
            case '<': return Iff(lhs, rhs);
            case '!': return Not(lhs);
            default:  break;
        }
        throw std::runtime_error("Parse error: unknown operator '" + token.token_list + "'");
    }

    enum class Assoc { LEFT, RIGHT };
    
    /**
     * Get precedence for operators (higher number = tighter binding)
     */
    std::pair<int, Assoc> getPrec(const Token &token) const
    {
        switch (token.token_list[0]){
            case '<': return {1, Assoc::RIGHT };
            case '=': return {2, Assoc::RIGHT };
            case '^': return {3, Assoc::LEFT };
            case '|': return {4, Assoc::LEFT };
            case '+': return {5, Assoc::LEFT };
            case '!': return {6, Assoc::LEFT };
            default:  break;
        }
        throw std::runtime_error("Parse error: unknown precedence for '" + token.token_list + "'");
    }
    
    /*
    ** parseFactor implementation
    ** ----------------------------
    ** This function parses a factor from the list of tokens.
    ** A factor can be a variable or a parenthesized expression.
    ** It returns an optional Expr, which is empty if parsing fails.
    ** It throws exceptions for syntax errors.
    */
    std::optional<Expr> parseFactor(){
        auto tok = current();
        if (!tok) return {};

        // Handle unary NOT
        if (tok->token_list == "!") {
            index++;
            auto operand = parseFactor();
            if (!operand) throw std::runtime_error("Expected factor after '!'");
            return Not(*operand);
        }

        if (tok->type == Token::Type::Variable ) {
            index++;
            return Var(tok->token_list[0]);
        }
        else if (tok->token_list == "(") {
            index++;
            auto expr = parseExpr();
            
            if (current() && current()->token_list == ")") {
                index++;
                return expr;
            } else {
                throw std::runtime_error("Expected closing parenthesis");
            }
        }
        throw std::runtime_error("Expected token: " + tok->token_list);
    }

    /*
    ** parseExpr implementation
    ** ----------------------------
    ** This function parses an expression from the list of tokens.
    ** It handles binary operators and respects operator precedence.
    ** It returns an optional Expr, which is empty if parsing fails.
    ** It throws exceptions for syntax errors.
    */
    std::optional<Expr> parseExpr(int min_prec = 0) {
        auto lhs = parseFactor();
        while (lhs && current() && current()->type == Token::Type::Operator) {
            auto op = *current();
            const auto [prec, assoc] = getPrec(op);
            if (prec < min_prec)
                break;
            index++;
            int next_min = (assoc == Assoc::LEFT) ? prec + 1 : prec;
            auto rhs = parseExpr(next_min);
            if (!rhs) throw std::runtime_error("Expected factor after operator: " + op.token_list);
            lhs = createNode(op, *lhs, *rhs);
        }
        
        return lhs;
    }
};

