#pragma once
#include <string>
#include <optional>

struct argument
{
    /*
     * if _key is empty and _value is empty => invalid argument.
     * if _key is not empty and _value is empty => option without value (e.g. -r).
     * if _key is not empty and _value is not empty => option with value (e.g. -f filename).
     * if _key is empty and _value is not empty => actual argument (e.g. "cp hajox valodik/" hajox and valodik are actual arguments).
     */
    std::string _key;
    std::optional<std::string> _value;
    
    argument(std::string key, std::optional<std::string> value)
    : _key(std::move(key))
    , _value(std::move(value))
    {}
};
