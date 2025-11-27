#include "rsdioimpl.h"
#include "rserrors.h"
#include "configparser.h"
#include "controllers/ite8783.h"
#include "controllers/ite8786.h"

#include <assert.h>
#include <iostream>

#ifndef RSSDK_VERSION_STRING
#define RSSDK_VERSION_STRING "beta"
#endif

RsDioImpl::RsDioImpl()
    : mp_controller(nullptr)
{
}

RsDioImpl::RsDioImpl(
    AbstractDioController* controller,
    DioControllerConfig config
)
    : mp_controller(controller), m_config(config)
{
    for (const auto& connector : config.connectors) {
        for (const auto& pin : connector.pins) {
            mp_controller->initPin(pin);
        }
    }
}

RsDioImpl::~RsDioImpl() { delete mp_controller; }

void RsDioImpl::destroy() { delete this; }

void RsDioImpl::init(const char* configFile)
{
    m_config = DioControllerConfig();
    if (mp_controller) delete mp_controller;
    mp_controller = nullptr;

    ConfigParser parser;
    parser.parse(configFile);
    std::vector<DioControllerConfig> configs = parser.getDioControllerConfigs();
    if (configs.size() == 0) {
        throw rs::RsException(rs::RsErrorCode::UnsupportedFunction, "DIO functionality not supported");
    }
    else if (configs.size() > 1) {
        fprintf(
            stderr, "Multiple DIO controllers found in config. Using first one."
        );
    }

    DioControllerConfig controller = configs.at(0);

    try {
        if (controller.controllerId == "ite8783") {
            mp_controller = new Ite8783();
        }
        else if (controller.controllerId == "ite8786") {
            mp_controller = new Ite8786(controller.registers);
        }
        else {
            throw rs::RsException(rs::RsErrorCode::ConfigParseError, "Invalid DIO controller ID");
        }
    }
    catch (...) {
        rs::rethrowAsRsException();
    }

    const DioPinConfig* sinkPin = nullptr;
    const DioPinConfig* sourcePin = nullptr;
    const int modeSink = static_cast<int>(rs::OutputMode::Sink);
    const int modeSource = static_cast<int>(rs::OutputMode::Source);
    for (const DioConnectorConfig& connector : controller.connectors) {
        for (const DioPinConfig& pin : connector.pins) {
            mp_controller->initPin(pin);
            if (pin.id == modeSink) {
                sinkPin = &pin;
            }
            else if (pin.id == modeSource) {
                sourcePin = &pin;
            }
        }
    }

    if (sinkPin && sourcePin) {
        try {
            // If these two pins are in the same state the dio will not operate.
            // Let's fix that.
            if (mp_controller->getPinState(*sinkPin) ==
                mp_controller->getPinState(*sourcePin)) {
                mp_controller->setPinState(*sinkPin, true);
                mp_controller->setPinState(*sourcePin, false);
            }
        }
        catch (...) {
            rs::rethrowAsRsException();
        }
    }
    m_config = controller;
}

rs::diomap_t RsDioImpl::getPinList() const
{
    ensureInitialized();

    rs::diomap_t dios;
    for (const auto& connector : m_config.connectors) {
        rs::pinmap_t pins;
        for (const auto& pin : connector.pins) {
            if (pin.id >= 0) {
                rs::PinInfo info(pin.supportsInput, pin.supportsOutput);
                pins[pin.id] = info;
            }
        }
        dios[connector.connectorId] = pins;
    }

    return dios;
}

bool RsDioImpl::canSetOutputMode(int dio)
{
    ensureInitialized();

    DioConnectorConfig connectorConfig = getConnectorConfig(dio);
    return connectorConfig.supportsOutputConfig();
}

void RsDioImpl::setOutputMode(int dio, rs::OutputMode mode)
{
    ensureInitialized();

    DioConnectorConfig connectorConfig = getConnectorConfig(dio);
    if (!connectorConfig.supportsOutputConfig()) {
        throw rs::RsException(rs::RsErrorCode::UnsupportedFunction, "Setting output mode not supported");
    }

    try {
        mp_controller->setPinState(connectorConfig.sinkPin, (mode == rs::OutputMode::Sink));
        mp_controller->setPinState(connectorConfig.sourcePin, (mode == rs::OutputMode::Source));
    }
    catch (...) {
        rs::rethrowAsRsException();
    }
}

rs::OutputMode RsDioImpl::getOutputMode(int dio)
{
    ensureInitialized();
    DioConnectorConfig connectorConfig = getConnectorConfig(dio);
    if (!connectorConfig.supportsOutputConfig()) {
        throw rs::RsException(rs::RsErrorCode::UnsupportedFunction, "Setting output mode not supported");
    }

    try {
        bool sink = mp_controller->getPinState(connectorConfig.sinkPin);
        bool source = mp_controller->getPinState(connectorConfig.sourcePin);

        if (sink && !source) {
            return rs::OutputMode::Sink;
        }
        else if (source && !sink) {
            return rs::OutputMode::Source;
        }
        else {
            // TODO: How should be handle this?
            // If sink and source are equal then both chips will be disabled
            // and outputs will not work at all. Throw error or force a mode?
            throw rs::RsException(rs::RsErrorCode::HardwareError, "Unexpected output mode found. Try calling setOutputMode to fix it");
        }
    }
    catch (...) {
        rs::rethrowAsRsException();
    }
}

bool RsDioImpl::digitalRead(int dio, int pin)
{
    ensureInitialized();
    DioPinConfig pinConfig = getPinConfig(dio, pin);

    try {
        return mp_controller->getPinState(pinConfig);
    }
    catch (...) {
        rs::rethrowAsRsException();
    }
}

void RsDioImpl::digitalWrite(int dio, int pin, bool state)
{
    ensureInitialized();

    DioPinConfig pinConfig = getPinConfig(dio, pin);
    if (!pinConfig.supportsOutput) {
        throw rs::RsException(rs::RsErrorCode::UnsupportedFunction, "Pin does not support output mode");
    }

    try {
        mp_controller->setPinState(pinConfig, state);
    }
    catch (...) {
        rs::rethrowAsRsException();
    }
}

void RsDioImpl::setPinDirection(int dio, int pin, rs::PinDirection dir)
{
    ensureInitialized();
    DioPinConfig pinConfig = getPinConfig(dio, pin);

    if (getPinDirection(dio, pin) == dir) {
        return;
    }

    if ((dir == rs::PinDirection::Input && !pinConfig.supportsInput) ||
        (dir == rs::PinDirection::Output && !pinConfig.supportsOutput)) {
        std::string dirString = dir == rs::PinDirection::Input ? "Input" : "Output";
        throw rs::RsException(rs::RsErrorCode::UnsupportedFunction, "Pin does not support direction: " + dirString);
    }

    try {
        mp_controller->setPinMode(pinConfig, dir);
    }
    catch (...) {
        rs::rethrowAsRsException();
    }
}

rs::PinDirection RsDioImpl::getPinDirection(int dio, int pin)
{
    ensureInitialized();
    DioPinConfig pinConfig = getPinConfig(dio, pin);

    try {
        return mp_controller->getPinMode(pinConfig);
    }
    catch (...) {
        rs::rethrowAsRsException();
    }
}

std::map<int, bool> RsDioImpl::readAll(int dio)
{
    ensureInitialized();

    std::map<int, bool> values;
    DioConnectorConfig connectorConfig = getConnectorConfig(dio);

    try {
        for (const auto& pinConfig : connectorConfig.pins) {
            values[pinConfig.id] = mp_controller->getPinState(pinConfig);
        }
    }
    catch (...) {
        rs::rethrowAsRsException();
    }

    return values;
}

void RsDioImpl::ensureInitialized() const
{
    if (mp_controller == nullptr) {
        throw rs::RsException(rs::RsErrorCode::NotInitialized, "XML file never set");
    }
}

const DioConnectorConfig & RsDioImpl::getConnectorConfig(int dio) const
{
    for (const auto& connector : m_config.connectors) {
        if (connector.connectorId == dio) {
            return connector;
        }
    }
    throw rs::RsException(rs::RsErrorCode::InvalidParameter, "Invalid DIO number");
}

const DioPinConfig & RsDioImpl::getPinConfig(int dio, int pin) const
{
    DioConnectorConfig connectorConfig = getConnectorConfig(dio);
    return getPinConfig(connectorConfig, pin);
}

const DioPinConfig & RsDioImpl::getPinConfig(const DioConnectorConfig& dio, int pin) const
{
    for (const auto& pinConfig : dio.pins) {
        if (pinConfig.id == pin) {
            return pinConfig;
        }
    }
    throw rs::RsException(rs::RsErrorCode::InvalidParameter, "Invalid pin number");
}