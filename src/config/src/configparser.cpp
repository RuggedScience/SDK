#include "configparser.h"

#include <memory>
#include <string>

#include "rserrors.h"
#include "rsparser.h"

void ConfigParser::parseImpl(const char* fileName)
{
    std::vector<std::unique_ptr<AbstractConfigParser>> parsers;
    // Add additional parsers here
    parsers.emplace_back(new RsXmlParser());

    // Loop through all parsers and try to parse the file.
    // If one succeeds, store the configs and return.
    for (const auto& parser : parsers) {
        try {
            parser->parse(fileName);
            for (auto config : parser->getDioControllerConfigs()) {
                addDioConfig(config);
            }

            for (auto config : parser->getPoeControllerConfigs()) {
                addPoeConfig(config);
            }
            return;
        }
        catch (rs::RsException exception) {
            // If the file wasn't found, no reason to keep trying.
            if (exception.code() == rs::RsErrorCode::FileNotFound) {
                throw;
            }
        }
    }

    throw rs::RsException(
        rs::RsErrorCode::ConfigParseError, "Invalid configuration file"
    );
}