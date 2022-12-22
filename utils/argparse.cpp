#include "argparse.h"

#include <assert.h>
#include <algorithm>
#include <iostream>


ArgParse::ArgParse(std::string name)
    : m_programName(name)
{

}

ArgParse &ArgParse::addSubCommand(const PositionalArg &arg, std::string value)
{
    bool found = false;
    for (const PositionalArg &posArg : m_positionalArgs)
    {
        if (arg.name == posArg.name)
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        m_positionalArgs.emplace_back(arg);
    }

    m_subParsers[value] = ArgParse(m_programName + " " + value);
    return m_subParsers[value];
}

void ArgParse::addPositionalArg(const PositionalArg &arg)
{
    assert(!arg.name.empty());

    PositionalArg posArg{arg};
    this->sanitizeString(posArg.name);
    m_positionalArgs.emplace_back(posArg);
}

void ArgParse::addOptionalArg(const OptionalArg &arg)
{
    assert(!arg.options.empty());

    OptionalArg optArg{arg};

    for (std::string &option : optArg.options)
    {
        this->sanitizeString(option);
        this->addOptionalDashes(option);
    }

    m_optionalArgs.emplace_back(optArg);
}

bool ArgParse::parse(int argc, char *argv[])
{
    m_foundArgs.clear();
    
    std::vector<std::string> ignoredOptionalArgs;
    std::vector<std::string> parsedPositionalArgs;

    int i;
    OptionalArg *previousArg = nullptr;
    for (i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "--help")
        {
            this->printUsage();
            return true;
        }

        // If the previous optional arg was a value type
        // this arg after should be it's associated value.
        if (previousArg)
        {
            m_foundArgs[previousArg->name] = arg;
            previousArg = nullptr;
            continue;
        }

        // Parse all optional args and add the remaning
        // args as positional args to be parsed next.
        if (arg.at(0) == '-')
        {
            OptionalArg *optArg = getOptionalArg(arg);
            if (optArg)
            {
                m_foundArgs[optArg->name] = "";
                if (optArg->type != ArgType::Flag)
                {
                    previousArg = optArg;
                }
            }
            else
            {
                ignoredOptionalArgs.emplace_back(arg);
            }
        }
        else
        {
            parsedPositionalArgs.emplace_back(arg);

            // If we have all of our positional args filled and we have a sub parser
            // that means we need to pass the remaining optional args down to it.
            if (parsedPositionalArgs.size() == m_positionalArgs.size() && !m_subParsers.empty())
            {
                break;
            }
        }
    }

    // Not enough arguments
    if (parsedPositionalArgs.size() < m_positionalArgs.size())
    {
        this->printUsage();
        return false;
    }

    std::string name;
    for (size_t i = 0; i < m_positionalArgs.size(); ++i)
    {
        name = m_positionalArgs[i].name;
        m_foundArgs[name] = parsedPositionalArgs[i];
    }

    if (!m_subParsers.empty())
    {
        std::string subcommand = m_foundArgs[name];
        if (m_subParsers.find(subcommand) == m_subParsers.end())
        {
            this->printUsage();
            return false;
        }

        return m_subParsers[subcommand].parse(argc - i, argv + i);
    }

    return true;
}

bool ArgParse::getValue(std::string name) const
{
    std::string v;
    return this->getValue(name, v);
}

bool ArgParse::getValue(std::string arg, std::string &value) const
{
    value.clear();
    if (m_foundArgs.find(arg) == m_foundArgs.end())
    {
        return false;
    }
    
    value = m_foundArgs.at(arg);
    return true;
}

void ArgParse::printUsage() const
{
    this->printUsage(std::cout);
}

void ArgParse::printUsage(std::ostream &stream) const
{
    stream << m_programName;

    for (const PositionalArg &arg : m_positionalArgs)
    {
        stream << " " << arg.name;
    }

    if (!m_optionalArgs.empty())
    {
        stream << " [OPTIONS...]";
    }

    stream << "\n";

    if (!m_positionalArgs.empty())
    {
        stream << "\nPositional Arguments:\n";
    
        size_t maxLength = 0;
        size_t minLength = 100;
        for (const PositionalArg &arg : m_positionalArgs)
        {
            maxLength = std::max(maxLength, arg.name.size());
            minLength = std::min(minLength, arg.name.size());
        }

        maxLength += 4;
        for (const PositionalArg &arg : m_positionalArgs)
        {
            size_t spaces = maxLength - arg.name.size();
            stream << "\t" << arg.name;
            for (int i = 0; i < spaces; ++i)
                stream << " ";
            stream << arg.description << "\n";
        }

        for (std::pair<std::string, ArgParse> pair : m_subParsers)
        {
            stream << "\t\t" << pair.first << "\n";
        }
    }

    if (!m_optionalArgs.empty())
    {

        stream << "\nOptions:\n";
        for (const OptionalArg &arg : m_optionalArgs)
        {
            stream << "\t";
            for (const std::string &opt : arg.options)
            {
                stream << opt << " | ";
            }

            stream << "\t" << arg.description;
        }
    }
}

void ArgParse::sanitizeString(std::string &arg) const
{
    // Replace all spaces with underscores
    std::replace(arg.begin(), arg.end(), ' ', '_');
}

void ArgParse::addOptionalDashes(std::string &arg) const
{
    if (arg.at(0) != '-')
    {
        // If the arg is just one character we can assume it's a shorthand arg which only has one dash.
        if (arg.length() == 1)
        {
            arg = "-" + arg;
        }
        else
        {
            arg = "--" + arg;
        }
    }
}

OptionalArg *ArgParse::getOptionalArg(std::string arg)
{
    for (OptionalArg &opt : m_optionalArgs)
    {
        std::vector<std::string> options = opt.options;
        if (std::find(options.begin(), options.end(), arg) != options.end())
        {
            return &opt;
        }
    }

    return nullptr;
}