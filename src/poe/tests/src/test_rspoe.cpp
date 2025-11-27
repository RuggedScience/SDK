#include "rspoe.h"

#include <functional>
#include <iostream>
#include <memory>


typedef std::shared_ptr<rs::RsPoe> RsPoe_t;

int main(int argc, char *argv[])
{
    RsPoe_t poe(rs::createRsPoe(), std::mem_fn(&rs::RsPoe::destroy));
    poe->init("fake_file");
    poe->init("invalid.xml");
    return 0;
}