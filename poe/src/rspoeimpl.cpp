#include "rspoeimpl.h"

#include "../../error/include/rserrors.h"
#include "../../utils/tinyxml2.h"
#include "controllers/ltc4266.h"
#include "controllers/pd69104.h"
#include "controllers/pd69200.h"

#ifndef RSSDK_VERSION_STRING
#define RSSDK_VERSION_STRING "beta"
#endif

RsPoeImpl::RsPoeImpl()
    : m_lastError(), m_lastErrorString(), mp_controller(nullptr)
{
}

RsPoeImpl::RsPoeImpl(AbstractPoeController *controller, portmap_t portMap)
    : m_lastError(),
      m_lastErrorString(),
      m_portMap(portMap),
      mp_controller(controller)
{
}

RsPoeImpl::~RsPoeImpl() { delete mp_controller; }

void RsPoeImpl::destroy() { delete this; }

void RsPoeImpl::setXmlFile(const char *fileName)
{
    using namespace tinyxml2;
    m_portMap.clear();
    delete mp_controller;
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

    XMLElement *poe = comp->FirstChildElement("poe_controller");
    if (!poe) {
        m_lastError = std::make_error_code(std::errc::function_not_supported);
        m_lastErrorString = "PoE functionality not supported";
        return;
    }

    std::string id(poe->Attribute("id"));

    // Try to load the new XML version with the chip_address attribute
    const char *chipAddressStr = poe->Attribute("chip_address");
    if (!chipAddressStr) {
        // If it doesn't work try the old XML version.
        chipAddressStr = poe->Attribute("address");
        if (!chipAddressStr) {
            m_lastError = RsErrorCode::XmlParseError;
            m_lastErrorString = "Missing address attribute for poe_controller";
            return;
        }
    }
    int chipAddress = std::stoi(std::string(chipAddressStr), nullptr, 0);

    // Default to the old bus address
    // We don't throw an error when the bus address is missing.
    // This is to keep old XML files working.
    int busAddress = 0xF040;
    if (busAddress) {
        busAddress =
            std::stoi(std::string(poe->Attribute("bus_address")), nullptr, 0);
    }

    try {
        if (id == "pd69104")
            mp_controller = new Pd69104(busAddress, chipAddress);
        else if (id == "pd69200")
            mp_controller = new Pd69200(busAddress, chipAddress);
        else if (id == "ltc4266")
            mp_controller = new Ltc4266(busAddress, chipAddress);
        else {
            m_lastError = RsErrorCode::XmlParseError;
            m_lastErrorString = "Invalid PoE controller ID";
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

    XMLElement *port = poe->FirstChildElement("port");
    for (; port; port = port->NextSiblingElement("port")) {
        int id, bit;
        if (port->QueryAttribute("id", &id) != XML_SUCCESS) continue;

        if (port->QueryAttribute("bit", &bit) != XML_SUCCESS) continue;

        m_portMap[id] = bit;
    }

    if (m_portMap.size() <= 0) {
        m_portMap.clear();
        delete mp_controller;
        mp_controller = nullptr;

        m_lastError = std::make_error_code(std::errc::function_not_supported);
        m_lastErrorString = "PoE function not supported";
        return;
    }

    m_lastError = std::error_code();
}

std::vector<int> RsPoeImpl::getPortList() const
{
    std::vector<int> keys;
    for (const auto &pair : m_portMap) {
        keys.push_back(pair.first);
    }
    return keys;
}

rs::PoeState RsPoeImpl::getPortState(int port)
{
    rs::PoeState state = rs::PoeState::Error;

    if (mp_controller == nullptr) {
        m_lastError = RsErrorCode::NotInitialized;
        m_lastErrorString = "XML file never set";
        return state;
    }

    if (m_portMap.find(port) == m_portMap.end()) {
        m_lastError = std::make_error_code(std::errc::invalid_argument);
        m_lastErrorString = "Invalid port";
        return state;
    }

    try {
        state = mp_controller->getPortState(m_portMap[port]);
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
        m_lastErrorString = "Unknown exception occurred";
    }

    return state;
}

void RsPoeImpl::setPortState(int port, rs::PoeState state)
{
    if (mp_controller == nullptr) {
        m_lastError = RsErrorCode::NotInitialized;
        m_lastErrorString = "XML file never set";
        return;
    }

    if (state == rs::PoeState::Error) {
        m_lastError = std::make_error_code(std::errc::invalid_argument);
        m_lastErrorString = "Invalid state";
        return;
    }

    if (m_portMap.find(port) == m_portMap.end()) {
        m_lastError = std::make_error_code(std::errc::invalid_argument);
        m_lastErrorString = "Invalid port";
        return;
    }

    try {
        mp_controller->setPortState(m_portMap[port], state);
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
        m_lastErrorString = "Unknown exception occurred";
    }
}

float RsPoeImpl::getPortVoltage(int port)
{
    float voltage = 0;

    if (mp_controller == nullptr) {
        m_lastError = RsErrorCode::NotInitialized;
        m_lastErrorString = "XML file never set";
        return voltage;
    }

    if (m_portMap.find(port) == m_portMap.end()) {
        m_lastError = std::make_error_code(std::errc::invalid_argument);
        m_lastErrorString = "Invalid port";
        return voltage;
    }

    try {
        voltage = mp_controller->getPortVoltage(m_portMap[port]);
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
        m_lastErrorString = "Unknown exception occurred";
    }

    return voltage;
}

float RsPoeImpl::getPortCurrent(int port)
{
    float current = 0;

    if (mp_controller == nullptr) {
        m_lastError = RsErrorCode::NotInitialized;
        m_lastErrorString = "XML file never set";
        return current;
    }

    if (m_portMap.find(port) == m_portMap.end()) {
        m_lastError = std::make_error_code(std::errc::invalid_argument);
        m_lastErrorString = "Invalid port";
        return current;
    }

    try {
        current = mp_controller->getPortCurrent(m_portMap[port]);
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
        m_lastErrorString = "Unknown exception occurred";
    }

    return current;
}

float RsPoeImpl::getPortPower(int port)
{
    float power = 0;

    if (mp_controller == nullptr) {
        m_lastError = RsErrorCode::NotInitialized;
        m_lastErrorString = "XML file never set";
        return power;
    }

    if (m_portMap.find(port) == m_portMap.end()) {
        m_lastError = std::make_error_code(std::errc::invalid_argument);
        m_lastErrorString = "Invalid port";
        return power;
    }

    try {
        power = mp_controller->getPortPower(m_portMap[port]);
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
        m_lastErrorString = "Unknown exception occurred";
    }

    return power;
}

int RsPoeImpl::getBudgetConsumed()
{
    int consumed = 0;
    if (mp_controller == nullptr) {
        m_lastError = RsErrorCode::NotInitialized;
        m_lastErrorString = "XML file never set";
        return consumed;
    }

    try {
        consumed = mp_controller->getBudgetConsumed();
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
        m_lastErrorString = "Unknown exception occurred";
    }

    return consumed;
}

int RsPoeImpl::getBudgetAvailable()
{
    int available = 0;

    if (mp_controller == nullptr) {
        m_lastError = RsErrorCode::NotInitialized;
        m_lastErrorString = "XML file never set";
        return available;
    }

    try {
        available = mp_controller->getBudgetAvailable();
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
        m_lastErrorString = "Unknown exception occurred";
    }

    return available;
}

int RsPoeImpl::getBudgetTotal()
{
    int total = 0;

    if (mp_controller == nullptr) {
        m_lastError = RsErrorCode::NotInitialized;
        m_lastErrorString = "XML file never set";
        return total;
    }

    try {
        total = mp_controller->getBudgetTotal();
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
        m_lastErrorString = "Unknown exception occurred";
    }

    return total;
}

std::error_code RsPoeImpl::getLastError() const { return m_lastError; }

std::string RsPoeImpl::getLastErrorString() const
{
    std::string lastError;

    if (m_lastError.value() != 0) {
        lastError += m_lastError.message();
        if (!m_lastErrorString.empty()) {
            lastError += ": " + m_lastErrorString;
        }
    }
    return lastError;
}

rs::RsPoe *rs::createRsPoe() { return new RsPoeImpl; }

const char *rs::rsPoeVersion() { return RSSDK_VERSION_STRING; }
