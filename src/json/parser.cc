#include <print>

#include "parser.hh"

namespace json
{

Parser::Parser(std::string_view path)
    : m_sName(path), m_l(path)
{
    m_tCurr = m_l.next();
    m_tNext = m_l.next();

    if ((m_tCurr.type != Token::LBRACE) && (m_tCurr.type != Token::LBRACKET))
    {
        std::print(stderr, "wrong first token\n");
        exit(2);
    }

    m_upHead = std::make_unique<Node>();
}

void
Parser::parse()
{
    parseNode(m_upHead.get());
}

void
Parser::expect(enum Token::TYPE t)
{
    if (m_tCurr.type != t)
    {

        std::print(stderr, "unexpected token\n");
        exit(2);
    }
}

void
Parser::next()
{
    m_tCurr = m_tNext;
    m_tNext = m_l.next();
}

void
Parser::parseNode(Node* pNode)
{
    switch (m_tCurr.type)
    {
        default:
            next();
            break;

        case Token::IDENT:
            parseIdent(&pNode->tagVal);
            break;

        case Token::NUMBER:
            parseNumber(&pNode->tagVal);
            break;

        case Token::LBRACE:
            next(); /* skip brace */
            parseObject(pNode);
            break;

        case Token::LBRACKET:
            next(); /* skip bracket */
            parseArray(pNode);
            break;

        case Token::NULL_:
            parseNull(&pNode->tagVal);
            break;

        case Token::TRUE:
        case Token::FALSE:
            parseBool(&pNode->tagVal);
            break;
    }
}

void
Parser::parseIdent(TagVal* pTV)
{
    *pTV = {TAG::STRING, m_tCurr.svLiteral};
    next();
}

void
Parser::parseNumber(TagVal* pTV)
{
    bool bReal = m_tCurr.svLiteral.find('.') != std::string::npos;

    if (bReal)
        *pTV = {.tag = TAG::FLOAT, .val = std::atof(m_tCurr.svLiteral.data())};
    else
        *pTV = {.tag = TAG::INT, .val = std::atoll(m_tCurr.svLiteral.data())};

    next();
}

void
Parser::parseObject(Node* pNode)
{
    pNode->tagVal.tag = TAG::OBJECT;
    pNode->tagVal.val = std::vector<Node>{};
    auto& aObjs = std::get<std::vector<Node>>(pNode->tagVal.val);

    for (; m_tCurr.type != Token::RBRACE; next())
    {
        expect(Token::IDENT);
        aObjs.push_back({.svKey = m_tCurr.svLiteral, .tagVal = {}});

        /* skip identifier and ':' */
        next();
        expect(Token::ASSIGN);
        next();

        parseNode(&aObjs.back());

        if (m_tCurr.type != Token::COMMA)
        {
            next();
            break;
        }
    }

    if (aObjs.empty())
        next();
}

void
Parser::parseArray(Node* pNode)
{
    pNode->tagVal.tag = TAG::ARRAY;
    pNode->tagVal.val = std::vector<TagVal>{};
    auto& aTVs = std::get<std::vector<TagVal>>(pNode->tagVal.val);

    /* collect each key/value pair inside array */
    for (; m_tCurr.type != Token::RBRACKET; next())
    {
        aTVs.push_back({});

        switch (m_tCurr.type)
        {
            default:
            case Token::IDENT:
                parseIdent(&aTVs.back());
                break;

            case Token::NULL_:
                parseNull(&aTVs.back());
                break;

            case Token::TRUE:
            case Token::FALSE:
                parseBool(&aTVs.back());
                break;

            case Token::NUMBER:
                parseNumber(&aTVs.back());
                break;

            case Token::LBRACE:
                next();
                aTVs.back().tag = TAG::OBJECT;
                aTVs.back().val = std::vector<Node>(1);
                auto& obj = std::get<std::vector<Node>>(aTVs.back().val).back();
                parseObject(&obj);
                break;
        }

        if (m_tCurr.type != Token::COMMA)
        {
            next();
            break;
        }
    }

    if (aTVs.empty())
        next();
}

void
Parser::parseNull(TagVal* pTV)
{
    *pTV = {.tag = TAG::NULL_, .val = nullptr};
    next();
}

void
Parser::parseBool(TagVal* pTV)
{
    bool b = m_tCurr.type == Token::TRUE? true : false;
    *pTV = {.tag = TAG::BOOL, .val = b};
    next();
}

void
Parser::print()
{
    printNode(m_upHead.get(), "");
    std::print("\n");
}

void
Parser::printNode(Node* pNode, std::string_view svEnd)
{
    std::string_view key = pNode->svKey;

    switch (pNode->tagVal.tag)
    {
        default:
            break;

        case TAG::OBJECT:
            {
                auto& obj = std::get<std::vector<Node>>(pNode->tagVal.val);
                std::string q0, q1, objName0, objName1;

                if (key.size() == 0)
                {
                    q0 = q1 = objName1 = objName0 = "";
                }
                else
                {
                    objName0 = key;
                    objName1 = ": ";
                    q1 = q0 = "\"";
                }

                std::print("{}{}{}{}{{\n", q0, objName0, q1, objName1);
                for (size_t i = 0; i < obj.size(); i++)
                {
                    std::string slE = (i == obj.size() - 1) ? "\n" : ",\n";
                    printNode(&obj[i], slE);
                }
                std::print("}}{}", svEnd);
            }
            break;

        case TAG::ARRAY:
            {
                auto& arr = std::get<std::vector<TagVal>>(pNode->tagVal.val);
                std::string q0, q1, arrName0, arrName1;

                if (key.size() == 0)
                {
                    q0 =  q1 = arrName1 = arrName0 = "";
                }
                else
                {
                    arrName0 = key;
                    arrName1 = ": ";
                    q1 = q0 = "\"";
                }

                std::print("{}{}{}{}[", q0, arrName0, q1, arrName1);
                for (size_t i = 0; i < arr.size(); i++)
                {
                    std::string slE = (i == arr.size() - 1) ? "\n" : ",\n";

                    switch (arr[i].tag)
                    {
                        default:
                        case TAG::STRING:
                            {
                                std::string_view sl = std::get<std::string_view>(arr[i].val);
                                std::print("\"{}\"{}", sl, slE);
                            }
                            break;

                        case TAG::NULL_:
                                std::print("{}{}", "null", slE);
                            break;

                        case TAG::INT:
                            {
                                long num = std::get<long>(arr[i].val);
                                std::print("{}{}", num, slE);
                            }
                            break;

                        case TAG::FLOAT:
                            {
                                double dnum = std::get<double>(arr[i].val);
                                std::print("{}{}", dnum, slE);
                            }
                            break;

                        case TAG::BOOL:
                            {
                                bool b = std::get<bool>(arr[i].val);
                                std::print("{}{}", b, slE);
                            }
                            break;

                        case TAG::OBJECT:
                                printNode(&std::get<std::vector<Node>>(arr[i].val).back(), slE);
                            break;
                    }
                }
                std::print("]{}", svEnd);
            }
            break;

        case TAG::FLOAT:
            {
                /* TODO: add some sort formatting for floats */
                double f = std::get<double>(pNode->tagVal.val);
                std::print("\"{}\": {}{}", key, f, svEnd);
            }
            break;

        case TAG::INT:
            {
                long i = std::get<long>(pNode->tagVal.val);
                std::print("\"{}\": {}{}", key, i, svEnd);
            }
            break;

        case TAG::NULL_:
                std::print("\"{}\": {}{}", key, "null", svEnd);
            break;

        case TAG::STRING:
            {
                std::string_view sl = std::get<std::string_view>(pNode->tagVal.val);
                std::print("\"{}\": \"{}\"{}", key, sl, svEnd);
            }
            break;

        case TAG::BOOL:
            {
                bool b = std::get<bool>(pNode->tagVal.val);
                std::print("\"{}\": {}{}", key, b, svEnd);
            }
            break;
    }
}

} /* namespace json */
