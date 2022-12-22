#include <rsdio.h>
#include <rserrors.h>

#include <functional>
#include <iostream>
#include <memory>

#include "utils.h"

typedef std::shared_ptr<rs::RsDio> RsDio_t;

int main(int argc, char *argv[])
{
    RsDio_t dio(rs::createRsDio(), std::mem_fn(&rs::RsDio::destroy));

    dio->setXmlFile("fake_file");
    verifyError(
        "setXmlFile(fake_file)",
        dio->getLastError(),
        std::errc::no_such_file_or_directory
    );

    dio->setXmlFile("invalid.xml");
    verifyError(
        "setXmlFile(invalid.xml",
        dio->getLastError(),
        RsErrorCode::XmlParseError
    );
}