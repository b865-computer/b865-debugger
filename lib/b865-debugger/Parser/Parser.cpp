#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "Parser.h"

DebuggerData DbgDataParser::parse(std::filesystem::path filename, std::filesystem::path mapFile)
{
    DebuggerData data;
    data.fail = false;
    std::ifstream file(filename.string());
    if (!file.is_open())
    {
        data.fail = true;
        return data;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::istringstream iss(buffer.str());
    std::string linestr;
    while (std::getline(iss, linestr))
    {
        if (linestr.size() < 3)
        {
            continue;
        }
        char type = linestr[0];

        std::vector<Token> tokens = tokenize(linestr.substr(2));

        size_t i = 0;

        switch (type)
        {
        case 'M':
            parseModule(tokens, i, data);
            break;

        case 'F':
            data.addFunc(parseFunction(tokens, i));
            break;

        case 'S':
            data.addSymbol(parseSymbol(tokens, i));
            break;

        case 'T':
            data.addType(parseType(tokens, i));
            break;

        case 'L':
            data.addLinkerRecord(parseLinker(tokens, i));
            break;

        default:
            printf("Unknown type: %c\n", type);
            break;
        }
    }

    if (mapFile != "")
    {
        parseMap(data, mapFile);
    }

    return data;
}

enum ParseState
{
    Header,
    BitWidht,
    Area,
    AreaDef,
    EndPage,
};

void DbgDataParser::parseMap(DebuggerData &data, std::filesystem::path filename)
{
    printf("Parsing map file: %s\n", filename.c_str());
    std::ifstream file(filename.string());
    if (!file.is_open())
    {
        data.fail = true;
        return;
    }

    ParseState state = EndPage;
    std::string area;

    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::istringstream iss(buffer.str());
    std::string linestr;
    while (std::getline(iss, linestr))
    {
        if (linestr.size() < 3)
        {
            continue;
        }
        switch (state)
        {
        case EndPage:
            if (linestr[0] == '\x0C')
            {
                state = Header;
            }
            break;

        case Header:
            if (linestr.size() > 6 && linestr.substr(0, 4) == "Area")
            {
                state = Area;
            }
            break;

        case Area:
            if (linestr[0] == '-')
            {
                // skip separator line
                break;
            }
            else
            {
                area = linestr.substr(0, linestr.find_first_of(" \t"));
                state = AreaDef;
            }
            break;

        case AreaDef:
            if (linestr[0] == '\x0C')
            {
                state = Header;
                break;
            }

            if (area != "CODE")
            {
                // Ignore other areas
                break;
            }

            if (linestr.find_first_not_of(" \t") == std::string::npos)
            {
                break;
            }
            linestr.erase(0, linestr.find_first_not_of(" \t"));

            if (linestr[0] == 'V' || linestr[0] == '-')
            {
                break;
            }

            // Attempt to extract symbols from columns in CODE section
            while (linestr.size() > 3)
            {
                if (linestr.find_first_not_of(" \t") == std::string::npos)
                {
                    break;
                }
                linestr.erase(0, linestr.find_first_not_of(" \t"));
                std::string addressStr, symbol;
                if (linestr.find_first_of(" \t") == std::string::npos)
                {
                    break;
                }
                addressStr = linestr.substr(0, linestr.find_first_of(" \t"));
                linestr.erase(0, linestr.find_first_of(" \t"));
                if (linestr.find_first_not_of(" \t") == std::string::npos)
                {
                    break;
                }
                linestr.erase(0, linestr.find_first_not_of(" \t"));
                if (linestr.find_first_of(" \t") == std::string::npos)
                {
                    break;
                }
                symbol = linestr.substr(0, linestr.find_first_of(" \t"));
                linestr.erase(0, linestr.find_first_of(" \t"));
                if (linestr.find_first_not_of(" \t") == std::string::npos)
                {
                    goto Process_Symbol;
                }
                linestr.erase(0, linestr.find_first_not_of(" \t"));
                if (linestr.size() && linestr[0] == '|')
                {
                    linestr.erase(linestr.begin());
                }
Process_Symbol:
                if (!symbol.empty() && symbol.find('$') == std::string::npos)
                {
                    // Parse hex address
                    uint32_t address = 0;
                    try
                    {
                        address = std::stoul(addressStr, nullptr, 16);
                    }
                    catch (const std::exception &)
                    {
                        // Invalid address string, skip
                        break;
                    }

                    // Add symbol
                    SymbolRecord symbolRec;
                    symbolRec.addressSpace = AddressSpace::CODE;
                    symbolRec.name = symbol;
                    symbolRec.typeChain.types.push_back(TypeChainRecord::Type{TypeChainRecord::Type::DCLType::FUNCTION, 0, symbol});
                    symbolRec.scope = Scope{Scope::Type::GLOBAL};
                    data.addSymbol(symbolRec);

                    // Add linker record
                    LinkerRecord linkerRec;
                    linkerRec.type = LinkerRecord::Type::SYMBOL_ADDR;
                    linkerRec.name = symbol;
                    linkerRec.addr = address;
                    linkerRec.scope = Scope{Scope::Type::GLOBAL};
                    data.addLinkerRecord(linkerRec);
                }
            }

            break;

        default:
            break;
        }
    }
}

void DbgDataParser::parseModule(std::vector<Token> &tokens, size_t &i, DebuggerData &data)
{
    if (tokens.size() < 1)
    {
        return;
    }
    data.addModule(tokens[0].value);
    (void)i;
}

FunctionRecord DbgDataParser::parseFunction(std::vector<Token> &tokens, size_t &i)
{
    FunctionRecord func;
    if (tokens.size() < 14)
    {
        return func;
    }
    parseScopeNameLevelBlock(tokens, i, func);
    func.typeChain = parseTypeChain(tokens, i);
    func.addressSpace = (AddressSpace)tokens[i++].value[0];
    func.onStack = (tokens[i++].value[0] != '0');
    func.stack_offs = std::stoi(tokens[i++].value);
    func.interrupt = (tokens[i++].value[0] != '0');
    func.interruptNum = std::stoi(tokens[i++].value);
    func.regBankNum = std::stoi(tokens[i++].value);
    return func;
}

SymbolRecord DbgDataParser::parseSymbol(std::vector<Token> &tokens, size_t &i)
{
    SymbolRecord symbol;
    if (tokens.size() < 12)
    {
        return symbol;
    }
    parseScopeNameLevelBlock(tokens, i, symbol);
    symbol.typeChain = parseTypeChain(tokens, i);
    symbol.addressSpace = (AddressSpace)tokens[i++].value[0];
    symbol.onStack = (tokens[i++].value[0] != '0');
    symbol.stack_offs = std::stoi(tokens[i++].value);
    if (tokens[i].type == Token::Type::LeftBracket)
    {
        while (tokens[i].type != Token::Type::RightBracket)
        {
            if (tokens[i].type == Token::Type::LeftBracket)
            {
                i++;
                continue;
            }
            symbol.registers.push_back(getReg(tokens[i++]));
        }
    }
    if (tokens[i].type == Token::Type::RightBracket)
    {
        i++;
    }
    return symbol;
}

TypeRecord DbgDataParser::parseType(std::vector<Token> &tokens, size_t &i)
{
    TypeRecord type;
    if (tokens.size() < 4)
    {
        return type;
    }
    type.scope.type = (Scope::Type)tokens[i++].value[0];
    if (type.scope.type != Scope::Type::GLOBAL)
    {
        type.scope.name = tokens[i - 1].value.substr(1);
    }
    type.name = tokens[i++].value;
    while (tokens[i].type != Token::Type::RightBracket && tokens[i].type != Token::Type::LineEnd)
    {
        while (tokens[i].type == Token::Type::LeftBracket)
        {
            i++;
        }
        TypeMember member;
        member.offset = std::stoi(tokens[i++].value);
        if (tokens[i].type == Token::Type::RightBracket)
        {
            i++;
        }
        if (tokens[i].type == Token::Type::Text &&
            tokens[i].value == "S") // Symbol record begins with "S"
        {
            i++;
        }
        member.member = parseSymbol(tokens, i);
        type.members.push_back(member);
        if (tokens[i].type == Token::Type::RightBracket)
        {
            i++;
        }
    }
    return type;
}

LinkerRecord DbgDataParser::parseLinker(std::vector<Token> &tokens, size_t &i)
{
    LinkerRecord record;
    record.scope.type = Scope::Type::GLOBAL;
    {
        Token &type = tokens[i];
        record.type = LinkerRecord::Type::SYMBOL_ADDR;
        if (type.value[0] == 'X')
        {
            record.type = LinkerRecord::Type::SYMBOL_END_ADDR;
            type.value = type.value.substr(1);
        }
        else if (type.value[0] == 'A')
        {
            record.type = LinkerRecord::Type::ASM_LINE;
            i++;
        }
        else if (type.value[0] == 'C')
        {
            record.type = LinkerRecord::Type::C_LINE;
            i++;
        }
    }
    if (record.type == LinkerRecord::Type::SYMBOL_ADDR ||
        record.type == LinkerRecord::Type::SYMBOL_END_ADDR)
    {
        parseScopeNameLevelBlock(tokens, i, record);
        record.addr = std::stoul(tokens[i++].value, 0, 16);
    }
    else if (record.type == LinkerRecord::Type::ASM_LINE)
    {
        record.name = tokens[i++].value;
        record.line = std::stoi(tokens[i++].value);
        record.addr = std::stoul(tokens[i++].value, 0, 16);
    }
    else
    {
        record.name = tokens[i++].value;
        record.line = std::stoi(tokens[i++].value);
        record.level = std::stoi(tokens[i++].value);
        record.block = std::stoi(tokens[i++].value);
        record.addr = std::stoul(tokens[i++].value, 0, 16);
    }
    return record;
}

TypeChainRecord DbgDataParser::parseTypeChain(std::vector<Token> &tokens, size_t &i)
{
    TypeChainRecord typeChain;
    if (tokens.size() < 2)
    {
        return typeChain;
    }
    if (tokens[i].type == Token::Type::LeftBracket)
    {
        i++;
    }
    if (tokens[i].type == Token::Type::LeftBracket)
    {
        i++;
    }
    typeChain.size = std::stoi(tokens[i++].value);
    if (tokens[i].type == Token::Type::RightBracket)
    {
        i++;
    }
    while (1)
    {
        Token &token = tokens[i];
        if (token.value.size() < 2)
        {
            typeChain.sign = (token.value == "S");
            i++;
            break;
        }
        else
        {
            TypeChainRecord::Type type;
            type.DCLtype = getDCLType(token);
            if (type.DCLtype == TypeChainRecord::Type::DCLType::STRUCT)
            {
                type.name = token.value.substr(2);
            }
            else if (type.DCLtype == TypeChainRecord::Type::DCLType::ARRAY)
            {
                type.num.n = strtoull(token.value.substr(2).c_str(), nullptr, 10);
            }
            else if (type.DCLtype == TypeChainRecord::Type::DCLType::BITFIELD)
            {
                type.num.bitField.offset = strtoull(token.value.substr(2).c_str(), nullptr, 10);
                type.num.bitField.size = strtoull(tokens[++i].value.c_str(), nullptr, 10);
            }
            typeChain.types.push_back(type);
        }
        i++;
    }
    while (tokens[i].type == Token::Type::RightBracket)
    {
        i++;
    }
    return typeChain;
}

void DbgDataParser::parseScopeNameLevelBlock(std::vector<Token> &tokens, size_t &i, ScopeNameLevelBlock &data)
{
    data.scope.type = (Scope::Type)tokens[i++].value[0];
    data.scope.name.clear();
    if (data.scope.type != Scope::Type::GLOBAL && data.scope.type != Scope::Type::STRUCT)
    {
        data.scope.name = tokens[i - 1].value.substr(1);
    }
    data.name += tokens[i++].value;
    data.level = std::stoi(tokens[i++].value);
    data.block = std::stoi(tokens[i++].value);
}

REG DbgDataParser::getReg(Token token)
{
    std::string reg = token.value;
    if (reg == "a")
    {
        return REG::A_IDX;
    }
    else if (reg == "x")
    {
        return REG::X_IDX;
    }
    else if (reg == "y")
    {
        return REG::Y_IDX;
    }
    else if (reg == "sp")
    {
        return REG::SP_IDX;
    }
    else if (reg == "r0")
    {
        return REG::R0_IDX;
    }
    else if (reg == "r1")
    {
        return REG::R1_IDX;
    }
    else if (reg == "r2")
    {
        return REG::R2_IDX;
    }
    else if (reg == "r3")
    {
        return REG::R3_IDX;
    }
    return REG::R0_IDX;
}

TypeChainRecord::Type::DCLType DbgDataParser::getDCLType(Token token)
{
    char c1 = token.value[0];
    char c2 = token.value[1];
    if (c1 == 'D')
    {
        if (c2 == 'A')
        {
            return TypeChainRecord::Type::DCLType::ARRAY;
        }
        else if (c2 == 'F')
        {
            return TypeChainRecord::Type::DCLType::FUNCTION;
        }
        else if (c2 == 'G')
        {
            return TypeChainRecord::Type::DCLType::GEN_POINTER;
        }
        else if (c2 == 'C')
        {
            return TypeChainRecord::Type::DCLType::CODE_POINTER;
        }
        else if (c2 == 'X')
        {
            return TypeChainRecord::Type::DCLType::EXT_RAM_POINTER;
        }
        else if (c2 == 'D')
        {
            return TypeChainRecord::Type::DCLType::INT_RAM_POINTER;
        }
        else if (c2 == 'P')
        {
            return TypeChainRecord::Type::DCLType::PAGED_POINTER;
        }
        else if (c2 == 'I')
        {
            return TypeChainRecord::Type::DCLType::UPPER128_POINTER;
        }
    }
    else
    {
        if (c2 == 'L')
        {
            return TypeChainRecord::Type::DCLType::LONG;
        }
        else if (c2 == 'I')
        {
            return TypeChainRecord::Type::DCLType::INT;
        }
        else if (c2 == 'C')
        {
            return TypeChainRecord::Type::DCLType::CHAR;
        }
        else if (c2 == 'S')
        {
            return TypeChainRecord::Type::DCLType::SHORT;
        }
        else if (c2 == 'V')
        {
            return TypeChainRecord::Type::DCLType::VOID;
        }
        else if (c2 == 'F')
        {
            return TypeChainRecord::Type::DCLType::FLOAT;
        }
        else if (c2 == 'T')
        {
            return TypeChainRecord::Type::DCLType::STRUCT;
        }
        else if (c2 == 'X')
        {
            return TypeChainRecord::Type::DCLType::SBIT;
        }
        else if (c2 == 'B')
        {
            return TypeChainRecord::Type::DCLType::BITFIELD;
        }
    }
    return TypeChainRecord::Type::DCLType::UNKNOWN;
}

std::vector<DbgDataParser::Token> DbgDataParser::tokenize(const std::string &line)
{
    std::vector<Token> tokens;

    for (size_t i = 0; i < line.size(); i++)
    {
        char c = line[i];

        if (c == '(' || c == '[' || c == '{')
        {
            tokens.push_back(Token{Token::Type::LeftBracket, std::string(1, c)});
        }
        else if (c == ')' || c == ']' || c == '}')
        {
            tokens.push_back(Token{Token::Type::RightBracket, std::string(1, c)});
        }
        else if (c == ':' || c == ',' || c == '$')
        {
            continue;
        }
        else
        {
            size_t start = i;
            while (i < line.size() && line[i] != '(' && line[i] != '[' && line[i] != '{' &&
                   line[i] != ')' && line[i] != ']' && line[i] != '}' && line[i] != ':' &&
                   line[i] != ',' && line[i] != '$')
            {
                i++;
            }
            if (i > start)
            {
                tokens.push_back(Token{Token::Type::Text, line.substr(start, i - start)});
                i--;
            }
        }
    }
    tokens.push_back(Token{Token::Type::LineEnd, "\n"});
    return tokens;
}
