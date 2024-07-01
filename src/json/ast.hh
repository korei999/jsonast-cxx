#pragma once

#include <string_view>
#include <variant>
#include <vector>

namespace json
{

enum class TAG
{
    NULL_,
    STRING,
    INT,
    FLOAT,
    ARRAY,
    OBJECT,
    BOOL
};

struct Node;

struct TagVal
{
    enum TAG tag;
    std::variant<std::string_view,
                 long,
                 double,
                 std::vector<TagVal>,
                 std::vector<Node>,
                 bool,
                 std::nullptr_t> val;
};

struct Node
{
    std::string_view svKey;
    TagVal tagVal;
};

} /* namespace json */
