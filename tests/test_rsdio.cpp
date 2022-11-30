#include <iostream>
#include <memory>
#include <functional>

#include <rsdio.h>
#include <rserrors.h>

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
    return 0;
}

int main(int argc, char *argv[])
{
    ArgParse mainParser("rsdio_test");

    mainParser.addPositionalArg(PositionalArg("filename", "XML file to parse"));

    PositionalArg commandArg("command", "Test command to run");
    // Run all tests. We don't need any args but let the parser know about he option.
    mainParser.addSubCommand(commandArg, "all");

    // Test XML parsing
    ArgParse &xmlCommandParser = mainParser.addSubCommand(commandArg, "xml");

    if (!mainParser.parse(argc, argv))
    {
        std::cerr << "Error parsing arguments\n";
        return 1;
    }

    std::string command, xmlFile;
    mainParser.getValue("command", command);
    mainParser.getValue("filename", xmlFile);
    if (command == "xml")
    {
        
    }

    RsDio_t dio(rs::createRsDio(), std::mem_fn(&rs::RsDio::destroy));
    if (dio->setXmlFile(xmlFile.c_str()))
    {
        std::cerr << "setXmlFile returned true with invalid file" << std::endl;
        return 1;
    }
}