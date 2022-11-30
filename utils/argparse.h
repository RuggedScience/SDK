#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <map>
#include <vector>
#include <string>
#include <ostream>

enum class ArgType
{
    Flag,
    Value
};

struct PositionalArg
{
    std::string name;
    std::string description;

    PositionalArg(std::string name, std::string description)
        : name(name), description(description)
    {
    }
};

struct OptionalArg : PositionalArg
{
    ArgType type;
    std::vector<std::string> options;

    OptionalArg(std::string name, std::string description, ArgType type, std::vector<std::string> options)
        : PositionalArg(name, description), type(type), options(options)
    {
    }
};

class ArgParse
{
public:
    ArgParse() {}
    ArgParse(std::string name);

    ArgParse &addSubCommand(const PositionalArg &arg, std::string value);
    void addPositionalArg(const PositionalArg &arg);
    void addOptionalArg(const OptionalArg &arg);

    bool parse(int argc, char *argv[]);

    bool getValue(std::string name) const;
    bool getValue(std::string name, std::string &value) const;

    void printUsage() const;
    void printUsage(std::ostream &stream) const;

private:
    void sanitizeString(std::string &arg) const;
    void addOptionalDashes(std::string &arg) const;

    OptionalArg *getOptionalArg(std::string arg);

    std::string m_programName;
    std::string m_description;
    std::vector<PositionalArg> m_positionalArgs;
    std::vector<OptionalArg> m_optionalArgs;
    std::map<std::string, ArgParse> m_subParsers;
    std::map<std::string, std::string> m_foundArgs;
};

#endif // ARGPARSE_H