#include <rserrors.h>
#include <rspoe.h>

#include <functional>
#include <iostream>
#include <memory>

#include "utils.h"

typedef std::shared_ptr<rs::RsPoe> RsPoe_t;

int main(int argc, char *argv[])
{
    RsPoe_t poe(rs::createRsPoe(), std::mem_fn(&rs::RsPoe::destroy));
    poe->setXmlFile("fake_file");
    verifyError(
        "setXmlFile(fake_file)",
        poe->getLastError(),
        std::errc::no_such_file_or_directory
    );

    poe->setXmlFile("invalid.xml");
    verifyError(
        "setXmlFile(invalid.xml)",
        poe->getLastError(),
        RsErrorCode::XmlParseError
    );

    return 0;
}