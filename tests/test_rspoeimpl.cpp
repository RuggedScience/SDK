#include <iostream>

#include "../poe/src/rspoeimpl.h"
#include "poecontroller.h"
#include "utils.h"

int main()
{
    portmap_t portMap;
    std::vector<uint8_t> ports = {0, 1, 2, 3};
    for (uint8_t port : ports) {
        portMap[port] = port;
    }
    TestPoeController *controller = new TestPoeController(100, ports);
    RsPoeImpl poe(controller, portMap);

    poe.getPortState(5);
    verifyError(
        "getPortState (invalid port)",
        poe.getLastError(),
        std::errc::invalid_argument
    );

    poe.getPortState(0);
    verifyError("getPortState (valid)", poe.getLastError());

    poe.setPortState(5, rs::PoeState::Auto);
    verifyError(
        "setPortState (invalid port)",
        poe.getLastError(),
        std::errc::invalid_argument
    );

    poe.setPortState(0, rs::PoeState::Error);
    verifyError(
        "setPortState (invalid state)",
        poe.getLastError(),
        std::errc::invalid_argument
    );

    poe.setPortState(0, rs::PoeState::Auto);
    verifyError("setPortState (valid)", poe.getLastError());

    rs::PoeState state;
    state = poe.getPortState(0);
    if (state != rs::PoeState::Auto) {
        std::cerr << "getPortState returned " << (int)state
                  << " after calling setPortState with PoeState::Auto"
                  << std::endl;
    }

    poe.setPortState(0, rs::PoeState::Disabled);
    state = poe.getPortState(0);
    if (state != rs::PoeState::Disabled) {
        std::cerr << "getPortState returned " << (int)state
                  << " after calling setPortState with PoeState::Disabled"
                  << std::endl;
    }

    return 0;
}