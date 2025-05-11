#include "CLI.h"
#include <iostream>
#include <iomanip>

CLI::CLI()
{
    addCommand("help", "", false, [this](const std::vector<std::string> &args)
               { printUsage(); (void)args ;}, "print avaliable command and usage");
}

bool CLI::run()
{
    std::string line;
    std::cout << ">> ";
    if (!std::getline(std::cin, line))
        return false;
    processCommand(line);
    return !running;
}

void CLI::addCommand(const std::string &name, const std::string &args, bool allowShortAlias,
                const std::function<void(const std::vector<std::string> &)> &handler,
                const std::string &usage)
{
    commands[name] = {handler, allowShortAlias, usage, args};
    if (allowShortAlias && !name.empty())
    {
        char shortAlias = name[0];
        shortAliases[shortAlias] = name;
    }
}

void CLI::processCommand(const std::string &input)
{
    std::vector<std::string> args;
    parseArguments(input, args);
    processFullCommand(args);
}

void CLI::parseArguments(const std::string &input, std::vector<std::string> &args)
{
    std::string currentArg;
    bool insideQuotes = false;
    char quoteChar = 0;  // To track the current type of quote (single or double)
    bool escapeNextChar = false;  // Flag to handle escaping

    for (size_t i = 0; i < input.size(); ++i)
    {
        char c = input[i];

        // Handle escape sequences only inside quotes
        if (escapeNextChar && insideQuotes)
        {
            handleEscape(input, i, currentArg);
            escapeNextChar = false;  // Reset escape flag
            continue;
        }

        // Check for escape sequence (only inside quotes)
        if (c == '\\' && insideQuotes)
        {
            escapeNextChar = true; // Flag the next character to be escaped
            continue;
        }

        // Handle quotes
        handleQuotes(c, insideQuotes, quoteChar);

        // If we're inside quotes, accumulate the characters
        if (insideQuotes && c != quoteChar)
        {
            currentArg += c;
        }
        else if (c == ' ' || c == '\t') // Handle space outside of quotes
        {
            if (!currentArg.empty())
            {
                args.push_back(currentArg);
                currentArg.clear();
            }
        }
        else if (c != '\'' && c != '"')
        {
            currentArg += c; // Add regular characters
        }
    }

    // Add the last argument if there's one remaining
    if (!currentArg.empty())
    {
        args.push_back(currentArg);
    }
}

void CLI::handleEscape(const std::string &input, size_t &i, std::string &currentArg)
{
    char c = input[i];

    // Handle common escape sequences
    if (c == 'n') 
        currentArg += '\n';
    else if (c == 't') 
        currentArg += '\t';
    else if (c == 'r') 
        currentArg += '\r';
    else if (c == 'b') 
        currentArg += '\b';
    else if (c == 'f') 
        currentArg += '\f';
    else if (c == 'v') 
        currentArg += '\v';
    else if (c == '0') 
        currentArg += '\0';
    else if (c == '\\') 
        currentArg += '\\';
    else if (c == '\"') 
        currentArg += '\"';
    else if (c == '\'') 
        currentArg += '\'';
    
    // Hexadecimal escape sequence \xNN (2 hex digits)
    else if (c == 'x' && i + 3 < input.size() && isxdigit(input[i + 1]) && isxdigit(input[i + 2]))
    {
        ++i; // Skip the 'x'
        std::string hexStr = input.substr(i, 2);  // Extract the next two characters
        int hexVal = 0;
        std::stringstream ss;
        ss << std::hex << hexStr;
        ss >> hexVal;

        currentArg += static_cast<char>(hexVal);  // Add corresponding character

        i += 1;  // Skip past the two hex digits
        return;
    }
    
    // Unicode escape sequence \uNNNN (4 hex digits)
    else if (c == 'u' && i + 5 < input.size() && isxdigit(input[i + 1]) && isxdigit(input[i + 2]) &&
             isxdigit(input[i + 3]) && isxdigit(input[i + 4]))
    {
        ++i; // Skip the 'u'
        std::string hexStr = input.substr(i, 4);  // Extract the next four characters
        int unicodeVal = 0;
        std::stringstream ss;
        ss << std::hex << hexStr;
        ss >> unicodeVal;

        currentArg += static_cast<char>(unicodeVal);  // Add corresponding character

        i += 3;  // Skip past the four hex digits
        return;
    }

    // Octal escape sequence \NNN (up to 3 digits)
    else if (c >= '0' && c <= '7' && i + 2 < input.size() && isdigit(input[i + 1]) && isdigit(input[i + 2]))
    {
        std::string octStr = input.substr(i, 3);  // Extract the next three digits
        int octVal = 0;
        std::stringstream ss;
        ss << std::oct << octStr;
        ss >> octVal;

        currentArg += static_cast<char>(octVal);  // Add corresponding character

        i += 2;  // Skip past the three octal digits
        return;
    }

    // Handle other escape sequences as regular characters
    else
    {
        currentArg += '\\';  // Add '\'
        currentArg += c;  // And add the other character
    }
}

void CLI::handleQuotes(char c, bool &insideQuotes, char &quoteChar)
{
    // Check if the current character is a quote
    if ((c == '"' || c == '\'') && (insideQuotes == false || c == quoteChar))
    {
        // If we're inside quotes, close it
        if (insideQuotes)
        {
            insideQuotes = false;
            quoteChar = 0;
        }
        else
        {
            // Otherwise, open a quote
            insideQuotes = true;
            quoteChar = c;
        }
    }
}

void CLI::processFullCommand(const std::vector<std::string> &args)
{
    if (args.empty())
    {
        return;
    }

    std::string command = args[0];

    // Process the command and arguments
    // Check for short alias
    if (command.length() == 1 && shortAliases.find(command[0]) != shortAliases.end())
    {
        commands[shortAliases[command[0]]].handler(args);
        return;
    }

    // Check for full command
    auto it = commands.find(command);
    if (it != commands.end())
    {
        it->second.handler(args);
        return;
    }

    std::cerr << "Unknown command: " << command << "\n";
}

void CLI::quit()
{
    running = false;
}

void CLI::printUsage()
{
    printf("%s", usage_str.c_str());
    std::cout << "Commands:\n";
    for (const auto &[name, info] : commands)
    {
        if (info.allowShortAlias)
        {
            if (info.args.size())
            {
                std::cout << std::setw(3) << name[0] << "    "  << name << " " << info.args << " - " << info.usage << "\n";
            }
            else
            {
                std::cout << std::setw(3) << name[0] << "    " << name << " - " << info.usage << "\n";
            }
        }
        else
        {
            if (info.args.size())
            {
                std::cout << "       "  << name << " " << info.args << " - "  << info.usage << "\n";
            }
            else
            {
                std::cout << "       " << name << " - " << info.usage << "\n";
            }
        }
    }
}
