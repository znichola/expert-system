#include "expert-system.hpp"



struct Parser {
    size_t index;
    vector<Token> tokens; 
    std::optional<Token> current(){
        if (index < tokens.size())
            return tokens[index];
        return {};
    }

    /* createNode implementation
    ** ----------------------------
    ** This function creates a new expression node based on the given token and its operands.
    ** It handles different types of binary and unary operators.
    ** It returns an Expr representing the new node.
    ** It throws an error message and returns an Empty expression for unknown operators.
    */
    Expr    createNode(const Token &token, const Expr &lhs, const Expr &rhs)
    {
        switch (token.token_list[0]){
            case '+':
                return And(lhs, rhs);
                break;
            case '|':
                return Or(lhs, rhs);
                break;
            case '^':
                return Xor(lhs, rhs);
                break;
            case '&':
                return And(lhs, rhs);
            case '=':
                return Imply(lhs, rhs);
                break;
            case '<':
                return Iff(lhs, rhs);
                break;
            case '!':
                return Not(lhs);
                break;
            default:
                break;
        }
        throw std::runtime_error("Parse error: unknown operator '" + token.token_list + "'");
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
    std::optional<Expr> parseExpr() {
        auto lhs = parseFactor();
        while (lhs && current() && current()->type == Token::Type::Operator) {
            auto op = *current();
            index++;
            auto rhs = parseFactor();
            if (!rhs) throw std::runtime_error("Expected factor after operator: " + op.token_list);
            lhs = createNode(op, *lhs, *rhs);
        }
        return lhs;
    }
};

