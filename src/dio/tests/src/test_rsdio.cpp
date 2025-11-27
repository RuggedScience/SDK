#include "rsdio.h"
#include "rserrors.h"

#include <iostream>
#include <functional>
#include <memory>

typedef std::shared_ptr<rs::RsDio> RsDio_t;

int main(int argc, char *argv[])
{
    RsDio_t dio(rs::createRsDio(), std::mem_fn(&rs::RsDio::destroy));

    try {
        dio->init("fake_file");
        std::cerr << "Expected FileNotFound exception but got nothing" << std::endl;
        return 1;
    } catch (const rs::RsException &ex) {
        if (ex.code() != rs::RsErrorCode::FileNotFound) {
            std::cerr << "Ivalid exception" << std::endl;
            return 1;
        }
    }

    

    dio->init("invalid.xml");
}