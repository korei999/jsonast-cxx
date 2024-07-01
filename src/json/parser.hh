#pragma once
#include <memory>

#include "lex.hh"
#include "ast.hh"

namespace json
{

struct Parser
{
    const std::string m_sName;
    std::unique_ptr<Node> m_upHead;

    Parser(std::string_view path);
    void parse();
    void print();
    void printNode(Node* pNode, std::string_view svEnd);

private:
    Lexer m_l;
    Token m_tCurr;
    Token m_tNext;

    void expect(enum Token::TYPE t);
    void next();
    void parseNode(Node* pNode);
    void parseIdent(TagVal* pTV);
    void parseNumber(TagVal* pTV);
    void parseObject(Node* pNode);
    void parseArray(Node* pNode);
    void parseNull(TagVal* pTV);
    void parseBool(TagVal* pTV);
};

} /* namespace json */
