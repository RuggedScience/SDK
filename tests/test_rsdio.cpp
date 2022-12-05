#include <iostream>
#include <memory>
#include <functional>

#include <rsdio.h>
#include <rserrors.h>

typedef std::shared_ptr<rs::RsDio> RsDio_t;

int testXml(const char *xmlFile)
{
    RsDio_t dio(rs::createRsDio(), std::mem_fn(&rs::RsDio::destroy));
    if (!dio->setXmlFile(xmlFile))
    {
        std::cerr << "setXmlFile returned false with valid file" << std::endl;
        return 1;
    }
    return 0;
}

int testXml(const char *xmlFile, std::error_code expectedError)
{
    RsDio_t dio(rs::createRsDio(), std::mem_fn(&rs::RsDio::destroy));
    if (dio->setXmlFile(xmlFile))
    {
        std::cerr << "setXmlFile returned true with invalid file" << std::endl;
        return 1;
    }

    if (dio->getLastError() != expectedError)
    {
        std::cerr << "Expected " << expectedError.message() << ". Got " << dio->getLastErrorString() << std::endl;
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
}