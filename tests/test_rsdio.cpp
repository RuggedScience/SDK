#include <iostream>
#include <memory>
#include <functional>

#include <rsdio.h>
#include <rspoe.h>
#include <rssdk_errors.h>

int main(int argc, char *argv[])
{
    std::shared_ptr<rs::RsDio> dio(rs::createRsDio(), std::mem_fn(&rs::RsDio::destroy));
    std::shared_ptr<rs::RsPoe> poe(rs::createRsPoe(), std::mem_fn(&rs::RsPoe::destroy));

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

    if (dio->setXmlFile("/home/rugged/sdk/tests/invalid.xml"))
    {
        std::cerr << "setXmlFile returned true with invalid file" << std::endl;
        return 1;
    }

    if (poe->setXmlFile("/home/rugged/sdk/tests/invalid.xml"))
    {
        
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