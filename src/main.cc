#include <print>

#include "json/parser.hh"

int
main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::print("jsonast version: {}\n\n", JSONASTCXX_VERSION);
        std::print("usage: {} <path to json>\n", argv[0]);
        exit(3);
    }

    json::Parser p(argv[1]);

    p.parse();

    if (argc >= 3 && std::string_view(argv[2]) == "-p")
        p.print();
}
