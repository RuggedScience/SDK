#include "rsdioimpl.h"

#include <assert.h>

#include <iostream>

#include "../../error/include/rserrors.h"
#include "../../utils/tinyxml2.h"
#include "controllers/ite8783.h"
#include "controllers/ite8786.h"

#ifndef RSSDK_VERSION_STRING
#define RSSDK_VERSION_STRING "beta"
#endif

static tinyxml2::XMLError
getInternalPinInfo(const tinyxml2::XMLElement *pin, int &pinId, PinConfig &info)
{
    using namespace tinyxml2;

    int id, bit, gpio;
    bool pullup = false;
    XMLError e = pin->QueryAttribute("id", &id);
    if (e != XML_SUCCESS) return e;
    e = pin->QueryAttribute("bit", &bit);
    if (e != XML_SUCCESS) return e;
    e = pin->QueryAttribute("gpio", &gpio);
    if (e != XML_SUCCESS) return e;
    e = pin->QueryAttribute("pullup", &pullup);
    if (e != XML_SUCCESS && e != XML_NO_ATTRIBUTE) return e;

    pinId = id;
    info = PinConfig(bit, gpio, false, pullup, false, true);
    return XML_SUCCESS;
}

static tinyxml2::XMLError
getExternalPinInfo(const tinyxml2::XMLElement *pin, int &pinId, PinConfig &info)
{
    using namespace tinyxml2;

    int id, bit, gpio;
    bool invert, input, output, pullup = false;
    XMLError e = pin->QueryAttribute("id", &id);
    if (e != XML_SUCCESS) return e;
    e = pin->QueryAttribute("bit", &bit);
    if (e != XML_SUCCESS) return e;
    e = pin->QueryAttribute("gpio", &gpio);
    if (e != XML_SUCCESS) return e;
    e = pin->QueryAttribute("invert", &invert);
    if (e != XML_SUCCESS) return e;
    e = pin->QueryAttribute("input", &input);
    if (e != XML_SUCCESS) return e;
    e = pin->QueryAttribute("output", &output);
    if (e != XML_SUCCESS) return e;
    e = pin->QueryAttribute("pullup", &pullup);
    if (e != XML_SUCCESS && e != XML_NO_ATTRIBUTE) return e;

    pinId = id;
    info = PinConfig(bit, gpio, invert, pullup, input, output);
    return XML_SUCCESS;
}

static tinyxml2::XMLError
get8786RegData(const tinyxml2::XMLElement *reg, Ite8786::RegisterData &data)
{
    using namespace tinyxml2;

    const char *tmp = reg->Attribute("id");
    if (!tmp) return XML_NO_ATTRIBUTE;
    data.addr = std::stoi(std::string(tmp), nullptr, 0);

    tmp = reg->Attribute("ldn");
    if (tmp)
        data.ldn = std::stoi(std::string(tmp), nullptr, 0);
    else
        data.ldn = 0;

    tmp = reg->Attribute("onBits");
    if (tmp)
        data.onBits = std::stoi(std::string(tmp), nullptr, 0);
    else
        data.onBits = 0;

    tmp = reg->Attribute("offBits");
    if (tmp)
        data.offBits = std::stoi(std::string(tmp), nullptr, 0);
    else
        data.offBits = 0;

    return XML_SUCCESS;
}

static rs::PinDirection modeToDirection(PinMode mode)
{
    return mode == PinMode::ModeInput ? rs::PinDirection::Input
                                      : rs::PinDirection::Output;
}

static PinMode directionToMode(rs::PinDirection dir)
{
    return dir == rs::PinDirection::Input ? PinMode::ModeInput
                                          : PinMode::ModeOutput;
}

RsDioImpl::RsDioImpl()
    : m_lastError(), m_lastErrorString(), mp_controller(nullptr)
{
}

RsDioImpl::RsDioImpl(AbstractDioController *controller, dioconfigmap_t dioMap)
    : m_lastError(),
      m_lastErrorString(),
      m_dioMap(dioMap),
      mp_controller(controller)
{
    for (auto &dio : m_dioMap) {
        auto pinMap = dio.second;
        for (auto &pin : pinMap) {
            controller->initPin(pin.second);
        }
    }
}

RsDioImpl::~RsDioImpl() { delete mp_controller; }

void RsDioImpl::destroy() { delete this; }

void RsDioImpl::setXmlFile(const char *fileName, bool debug)
{
    using namespace tinyxml2;
    m_dioMap.clear();
    if (mp_controller) delete mp_controller;
    mp_controller = nullptr;

    XMLDocument doc;
    if (doc.LoadFile(fileName) != XML_SUCCESS) {
        if (doc.ErrorID() == XML_ERROR_FILE_NOT_FOUND) {
            m_lastError =
                std::make_error_code(std::errc::no_such_file_or_directory);
            m_lastErrorString = std::string(fileName) + " not found";
        }
        else {
            m_lastError = RsErrorCode::XmlParseError;
            m_lastErrorString = doc.ErrorStr();
        }

        return;
    }

    XMLElement *comp = doc.FirstChildElement("computer");
    if (!comp) {
        m_lastError = RsErrorCode::XmlParseError;
        m_lastErrorString = "Missing computer node";
        return;
    }

    XMLElement *dio = comp->FirstChildElement("dio_controller");
    if (!dio) {
        m_lastError = std::make_error_code(std::errc::function_not_supported);
        m_lastErrorString = "DIO functionality not supported";
        return;
    }

    std::string id(dio->Attribute("id"));
    try {
        if (debug) {
            std::cout << "XML DIO Controller ID: " << id << std::endl;
        }

        if (id == "ite8783") {
            mp_controller = new Ite8783(debug);
        }
        else if (id == "ite8786") {
            Ite8786::RegisterList_t list;
            XMLElement *reg = dio->FirstChildElement("register");
            for (; reg; reg = reg->NextSiblingElement("register")) {
                Ite8786::RegisterData data;
                if (get8786RegData(reg, data) == XML_SUCCESS)
                    list.emplace_back(data);
            }
            mp_controller = new Ite8786(list, debug);
        }
        else {
            m_lastError = RsErrorCode::XmlParseError;
            m_lastErrorString = "Invalid DIO controller ID";
            return;
        }
    }
    catch (const std::system_error &ex) {
        m_lastError = ex.code();
        m_lastErrorString = ex.what();
        return;
    }
    catch (const std::exception &ex) {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = ex.what();
        return;
    }
    catch (...) {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = "Unknown exception occurred";
        return;
    }

    // Print the registers before we initialize all the pins.
    if (debug) {
        mp_controller->printRegs();
    }

    XMLElement *con = dio->FirstChildElement("connector");
    // No connectors means this unit doesn't support DIO.
    // Assuming it's a legit XML file...
    if (!con) {
        m_dioMap.clear();
        delete mp_controller;
        mp_controller = nullptr;

        m_lastError = std::make_error_code(std::errc::function_not_supported);
        m_lastErrorString = "DIO function not supported";
        return;
    }

    for (; con; con = con->NextSiblingElement("connector")) {
        int conId = 0;
        if (con->QueryAttribute("id", &conId) == XML_SUCCESS) {
            XMLElement *ip = con->FirstChildElement("internal_pin");
            for (; ip; ip = ip->NextSiblingElement("internal_pin")) {
                int pinId;
                PinConfig info;
                if (getInternalPinInfo(ip, pinId, info) == XML_SUCCESS) {
                    mp_controller->initPin(info);
                    m_dioMap[conId][pinId] = info;
                }
            }

            XMLElement *ep = con->FirstChildElement("external_pin");
            for (; ep; ep = ep->NextSiblingElement("external_pin")) {
                int pinId;
                PinConfig info;
                if (getExternalPinInfo(ep, pinId, info) == XML_SUCCESS) {
                    mp_controller->initPin(info);
                    m_dioMap[conId][pinId] = info;
                }
            }
        }
    }

    // Print the registers again after all the pins have been initialized.
    if (debug) mp_controller->printRegs();

    if (m_dioMap.size() <= 0) {
        m_dioMap.clear();
        delete mp_controller;
        mp_controller = nullptr;

        m_lastError = RsErrorCode::XmlParseError;
        m_lastErrorString = "Found DIO connector node but no pins";
        return;
    }

    // Set the output mode of each dio if it's not already a valid mode.
    dioconfigmap_t::iterator it;
    for (it = m_dioMap.begin(); it != m_dioMap.end(); ++it) {
        const int modeSink = static_cast<int>(rs::OutputMode::Sink);
        const int modeSource = static_cast<int>(rs::OutputMode::Source);

        pinconfigmap_t pinMap = it->second;
        // Not all units support programmable Source/Sink modes so if these pins
        // don't exist we don't really care.
        if (pinMap.find(modeSink) != pinMap.end() &&
            pinMap.find(modeSource) != pinMap.end()) {
            PinConfig sink = pinMap[modeSink];
            PinConfig source = pinMap[modeSource];

            try {
                // If these two pins are in the same state the dio will not
                // operate. Let's fix that.
                if (mp_controller->getPinState(sink) ==
                    mp_controller->getPinState(source)) {
                    mp_controller->setPinState(sink, true);
                    mp_controller->setPinState(source, false);
                }
            }
            catch (const std::system_error &ex) {
                m_dioMap.clear();
                delete mp_controller;
                mp_controller = nullptr;

                m_lastError = ex.code();
                m_lastErrorString = ex.what();
                return;
            }
            catch (const std::exception &ex) {
                m_dioMap.clear();
                delete mp_controller;
                m_lastError = RsErrorCode::UnknownError;
                m_lastErrorString = ex.what();
                return;
            }
            catch (...) {
                m_dioMap.clear();
                delete mp_controller;
                m_lastError = RsErrorCode::UnknownError;
                m_lastErrorString = "Unknown exception occurred";
                return;
            }
        }
    }

    m_lastError = std::error_code();
}

rs::diomap_t RsDioImpl::getPinList() const
{
    rs::diomap_t dios;
    dioconfigmap_t::const_iterator dioIt;
    for (dioIt = m_dioMap.begin(); dioIt != m_dioMap.end(); ++dioIt) {
        rs::pinmap_t pins;
        pinconfigmap_t::const_iterator pinIt;
        for (pinIt = dioIt->second.begin(); pinIt != dioIt->second.end();
             ++pinIt) {
            if (pinIt->first >= 0) {
                PinConfig config = pinIt->second;
                rs::PinInfo info(config.supportsInput, config.supportsOutput);
                pins[pinIt->first] = info;
            }
        }

        dios[dioIt->first] = pins;
    }

    return dios;
}

bool RsDioImpl::canSetOutputMode(int dio)
{
    bool supported = false;

    if (m_dioMap.find(dio) == m_dioMap.end()) {
        m_lastError = std::make_error_code(std::errc::invalid_argument);
        m_lastErrorString = "Invalid DIO";
        return supported;
    }

    pinconfigmap_t pinMap = m_dioMap.at(dio);
    const int modeSink = static_cast<int>(rs::OutputMode::Sink);
    const int modeSource = static_cast<int>(rs::OutputMode::Source);
    if (pinMap.find(modeSink) == pinMap.end() ||
        pinMap.find(modeSource) == pinMap.end())
        supported = false;
    else
        supported = true;

    m_lastError = std::error_code();
    return supported;
}

void RsDioImpl::setOutputMode(int dio, rs::OutputMode mode)
{
    if (mp_controller == nullptr) {
        m_lastError = RsErrorCode::NotInitialized;
        m_lastErrorString = "XML file never set";
        return;
    }

    if (m_dioMap.find(dio) == m_dioMap.end()) {
        m_lastError = std::make_error_code(std::errc::invalid_argument);
        m_lastErrorString = "Invalid DIO";
        return;
    }

    pinconfigmap_t pinMap = m_dioMap.at(dio);
    const int modeSink = static_cast<int>(rs::OutputMode::Sink);
    const int modeSource = static_cast<int>(rs::OutputMode::Source);
    if (pinMap.find(modeSink) == pinMap.end() ||
        pinMap.find(modeSource) == pinMap.end()) {
        m_lastError = std::make_error_code(std::errc::function_not_supported);
        m_lastErrorString = "Setting output mode not supported";
        return;
    }

    try {
        mp_controller->setPinState(
            pinMap.at(modeSink), (mode == rs::OutputMode::Sink)
        );
        mp_controller->setPinState(
            pinMap.at(modeSource), (mode == rs::OutputMode::Source)
        );
        m_lastError = std::error_code();
    }
    catch (const std::system_error &ex) {
        m_lastError = ex.code();
        m_lastErrorString = ex.what();
    }
    catch (const std::exception &ex) {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = ex.what();
    }
    catch (...) {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = "Unknown exception occured";
    }
}

rs::OutputMode RsDioImpl::getOutputMode(int dio)
{
    rs::OutputMode mode = rs::OutputMode::Sink;
    if (mp_controller == nullptr) {
        m_lastError = RsErrorCode::NotInitialized;
        m_lastErrorString = "XML file never set";
        return mode;
    }

    if (m_dioMap.find(dio) == m_dioMap.end()) {
        m_lastError = std::make_error_code(std::errc::invalid_argument);
        m_lastErrorString = "Invalid DIO";
        return mode;
    }

    pinconfigmap_t pinMap = m_dioMap.at(dio);
    const int modeSink = static_cast<int>(rs::OutputMode::Sink);
    const int modeSource = static_cast<int>(rs::OutputMode::Source);
    if (pinMap.find(modeSink) == pinMap.end() ||
        pinMap.find(modeSource) == pinMap.end()) {
        m_lastError = std::make_error_code(std::errc::function_not_supported);
        m_lastErrorString = "Setting output mode not supported";
        return mode;
    }

    try {
        bool sink = mp_controller->getPinState(pinMap.at(modeSink));
        bool source = mp_controller->getPinState(pinMap.at(modeSource));

        m_lastError = std::error_code();

        if (sink && !source) {
            mode = rs::OutputMode::Sink;
        }
        else if (source && !sink) {
            mode = rs::OutputMode::Source;
        }
        else {
            // TODO: How should be handle this?
            // If sink and source are equal then both chips will be disabled
            // and outputs will not work at all. Throw error or force a mode?
            m_lastError = RsErrorCode::UnknownError;
            m_lastErrorString =
                "Unexpected output mode found. Try calling setOutputMode to "
                "fix it";
        }
    }
    catch (const std::system_error &ex) {
        m_lastError = ex.code();
        m_lastErrorString = ex.what();
    }
    catch (const std::exception &ex) {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = ex.what();
    }
    catch (...) {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = "Unknown exception occured";
    }

    return mode;
}

bool RsDioImpl::digitalRead(int dio, int pin)
{
    bool state = false;

    if (mp_controller == nullptr) {
        m_lastError = RsErrorCode::NotInitialized;
        m_lastErrorString = "XML file never set";
        return state;
    }

    if (m_dioMap.find(dio) == m_dioMap.end()) {
        m_lastError = std::make_error_code(std::errc::invalid_argument);
        m_lastErrorString = "Invalid DIO";
        return state;
    }

    pinconfigmap_t pinMap = m_dioMap.at(dio);
    if (pinMap.find(pin) == pinMap.end()) {
        m_lastError = std::make_error_code(std::errc::invalid_argument);
        m_lastErrorString = "Invalid pin";
        return state;
    }

    PinConfig config = pinMap.at(pin);

    try {
        state = mp_controller->getPinState(config);
        m_lastError = std::error_code();
    }
    catch (const std::system_error &ex) {
        m_lastError = ex.code();
        m_lastErrorString = ex.what();
    }
    catch (const std::exception &ex) {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = ex.what();
    }
    catch (...) {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = "unknown exception occured";
    }

    return state;
}

void RsDioImpl::digitalWrite(int dio, int pin, bool state)
{
    if (mp_controller == nullptr) {
        m_lastError = RsErrorCode::NotInitialized;
        m_lastErrorString = "XML file never set";
        return;
    }

    if (m_dioMap.find(dio) == m_dioMap.end()) {
        m_lastError = std::make_error_code(std::errc::invalid_argument);
        m_lastErrorString = "Invalid DIO";
        return;
    }

    pinconfigmap_t pinMap = m_dioMap.at(dio);
    if (pin < 0 || pinMap.find(pin) == pinMap.end()) {
        m_lastError = std::make_error_code(std::errc::invalid_argument);
        m_lastErrorString = "Invalid pin";
        return;
    }

    PinConfig config = pinMap.at(pin);
    if (!config.supportsOutput) {
        m_lastError = std::make_error_code(std::errc::function_not_supported);
        m_lastErrorString = "Pin does not support output mode";
        return;
    }

    try {
        mp_controller->setPinState(config, state);
        m_lastError = std::error_code();
    }
    catch (const std::system_error &ex) {
        m_lastError = ex.code();
        m_lastErrorString = ex.what();
    }
    catch (const std::exception &ex) {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = ex.what();
    }
    catch (...) {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = "unknown exception occured";
    }
}

void RsDioImpl::setPinDirection(int dio, int pin, rs::PinDirection dir)
{
    if (mp_controller == nullptr) {
        m_lastError = RsErrorCode::NotInitialized;
        m_lastErrorString = "XML file never set";
        return;
    }

    if (m_dioMap.find(dio) == m_dioMap.end()) {
        m_lastError = std::make_error_code(std::errc::invalid_argument);
        m_lastErrorString = "Invalid DIO";
        return;
    }

    pinconfigmap_t pinMap = m_dioMap.at(dio);
    if (pin < 0 || pinMap.find(pin) == pinMap.end()) {
        m_lastError = std::make_error_code(std::errc::invalid_argument);
        m_lastErrorString = "Invalid pin";
        return;
    }

    PinConfig config = pinMap.at(pin);
    if (getPinDirection(dio, pin) == dir)
    {
        m_lastError = std::error_code();
        return;
    }

    if (!config.supportsInput || !config.supportsOutput) {

    }
    if ((dir == rs::PinDirection::Input && !config.supportsInput) ||
        (dir == rs::PinDirection::Output && !config.supportsOutput)) {
        m_lastError = std::make_error_code(std::errc::function_not_supported);

        m_lastErrorString = "Pin does not support direction: ";
        if (dir == rs::PinDirection::Input)
            m_lastErrorString += "Input";
        else
            m_lastErrorString += "Output";
        return;
    }

    try {
        mp_controller->setPinMode(config, directionToMode(dir));
        m_lastError = std::error_code();
    }
    catch (const std::system_error &ex) {
        m_lastError = ex.code();
        m_lastErrorString = ex.what();
    }
    catch (const std::exception &ex) {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = ex.what();
    }
    catch (...) {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = "unknown exception occured";
    }
}

rs::PinDirection RsDioImpl::getPinDirection(int dio, int pin)
{
    rs::PinDirection dir = rs::PinDirection::Input;
    if (mp_controller == nullptr) {
        m_lastError = RsErrorCode::NotInitialized;
        m_lastErrorString = "XML file never set";
        return dir;
    }

    if (m_dioMap.find(dio) == m_dioMap.end()) {
        m_lastError = std::make_error_code(std::errc::invalid_argument);
        m_lastErrorString = "Invalid DIO";
        return dir;
    }

    pinconfigmap_t pinMap = m_dioMap.at(dio);
    if (pin < 0 || pinMap.find(pin) == pinMap.end()) {
        m_lastError = std::make_error_code(std::errc::invalid_argument);
        m_lastErrorString = "Invalid pin";
        return dir;
    }

    PinConfig config = pinMap.at(pin);

    try {
        dir = modeToDirection(mp_controller->getPinMode(config));
        m_lastError = std::error_code();
    }
    catch (const std::system_error &ex) {
        m_lastError = ex.code();
        m_lastErrorString = ex.what();
    }
    catch (const std::exception &ex) {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = ex.what();
    }
    catch (...) {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = "unknown exception occured";
    }

    return dir;
}

std::map<int, bool> RsDioImpl::readAll(int dio)
{
    std::map<int, bool> values;

    if (mp_controller == nullptr) {
        m_lastError = RsErrorCode::NotInitialized;
        m_lastErrorString = "XML file never set";
        return values;
    }

    if (m_dioMap.find(dio) == m_dioMap.end()) {
        m_lastError = std::make_error_code(std::errc::invalid_argument);
        m_lastErrorString = "Invalid DIO";
        return values;
    }

    pinconfigmap_t pinMap = m_dioMap.at(dio);

    try {
        for (const auto &pin : pinMap) {
            if (pin.first >= 0)
                values[pin.first] = mp_controller->getPinState(pin.second);
        }
        m_lastError = std::error_code();
    }
    catch (const std::system_error &ex) {
        m_lastError = ex.code();
        m_lastErrorString = ex.what();
    }
    catch (const std::exception &ex) {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = ex.what();
    }
    catch (...) {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = "unknown exception occured";
    }

    return values;
}

std::error_code RsDioImpl::getLastError() const { return m_lastError; }

std::string RsDioImpl::getLastErrorString() const
{
    std::string lastError;

    if (m_lastError) {
        lastError += m_lastError.message();
        if (!m_lastErrorString.empty()) {
            lastError += ": " + m_lastErrorString;
        }
    }
    return lastError;
}

rs::RsDio *rs::createRsDio() { return new RsDioImpl; }

const char *rs::rsDioVersion() { return RSSDK_VERSION_STRING; }
