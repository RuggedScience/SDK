#include <iostream>
#include <memory>
#include <functional>

#include <rsdio.h>
#include <rssdk_errors.h>

#include "argparse.h"

typedef std::shared_ptr<rs::RsDio> RsDio_t;

int testNoSuchFile(RsDio_t dio)
{
    if (dio->setXmlFile("fake_file"))
    {
        std::cerr << "setXmlFile returned true with invalid file" << std::endl;
        return 1;
    }

    if (dio->getLastError() != std::errc::no_such_file_or_directory)
    {
        std::cerr << "Expected no such file error. Got " << dio->getLastErrorString() << std::endl;
        return 1; 
    }

}

int main(int argc, char *argv[])
{
    ArgParse mainParser("rsdio_test");

    mainParser.addPositionalArg(PositionalArg{
        .name = "filename", 
        .description = "XML file to parse"
    });

    PositionalArg commandArg{
        .name = "command", 
        .description = "Test command to run"
    };

    // Run all tests. We don't need any args but let the parser know about he option.
    mainParser.addSubCommand(commandArg, "all");

    // Test XML parsing
    ArgParse &xmlCommandParser = mainParser.addSubCommand(commandArg, "xml");

    if (!mainParser.parse(argc, argv))
    {
        std::cerr << "Error parsing arguments\n";
        return 1;
    }

    std::string command;
    mainParser.getValue("command", command);
    if (command == "xml")
    {

    }

    RsDio_t dio(rs::createRsDio(), std::mem_fn(&rs::RsDio::destroy));

    std::string xmlFile;
    xmlCommandParser.getValue("filename", xmlFile);
    if (dio->setXmlFile(xmlFile.c_str()))
    {
        std::cerr << "setXmlFile returned true with invalid file" << std::endl;
        return 1;
    }


    std::error_code c = dio->getLastError();
    std::error_code e = RsSdkError::XmlParseError;

    if (dio->getLastError() != RsSdkError::XmlParseError)
    {
        std::cerr << dio->getLastError().message() << std::endl;
        std::cerr << RsSdkError::XmlParseError << std::endl;
        std::cerr << "Expected " << RsSdkError::XmlParseError << ". Got " << dio->getLastError() << std::endl;
        return 1; 
    }
}