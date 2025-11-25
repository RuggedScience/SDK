#include <rsdio.h>
#include <rserrors.h>
#include <test_utils.h>

#include <functional>
#include <memory>

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

    dio->setXmlFile("/home/rugged/Downloads/mc4000.xml");

    dio->setXmlFile("invalid.xml");
    verifyError(
        "setXmlFile(invalid.xml)",
        dio->getLastError(),
        RsErrorCode::XmlParseError
    );
}