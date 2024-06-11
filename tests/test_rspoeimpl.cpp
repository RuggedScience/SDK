#include <iostream>

#include "../poe/src/rspoeimpl.h"
#include "poecontroller.h"
#include "utils.h"

int main()
{
    const std::vector<int> external_ports = {1, 2, 3, 4};
    const std::vector<uint8_t> internal_ports = {0, 1, 2, 3};
    portmap_t portMap = {
        {external_ports[0], internal_ports[0]},
        {external_ports[1], internal_ports[1]},
        {external_ports[2], internal_ports[2]},
        {external_ports[3], internal_ports[3]},
    };

    TestPoeController *controller = new TestPoeController(100, internal_ports);
    RsPoeImpl poe(controller, portMap);

    if (poe.getPortList() != external_ports) {
        std::cerr << "getPortList returned invalid ports" << std::endl;
        return 1;
    }

    poe.getPortState(5);
    verifyError(
        "getPortState (invalid port)",
        poe.getLastError(),
        std::errc::invalid_argument
    );

    poe.getPortState(1);
    verifyError("getPortState (valid)", poe.getLastError());

    poe.setPortState(5, rs::PoeState::Auto);
    verifyError(
        "setPortState (invalid port)",
        poe.getLastError(),
        std::errc::invalid_argument
    );

    poe.setPortState(1, rs::PoeState::Error);
    verifyError(
        "setPortState (invalid state)",
        poe.getLastError(),
        std::errc::invalid_argument
    );

    poe.setPortState(1, rs::PoeState::Auto);
    verifyError("setPortState (valid)", poe.getLastError());

    rs::PoeState state;
    state = poe.getPortState(1);
    if (state != rs::PoeState::Auto) {
        std::cerr << "getPortState returned " << (int)state
                  << " after calling setPortState with PoeState::Auto"
                  << std::endl;
    }

    poe.setPortState(1, rs::PoeState::Disabled);
    state = poe.getPortState(1);
    if (state != rs::PoeState::Disabled) {
        std::cerr << "getPortState returned " << (int)state
                  << " after calling setPortState with PoeState::Disabled"
                  << std::endl;
    }

    return 0;
}