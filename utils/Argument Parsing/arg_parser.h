#pragma once
#include "argument.h"
#include "../logging/logger.h"
#include <vector>
#include <iterator>
#include <cstddef>
#include <getopt.h>

extern char* optarg;
extern int optind, opterr, optopt;

class arg_parser {
private:
    std::vector<argument> _arguments;
    const char *_optstring;
    int _argc;
    char **_argv;

public:

    using iterator = std::vector<argument>::iterator;
    using const_iterator = std::vector<argument>::const_iterator;

    arg_parser(int argc, char **argv, const char *optstring)
            : _argc(argc), _argv(argv), _optstring(optstring) {
	using namespace std::literals::string_literals;
        
	int c = 0;
        while ((c = getopt(argc, argv, optstring)) != -1)
        {
            std::string current_key;
            std::optional<std::string> current_value;
            if (c == '?')
            {
                LOG_WARNING("Unexpected options were specified: -"s + argv[optind - 1]);
            }

            current_key.assign(1, static_cast<char>(c));
            current_value = optarg ? std::make_optional<std::string>(optarg) : std::nullopt;
            _arguments.emplace_back(std::move(current_key), std::move(current_value));
        }
        for (int i = optind; i < _argc; ++i)
        {
            _arguments.emplace_back(""s, _argv[i]);
        }
    }

    iterator begin()
    {
        return _arguments.begin();
    }

    iterator end()
    {
        return _arguments.end();
    }
    
    const_iterator cbegin()
    {
        return _arguments.cbegin();
    }

    const_iterator cend()
    {
        return _arguments.cend();
    }
};
