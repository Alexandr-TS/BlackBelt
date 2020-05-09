#include "lexer.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <charconv>
#include <unordered_map>

using namespace std;

namespace Parse {

    bool operator == (const Token& lhs, const Token& rhs) {
        using namespace TokenType;

        if (lhs.index() != rhs.index()) {
            return false;
        }
        if (lhs.Is<Char>()) {
            return lhs.As<Char>().value == rhs.As<Char>().value;
        }
        else if (lhs.Is<Number>()) {
            return lhs.As<Number>().value == rhs.As<Number>().value;
        }
        else if (lhs.Is<String>()) {
            return lhs.As<String>().value == rhs.As<String>().value;
        }
        else if (lhs.Is<Id>()) {
            return lhs.As<Id>().value == rhs.As<Id>().value;
        }
        else {
            return true;
        }
    }

    std::ostream& operator << (std::ostream& os, const Token& rhs) {
        using namespace TokenType;

#define VALUED_OUTPUT(type) \
  if (auto p = rhs.TryAs<type>()) return os << #type << '{' << p->value << '}';

        VALUED_OUTPUT(Number);
        VALUED_OUTPUT(Id);
        VALUED_OUTPUT(String);
        VALUED_OUTPUT(Char);

#undef VALUED_OUTPUT

#define UNVALUED_OUTPUT(type) \
    if (rhs.Is<type>()) return os << #type;

        UNVALUED_OUTPUT(Class);
        UNVALUED_OUTPUT(Return);
        UNVALUED_OUTPUT(If);
        UNVALUED_OUTPUT(Else);
        UNVALUED_OUTPUT(Def);
        UNVALUED_OUTPUT(Newline);
        UNVALUED_OUTPUT(Print);
        UNVALUED_OUTPUT(Indent);
        UNVALUED_OUTPUT(Dedent);
        UNVALUED_OUTPUT(And);
        UNVALUED_OUTPUT(Or);
        UNVALUED_OUTPUT(Not);
        UNVALUED_OUTPUT(Eq);
        UNVALUED_OUTPUT(NotEq);
        UNVALUED_OUTPUT(LessOrEq);
        UNVALUED_OUTPUT(GreaterOrEq);
        UNVALUED_OUTPUT(None);
        UNVALUED_OUTPUT(True);
        UNVALUED_OUTPUT(False);
        UNVALUED_OUTPUT(Eof);

#undef UNVALUED_OUTPUT

        return os << "Unknown token :(";
    }


    Lexer::Lexer(std::istream& input)
        : is(input)
    {
        token = NextToken();
    }

    const Token& Lexer::CurrentToken() const {
        return token;
    }

    Token Lexer::NextToken() {
        if (balance_indent > 0) {
            --balance_indent;
            return token = TokenType::Indent{};
        }
        else if (balance_indent < 0) {
            ++balance_indent;
            return token = TokenType::Dedent{};
        }

        char next_ch_tmp = is.peek();
        if (next_ch_tmp != ' ' && next_ch_tmp != '\n') {
            if (is_new_line) {
                if (last_indent) {
                    balance_indent = -last_indent;
                    last_indent = 0;
                    return token = NextToken();
                }
            }
        }


        char ch = EOF;
        if (!input_ended) {
            is.get(ch);
        }

        // EOF
        if (ch == EOF) {
            if (!is_new_line) {
                is_new_line = true;
                return token = TokenType::Newline{};
            }
            input_ended = true;
            return token = TokenType::Eof{};
        }

        // space
        if (ch == ' ') {
            int cnt_spaces = 1;
            while (is.peek() == ' ') {
                is.get(ch);
                ++cnt_spaces;
            }
            int new_indent = cnt_spaces / 2;

            if (is_new_line) {
                balance_indent = new_indent - last_indent;
                last_indent = new_indent;
                is_new_line = false;
            }
            return token = NextToken();
        }

        // newline
        if (ch == '\n') {
            while (is.peek() == '\n') {
                is.get(ch);
            }
            if (is_new_line) {
                return token = NextToken();
            }
            is_new_line = true;
            return token = TokenType::Newline{};
        }
        else {
            is_new_line = false;
        }

        // number
        if (isdigit(ch)) {
            int number = static_cast<int>(ch - '0');
            while (isdigit(is.peek())) {
                is.get(ch);
                number = 10 * number + static_cast<int>(ch - '0');
            }
            return token = TokenType::Number{ number };
        }

        // chars
        if (ch == '=' || ch == '<' || ch == '>' || ch == '!') {
            if (is.peek() == '=') {
                string s;
                s += ch;
                is.get(ch);
                s += ch;
                if (s == "==") return token = TokenType::Eq{};
                else if (s == "<=") return token = TokenType::LessOrEq{};
                else if (s == ">=") return token = TokenType::GreaterOrEq{};
                else if (s == "!=") return token = TokenType::NotEq{};
                else assert(false);
            }
            else {
                return token = TokenType::Char{ ch };
            }
        }

        // strings
        if (ch == '\'' || ch == '\"') {
            string str;
            char new_ch;
            while (true) {
                is.get(new_ch);
                if (new_ch == ch) {
                    break;
                }
                str += new_ch;
            }
            return token = TokenType::String{ str };
        }

        // Id or key word
        if (isalpha(ch) || ch == '_') {
            string str;
            str += ch;
            while (isalnum(is.peek()) || is.peek() == '_') {
                is.get(ch);
                str += ch;
            }
            if (str == "class") return token = TokenType::Class{};
            if (str == "return") return token = TokenType::Return{};
            if (str == "if") return token = TokenType::If{};
            if (str == "else") return token = TokenType::Else{};
            if (str == "def") return token = TokenType::Def{};
            if (str == "print") return token = TokenType::Print{};
            if (str == "and") return token = TokenType::And{};
            if (str == "or") return token = TokenType::Or{};
            if (str == "not") return token = TokenType::Not{};
            if (str == "None") return token = TokenType::None{};
            if (str == "True") return token = TokenType::True{};
            if (str == "False") return token = TokenType::False{};
            return token = TokenType::Id{ str };
        }

        // char
        return token = TokenType::Char{ ch };
    }

} /* namespace Parse */
