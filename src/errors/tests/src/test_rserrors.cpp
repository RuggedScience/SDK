#include <iostream>
#include <memory>
#include <functional>

#include <rsdio.h>
#include <rspoe.h>
#include <rserrors.h>

typedef std::shared_ptr<rs::RsDio> RsDio_t;
typedef std::shared_ptr<rs::RsPoe> RsPoe_t;

int main(int argc, char *argv[])
{
    RsDio_t dio(rs::createRsDio(), std::mem_fn(&rs::RsDio::destroy));
    RsPoe_t poe(rs::createRsPoe(), std::mem_fn(&rs::RsPoe::destroy));

    dio->setXmlFile("fake_file");
    poe->setXmlFile("fake_file");

    // Ensure the static error category is always the same between libs
    if (dio->getLastError() != poe->getLastError())
    {
        std::cerr << "DIO and PoE errors don't match" << std::endl;
        return 1;
    }

    return 0;
}