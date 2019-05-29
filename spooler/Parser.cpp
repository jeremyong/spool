#include "Parser.hpp"

#include <cassert>
#include <cctype>
#include <iostream>
#include <stdexcept>


Parser::Parser(const char* contents, size_t size, const char* macro_name)
    : contents_{contents, size}
    , macro_name_{macro_name}
{
    assert(contents[0] == '\0' && "contents must be null-padded at the start");
}

void Parser::parse()
{
    bool in_macro = false;
    bool in_quote = false;

    size_t i = 1;
    std::string literal;

    // This is not the way I'd build a parser in general, but is quite fast and suitable for the relatively
    // simple parsing grammar we need to accommodate (quoted strings in a user-defined macro, accounting for
    // quote and escape sequences).
    while (i < contents_.size())
    {
        char c = contents_[i];
        if (c == '\\')
        {
            if (in_macro && in_quote)
            {
                literal += c;
                if (i + 1 >= contents_.size())
                {
                    throw std::runtime_error("Encountered backslash at end of file");
                }
                literal += contents_[i + 1];
            }
            // Handle all backslashed escaped characters (note that this conveniently handles escaped backslashes)
            i += 2;
            continue;
        }

        if (c == '"')
        {
            in_quote = !in_quote;
            ++i;
            continue;
        }

        if (in_quote)
        {
            if (in_macro)
            {
                literal += c;
            }
            ++i;
            continue;
        }

        if (in_macro)
        {
            if (isspace(c))
            {
                ++i;
                continue;
            }
            else if (c == ')')
            {
                literals_.push_back(literal);
                in_macro = false;
                literal.clear();
            }
            else
            {
                throw std::runtime_error(std::string("Unsupported token ") + c + " found in spool macro");
            }
        }
        else
        {
            char last = contents_[i - 1];
            if (isalnum(last))
            {
                ++i;
                continue;
            }

            const char* macro_cursor = macro_name_;
            bool macro_found = false;

            // We're scanning ahead for the occurence of the macro string
            for (; i < contents_.size(); ++i, ++macro_cursor)
            {
                if (*macro_cursor == '\0')
                {
                    macro_found = true;
                    break;
                }

                if (*macro_cursor != contents_[i])
                {
                    break;
                }
            }

            if (macro_found)
            {
                // Consume whitespace until we find a left parenthesis
                for (; i < contents_.size(); ++i)
                {
                    char c = contents_[i];
                    if (c == '\\')
                    {
                        // Skip escaped characters
                        ++i;
                        continue;
                    }

                    if (!isspace(c))
                    {
                        if (c == '(')
                        {
                            // Macro name and leading parenthesis found, we're in a macro
                            in_macro = true;
                        }
                        break;
                    }
                }
            }
        }
        ++i;
    }
}

void Parser::print()
{
    std::cout << "Literals spooled in order of occurrence: \n";
    for (auto& literal : literals_)
    {
        std::cout << '\t' << literal << '\n';
    }
    std::cout << std::flush;
}
