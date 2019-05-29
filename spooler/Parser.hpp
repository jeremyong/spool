#pragma once

#include <string>
#include <string_view>
#include <vector>

class Parser
{
public:
    Parser(const char* contents, size_t size, const char* macro_name);

    // Scan the contents looking for occurrences of [MACRO_NAME]("literal")
    // Handles the following edge cases:
    // - Escaped characters
    // - Occurrences of the macro in quoted text
    // - Whitespace between macro name and leading paren
    // - Whitespace between macro arguments and trailing paren
    // - Coalescing of adjacent C string literals
    // - Escaping within C string literals
    void parse();

    [[nodiscard]] const std::vector<std::string>& literals() const noexcept
    {
        return literals_;
    }

    // For debugging purposes, print the literals parsed to stdout
    void print();

private:
    std::string_view contents_;
    std::vector<std::string> literals_;
    const char* macro_name_;
};
