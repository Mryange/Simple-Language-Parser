
#pragma once
#include <cctype>
#include <vector>

#include "Type.h"
#include "util.h"
struct Token {
    constexpr static std::string_view key_words[] = {
            "if", "while", "for", "def", "return", "struct", "extends", "else", "foreach"};

    constexpr static std::string_view brackets[] = {"(", "{", "}", ")", ",", ";", "[", "]", ":"};

    constexpr static std::string_view ops[] = {"+",  "-", "*", "/",  "==", "!=", "=",   ".",  "<=",
                                               ">=", "<", ">", "&&", "||", "&",  "arr", "let"};

    // copy from https://learn.microsoft.com/zh-cn/cpp/cpp/cpp-built-in-operators-precedence-and-associativity?view=msvc-170
    constexpr static int prioritys[] = {-6, -6, -5, -5,  -9,  -9, -15, -2, -8,
                                        -8, -8, -8, -13, -14, -3, -3,  -3};
    static_assert(sizeof(ops) / sizeof(std::string_view) == sizeof(prioritys) / sizeof(int));
    enum Type {
        key_word = 0, // if while for def (  {
        variable,
        constant,
        bracket,
        op, // + - * /

    };

    static std::string token_type_string(Type type) {
        switch (type) {
        case key_word:
            return "key_word";
        case variable:
            return "variable";
        case constant:
            return "constant";
        case bracket:
            return "bracket";
        case op:
            return "op";
        default:
            return "error token type";
        }
    }

    Token(const std::string& s, Token::Type type) : _str(s), _type(type) {}
    Token(const Token& rhs) = default;
    Token() {}
    Type type() const { return _type; }
    std::string str() const { return _str; }

    int priority() const {
        int idx = 0;
        for (auto op : ops) {
            if (_str == op) return prioritys[idx];
            idx++;
        }
        if (_str == "[") return -2;
        CHECK(0, "should not reach here.");
        return -20;
    }

private:
    std::string _str;
    Type _type;
};

inline static auto token_from_str_map = [](

                                        ) {
    std::map<std::string, Token> map;
    for (auto& s : Token::key_words) {
        map.insert({std::string {s}, Token(std::string {s}, Token::Type::key_word)});
    }
    for (auto& s : Token::ops) {
        map.insert({std::string {s}, Token(std::string {s}, Token::Type::op)});
    }
    for (auto& s : Token::brackets) {
        map.insert({std::string {s}, Token(std::string {s}, Token::Type::bracket)});
    }
    return map;
}();

inline bool operator==(const Token& l, const Token& r) {
    return l.type() == r.type() && l.str() == r.str();
}

struct TokenStream {
    TokenStream(const std::vector<Token> tokens) : _tokens(tokens) {}

    Token get() {
        CHECK(_cur_pos < _tokens.size(), "out of bound");
        return _tokens[_cur_pos++];
    }
    Token top() {
        CHECK(_cur_pos < _tokens.size(), "out of bound");
        return _tokens[_cur_pos];
    }
    bool end() { return _cur_pos == _tokens.size(); }
    Token eat(const std::string& s) {
        Token token = token_from_str_map[s];
        return eat(token);
    }

    Token eat(const Token token) {
        auto cur_token = get();
        CHECK(token == cur_token,
              "eat token failed want eat  " + token.str() + " but now is " + cur_token.str());
        return cur_token;
    }

    Token eat(Token::Type type) {
        auto cur_token = get();
        CHECK(cur_token.type() == type, "eat token failed want eat type" +
                                                Token::token_type_string(type) + " but now is " +
                                                Token::token_type_string(cur_token.type()));
        return cur_token;
    }

    bool top_equal(const std::string s) {
        Token token = token_from_str_map[s];
        return token == top();
    }
    bool top_equal(Token::Type type) { return type == top().type(); }

    auto cur_pos() const { return _cur_pos; }

    std::string get_token_string(size_t begin, size_t end) {
        std::string name;
        for (int i = begin; i < end; i++) {
            name += _tokens[i].str();
        }
        return name;
    }

private:
    std::vector<Token> _tokens;
    size_t _cur_pos = 0;
};

struct Tokenizer {
    static auto isvariable(char c) { return std::isalpha(c) || std::isdigit(c); }

    static TokenStream get_token_stream(std::string text) {
        std::vector<Token> tokens;
        size_t pos = 0;
        auto is_key_words = [&]() -> std::optional<std::string> {
            for (auto& key_word : Token::key_words) {
                if (text.substr(pos, key_word.size()) == key_word &&
                    !isvariable(text[pos + key_word.size()])) {
                    pos += key_word.size();
                    return std::string(key_word);
                }
            }
            return std::nullopt;
        };

        auto is_op_words = [&]() -> std::optional<std::string> {
            for (auto& op : Token::ops) {
                if (text.substr(pos, op.size()) == op) {
                    pos += op.size();
                    return std::string(op);
                }
            }
            return std::nullopt;
        };

        auto is_bracket_word = [&]() -> std::optional<std::string> {
            for (auto& bracket : Token::brackets) {
                if (text.substr(pos, bracket.size()) == bracket) {
                    pos += bracket.size();
                    return std::string(bracket);
                }
            }
            return std::nullopt;
        };
        while (pos < text.size()) {
            if (std::isspace(text[pos])) {
                pos++;
            } else if (auto key_word = is_key_words()) {
                tokens.push_back(Token(*key_word, Token::Type::key_word));
            } else if (auto op = is_op_words()) {
                tokens.push_back(Token(*op, Token::Type::op));
            } else if (auto bracket = is_bracket_word()) {
                tokens.push_back(Token(*bracket, Token::Type::bracket));
            } else {
                if (std::isalpha(text[pos])) {
                    // variable
                    std::string variable;
                    while (isvariable(text[pos])) {
                        variable.push_back(text[pos++]);
                    }
                    tokens.push_back(Token {variable, Token::Type::variable});
                } else {
                    // constants
                    if (text[pos] == '\"') {
                        // string
                        std::string str = "\"";
                        pos++;
                        while (text[pos] != '\"') {
                            str.push_back(text[pos++]);
                        }
                        pos++;
                        tokens.push_back(Token {str, Token::Type::constant});
                    } else {
                        std::string constant;
                        while (std::isalpha(text[pos]) || std::isdigit(text[pos]) ||
                               text[pos] == '.' || text[pos] == '-') {
                            constant.push_back(text[pos++]);
                        }
                        tokens.push_back(Token {constant, Token::Type::constant});
                    }
                }
            }
        }
        return tokens;
    }
};