#include "rspoeimpl.h"
#include "poecontroller.h"

#include <iostream>


int main()
{
    const std::vector<uint8_t> internal_ports = {0, 1, 2, 3};

    PoeControllerConfig controllerConfig;
    int external_port = 1;
    for (uint8_t port : internal_ports) {
        controllerConfig.ports.push_back({external_port++, port});
    }

    TestPoeController *controller = new TestPoeController(100, internal_ports);
    RsPoeImpl poe(controller, controllerConfig);

    poe.getPortState(5);

    poe.getPortState(1);

    poe.setPortState(5, rs::PoeState::Auto);

    poe.setPortState(1, rs::PoeState::Auto);

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