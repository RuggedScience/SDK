#include "rspoeimpl.h"

#include "configparser.h"
#include "controllers/ltc4266.h"
#include "controllers/pd69104.h"
#include "controllers/pd69200.h"
#include "rserrors.h"

#ifndef RSSDK_VERSION_STRING
#define RSSDK_VERSION_STRING "beta"
#endif

RsPoeImpl::RsPoeImpl() : m_controller() {}

RsPoeImpl::RsPoeImpl(
    std::unique_ptr<AbstractPoeController> controller,
    PoeControllerConfig config
)
    : m_config(config), m_controller(std::move(controller))
{
}

void RsPoeImpl::destroy() { delete this; }

void RsPoeImpl::init(const char* configFile)
{
    m_config = PoeControllerConfig();
    m_controller.reset();

    ConfigParser parser;
    parser.parse(configFile);
    std::vector<PoeControllerConfig> configs = parser.getPoeControllerConfigs();
    if (configs.size() == 0) {
        throw rs::RsException(
            rs::RsErrorCode::UnsupportedFunction,
            "PoE functionality not supported"
        );
    }
    else if (configs.size() > 1) {
        fprintf(
            stderr, "Multiple PoE controllers found in config. Using first one."
        );
    }

    PoeControllerConfig controller = configs.at(0);

    try {
        if (controller.controllerId == "pd69104")
            m_controller.reset(
                new Pd69104(controller.busAddr, controller.chipAddr)
            );
        else if (controller.controllerId == "pd69200")
            m_controller.reset(
                new Pd69200(controller.busAddr, controller.chipAddr)
            );
        else if (controller.controllerId == "ltc4266")
            m_controller.reset(
                new Ltc4266(controller.busAddr, controller.chipAddr)
            );
        else {
            throw rs::RsException(
                rs::RsErrorCode::ConfigParseError, "Invalid PoE controller ID"
            );
        }
    }
    catch (...) {
        rs::rethrowAsRsException();
    }
}

std::vector<int> RsPoeImpl::getPortList() const
{
    std::vector<int> keys;
    for (const auto& portConfig : m_config.ports) {
        keys.push_back(portConfig.id);
    }
    return keys;
}

rs::PoeState RsPoeImpl::getPortState(int port)
{
    ensureInitialized();
    const PoePortConfig& portConfig = getPortConfig(port);
    try {
        return m_controller->getPortState(portConfig.offset);
    }
    catch (...) {
        rs::rethrowAsRsException();
    }
}

void RsPoeImpl::setPortState(int port, rs::PoeState state)
{
    ensureInitialized();
    const PoePortConfig& portConfig = getPortConfig(port);
    try {
        m_controller->setPortState(portConfig.offset, state);
    }
    catch (...) {
        rs::rethrowAsRsException();
    }
}

float RsPoeImpl::getPortVoltage(int port)
{
    ensureInitialized();
    const PoePortConfig& portConfig = getPortConfig(port);
    try {
        return m_controller->getPortVoltage(portConfig.offset);
    }
    catch (...) {
        rs::rethrowAsRsException();
    }
}

float RsPoeImpl::getPortCurrent(int port)
{
    ensureInitialized();
    const PoePortConfig& portConfig = getPortConfig(port);

    try {
        return m_controller->getPortCurrent(portConfig.offset);
    }
    catch (...) {
        rs::rethrowAsRsException();
    }
}

float RsPoeImpl::getPortPower(int port)
{
    ensureInitialized();
    const PoePortConfig& portConfig = getPortConfig(port);

    try {
        return m_controller->getPortPower(portConfig.offset);
    }
    catch (...) {
        rs::rethrowAsRsException();
    }
}

int RsPoeImpl::getBudgetConsumed()
{
    ensureInitialized();

    try {
        return m_controller->getBudgetConsumed();
    }
    catch (...) {
        rs::rethrowAsRsException();
    }
}

int RsPoeImpl::getBudgetAvailable()
{
    ensureInitialized();

    try {
        return m_controller->getBudgetAvailable();
    }
    catch (...) {
        rs::rethrowAsRsException();
    }
}

int RsPoeImpl::getBudgetTotal()
{
    ensureInitialized();

    try {
        return m_controller->getBudgetTotal();
    }
    catch (...) {
        rs::rethrowAsRsException();
    }
}

void RsPoeImpl::ensureInitialized() const
{
    if (!m_controller) {
        throw rs::RsException(
            rs::RsErrorCode::NotInitialized, "XML file never set"
        );
    }
}

const PoePortConfig& RsPoeImpl::getPortConfig(int port) const
{
    for (const auto& portConfig : m_config.ports) {
        if (portConfig.id == port) {
            return portConfig;
        }
    }
    throw rs::RsException(
        rs::RsErrorCode::InvalidParameter, "Invalid port nuber"
    );
}