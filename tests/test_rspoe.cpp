#include <iostream>
#include <memory>
#include <functional>

#include <rspoe.h>
#include <rserrors.h>

#include "argparse.h"

typedef std::shared_ptr<rs::RsPoe> RsPoe_t;

int testNoSuchFile(RsPoe_t poe)
{
    if (poe->setXmlFile("fake_file"))
    {
        std::cerr << "setXmlFile returned true with invalid file" << std::endl;
        return 1;
    }

    if (poe->getLastError() != std::errc::no_such_file_or_directory)
    {
        std::cerr << "Expected no such file error. Got " << poe->getLastErrorString() << std::endl;
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
        //return 1;
    }

    std::string command, xmlFile;
    mainParser.getValue("command", command);
    mainParser.getValue("filename", xmlFile);
    if (command == "xml")
    {
        
    }

    RsPoe_t poe(rs::createRsPoe(), std::mem_fn(&rs::RsPoe::destroy));
    if (poe->setXmlFile("/home/tlassiter/SDK/xml/ecs9000.xml"))
    {
        std::cerr << "setXmlFile returned true with invalid file" << std::endl;
        return 1;
    }

    std::cout << (poe->getLastError() == RsErrorCode::XmlParseError) << std::endl;
    std::cout << (poe->getLastError() == RsErrorCondition::HardwareError) << std::endl;
    std::cout << (poe->getLastError() == RsErrorCondition::UserError) << std::endl;
    std::cout << (poe->getLastError() == std::errc::operation_not_permitted) << std::endl;
}