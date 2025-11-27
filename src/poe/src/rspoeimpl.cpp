#include "rspoeimpl.h"
#include "rserrors.h"
#include "configparser.h"
#include "controllers/ltc4266.h"
#include "controllers/pd69104.h"
#include "controllers/pd69200.h"

#ifndef RSSDK_VERSION_STRING
#define RSSDK_VERSION_STRING "beta"
#endif

RsPoeImpl::RsPoeImpl()
    : mp_controller(nullptr)
{
}

RsPoeImpl::RsPoeImpl(AbstractPoeController *controller, PoeControllerConfig config)
    : m_config(config), mp_controller(controller)
{
}

RsPoeImpl::~RsPoeImpl() { delete mp_controller; }

void RsPoeImpl::destroy() { delete this; }

void RsPoeImpl::init(const char *configFile)
{
    m_config = PoeControllerConfig();

    delete mp_controller;
    mp_controller = nullptr;

    ConfigParser parser;
    parser.parse(configFile);
    std::vector<PoeControllerConfig> configs = parser.getPoeControllerConfigs();
    if (configs.size() == 0) {
        throw rs::RsException(rs::RsErrorCode::UnsupportedFunction, "PoE functionality not supported");
    }
    else if (configs.size() > 1) {
        fprintf(
            stderr, "Multiple PoE controllers found in config. Using first one."
        );
    }

    PoeControllerConfig controller = configs.at(0);

    try {
        if (controller.controllerId == "pd69104")
            mp_controller = new Pd69104(controller.busAddr, controller.chipAddr);
        else if (controller.controllerId == "pd69200")
            mp_controller = new Pd69200(controller.busAddr, controller.chipAddr);
        else if (controller.controllerId == "ltc4266")
            mp_controller = new Ltc4266(controller.busAddr, controller.chipAddr);
        else {
            throw rs::RsException(rs::RsErrorCode::ConfigParseError, "Invalid PoE controller ID");
        }
    }
    catch (...) {
        rs::rethrowAsRsException();
    }
}

std::vector<int> RsPoeImpl::getPortList() const
{
    std::vector<int> keys;
    for (const auto &portConfig : m_config.ports) {
        keys.push_back(portConfig.id);
    }
    return keys;
}

rs::PoeState RsPoeImpl::getPortState(int port)
{
    ensureInitialized();
    PoePortConfig portConfig = getPortConfig(port);
    try {
        return mp_controller->getPortState(portConfig.offset);
    }
    catch (...) {
        rs::rethrowAsRsException();
    }
}

void RsPoeImpl::setPortState(int port, rs::PoeState state)
{
    ensureInitialized();
    PoePortConfig portConfig = getPortConfig(port);
    try {
        mp_controller->setPortState(portConfig.offset, state);
    }
    catch (...) {
        rs::rethrowAsRsException();
    }
}

float RsPoeImpl::getPortVoltage(int port)
{
    ensureInitialized();
    PoePortConfig portConfig = getPortConfig(port);
    try {
        return mp_controller->getPortVoltage(portConfig.offset);
    }
    catch (...) {
        rs::rethrowAsRsException();
    }
}

float RsPoeImpl::getPortCurrent(int port)
{
    ensureInitialized();
    PoePortConfig portConfig = getPortConfig(port);

    try {
        return mp_controller->getPortCurrent(portConfig.offset);
    }
    catch (...) {
        rs::rethrowAsRsException();
    }
}

float RsPoeImpl::getPortPower(int port)
{
    ensureInitialized();
    PoePortConfig portConfig = getPortConfig(port);

    try {
        return mp_controller->getPortPower(portConfig.offset);
    }
    catch (...) {
        rs::rethrowAsRsException();
    }
}

int RsPoeImpl::getBudgetConsumed()
{
    ensureInitialized();

    try {
        return mp_controller->getBudgetConsumed();
    }
    catch (...) {
        rs::rethrowAsRsException();
    }
}

int RsPoeImpl::getBudgetAvailable()
{
    ensureInitialized();

    try {
        return mp_controller->getBudgetAvailable();
    }
    catch (...) {
        rs::rethrowAsRsException();
    }
}

int RsPoeImpl::getBudgetTotal()
{
    ensureInitialized();

    try {
        return mp_controller->getBudgetTotal();
    }
    catch (...) {
        rs::rethrowAsRsException();
    }
}

void RsPoeImpl::ensureInitialized() const
{
    if (mp_controller == nullptr) {
        throw rs::RsException(rs::RsErrorCode::NotInitialized, "XML file never set");
    }
}

const PoePortConfig & RsPoeImpl::getPortConfig(int port) const
{
    for (const auto &portConfig : m_config.ports) {
        if (portConfig.id == port) {
            return portConfig;
        }
    }
    throw rs::RsException(rs::RsErrorCode::InvalidParameter, "Invalid port nuber");
}