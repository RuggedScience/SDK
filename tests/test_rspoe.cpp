#include <iostream>
#include <memory>
#include <functional>

#include <rspoe.h>
#include <rserrors.h>

typedef std::shared_ptr<rs::RsPoe> RsPoe_t;

int testXml(const char *xmlFile)
{
    RsPoe_t poe(rs::createRsPoe(), std::mem_fn(&rs::RsPoe::destroy));
    if (!poe->setXmlFile(xmlFile))
    {
        std::cerr << "setXmlFile returned false with valid file" << std::endl;
        return 1;
    }
    return 0;
}

int testXml(const char *xmlFile, std::error_code expectedError)
{
    RsPoe_t poe(rs::createRsPoe(), std::mem_fn(&rs::RsPoe::destroy));
    
    if (poe->setXmlFile(xmlFile))
    {
        std::cerr << "setXmlFile returned true with invalid file" << std::endl;
        return 1;
    }

    if (poe->getLastError() != expectedError)
    {
        std::cerr << "Expected " << expectedError.message() << ". Got " << poe->getLastErrorString() << std::endl;
        return 1; 
    }
    return 0;
}

int testXml(const char *xmlFile, std::errc expectedError)
{
    return testXml(xmlFile, std::make_error_code(expectedError));
}

int testXml(const char *xmlFile, RsErrorCode expectedError)
{
    return testXml(xmlFile, make_error_code(expectedError));
}

int main(int argc, char *argv[])
{
    if (testXml("fake_file", std::errc::no_such_file_or_directory))
        return 1;

    if (testXml("invalid.xml", RsErrorCode::XmlParseError))
        return 1;

    return 0;
}