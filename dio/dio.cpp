#include "dio.h"
#include "controllers/ite8783.h"
#include "controllers/ite8786.h"
#include "../utils/tinyxml2.h"

#include <map>
#include <string>

static std::string s_lastError;
static AbstractDioController *sp_controller;

typedef std::map<int, PinInfo> pinmap_t;
static std::map<int, pinmap_t> s_dioMap;

static tinyxml2::XMLError getInternalPinInfo(tinyxml2::XMLElement *pin, int& pinId, PinInfo& info)
{
	using namespace tinyxml2;

	int id, bit, gpio;
	XMLError e = pin->QueryAttribute("id", &id);
	if (e != XML_SUCCESS) return e;
	e = pin->QueryAttribute("bit", &bit);
	if (e != XML_SUCCESS) return e;
	e = pin->QueryAttribute("gpio", &gpio);
	if (e != XML_SUCCESS) return e;
	
	pinId = id;
	info = PinInfo(bit, gpio, false, false, true);
	return XML_SUCCESS;
}

static tinyxml2::XMLError getExternalPinInfo(tinyxml2::XMLElement *pin, int& pinId, PinInfo& info)
{
	using namespace tinyxml2;

	int id, bit, gpio;
	bool invert, input, output;
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

	pinId = id;
	info = PinInfo(bit, gpio, invert, input, output);
	return XML_SUCCESS;
}

bool initDio(const char* initFile)
{
    using namespace tinyxml2;
	s_dioMap.clear();
	if (sp_controller) delete sp_controller;
	sp_controller = nullptr;

	XMLDocument doc;
	if (doc.LoadFile(initFile) != XML_SUCCESS)
	{
        s_lastError = "XML Error: Unable to load file";
        return false;
    }

	XMLElement *comp = doc.FirstChildElement("computer");
	if (!comp)
	{
        s_lastError = "XML Error: Unable to find computer node";
        return false;
    }

	XMLElement *dio = comp->FirstChildElement("dio_controller");
	if (!dio)
	{
        s_lastError = "XML Error: Unable to find dio_controller node";
        return false;
    }

	std::string id(dio->Attribute("id"));
    try
    {
	    if (id == "ite8783")
    		sp_controller = new Ite8783();
    	else if (id == "ite8786")
		    sp_controller = new Ite8786();
	    else
	    {
    		s_lastError = "XML Error: Invalid id found for dio_controller";
		    return false;
	    }
    }
    catch (std::exception &ex)
    {
        s_lastError = "DIO Controller Error: " + std::string(ex.what());
        return false;
    }

	XMLElement *con = dio->FirstChildElement("connector");
	if (!con)
	{
        s_lastError = "XML Error: Unable to find connector node";
        return false;
    }

	for (; con; con = con->NextSiblingElement("connector"))
	{
		int conId = 0;
		if (con->QueryAttribute("id", &conId) == XML_SUCCESS)
		{
			XMLElement *ip = con->FirstChildElement("internal_pin");
			for (; ip; ip = ip->NextSiblingElement("internal_pin"))
			{
				int pinId;
				PinInfo info;
				if (getInternalPinInfo(ip, pinId, info) == XML_SUCCESS)
				{
					sp_controller->initPin(info);
					s_dioMap[conId][pinId] = info;
				}
			}

			XMLElement *ep = con->FirstChildElement("external_pin");
			for (; ep; ep = ep->NextSiblingElement("external_pin"))
			{
				int pinId;
				PinInfo info;
				if (getExternalPinInfo(ep, pinId, info) == XML_SUCCESS)
				{
					sp_controller->initPin(info);
					s_dioMap[conId][pinId] = info;
				}
			}
		}
	}
    
    if (s_dioMap.size() <= 0)
    {
        sp_controller = nullptr;
        s_lastError = "XML Error: No valid connectors found";
        return false;
    }

    //Set the output mode of each dio if it's not already a valid mode.
    std::map<int, pinmap_t>::iterator it;
    for (it = s_dioMap.begin(); it != s_dioMap.end(); ++it)
    {
        pinmap_t pinMap = it->second;
        //Not all units support programmable NPN/PNP modes so if these pins don't exist we don't really care.
        if (pinMap.find(ModeNpn) != pinMap.end() && pinMap.find(ModePnp) != pinMap.end())
        {
            PinInfo npn = pinMap[ModeNpn];
            PinInfo pnp = pinMap[ModePnp];

            try
            {
                //If these two pins are in the same state the dio will not operate. Let's fix that.
                if (sp_controller->getPinState(npn) == sp_controller->getPinState(pnp))
                {
                    sp_controller->setPinState(npn, true);
                    sp_controller->setPinState(pnp, false);
                }
            }
            catch (std::exception &ex) 
            {
                s_lastError = "DIO Controller Error: " + std::string(ex.what());
                return false;
            }
        }
    }

    return true;
}

int digitalRead(int dio, int pin)
{
    if (sp_controller == nullptr)
        return -1;

    if (s_dioMap.find(dio) == s_dioMap.end())
    {
        s_lastError = "Argument Error: Invalid dio " + std::to_string(dio);
        return -1;
    }

    pinmap_t pinMap = s_dioMap.at(dio);
    if (pinMap.find(pin) == pinMap.end())
    {
        s_lastError = "Argument Error: Invalid pin " + std::to_string(pin);
        return -1;
    }

    PinInfo info = pinMap.at(pin);

    try 
    { 
        return sp_controller->getPinState(info);
    }
    catch (DioControllerError &ex)
    {
        s_lastError = "DIO Controller Error: " + std::string(ex.what());
        return -1;
    }
}

int digitalWrite(int dio, int pin, bool state)
{
    if (sp_controller == nullptr)
        return -1;

    if (s_dioMap.find(dio) == s_dioMap.end())
    {
        s_lastError = "Argument Error: Invalid dio " + std::to_string(dio);
        return -1;
    }

    pinmap_t pinMap = s_dioMap.at(dio);
    if (pinMap.find(pin) == pinMap.end())
    {
        s_lastError = "Argument Error: Invalid pin " + std::to_string(pin);
        return -1;
    }

    PinInfo info = pinMap.at(pin);
    if (!info.supportsOutput)
    {
        s_lastError = "Argument Error: Output mode not supported for pin " + std::to_string(pin);
        return -1;
    }

    try
    {
        sp_controller->setPinState(info, state);
    }
    catch (DioControllerError &ex)
    {
        s_lastError = "DIO Controller Error: " + std::string(ex.what());
        return -1;
    }

    return 0;
}

int setOutputMode(int dio, OutputMode mode)
{
    if (sp_controller == nullptr)
        return -1;

    if (s_dioMap.find(dio) == s_dioMap.end())
    {
        s_lastError = "Argument Error: Invalid dio " + std::to_string(dio);
        return -1;
    }

    pinmap_t pinMap = s_dioMap.at(dio);
    if (pinMap.find(ModeNpn) == pinMap.end() || pinMap.find(ModePnp) == pinMap.end())
    {
        s_lastError = "Argument Error: Function not supported by dio " + std::to_string(dio);
        return -1;
    }

    try
    {
        sp_controller->setPinState(pinMap.at(ModeNpn), (mode == ModeNpn));
        sp_controller->setPinState(pinMap.at(ModePnp), (mode == ModePnp));
    }
    catch (DioControllerError &ex)
    {
        s_lastError = "DIO Controller Error: " + std::string(ex.what());
        return -1;
    }

    return 0;
}

const char* getLastDioError()
{
    return s_lastError.c_str();
}