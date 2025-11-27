# librspoe

The **librspoe** library is used to control the PoE ports on supported Rugged Science units.

## Basic Usage

```c++
#include <rspoe.h>
...
rs::RsPoe *poe = rs::createRsPoe();
try {
    poe->init('ecs9000.xml');
} catch (const rs::RsException &ex) {
    std::cerr << ex.what() << std::endl;
    poe->destroy();
    return 1;
}

poe->setPortState(3, rs::PoeState::Enabled);
float power = poe->getPortPower(3);
poe->destroy();
```

One important thing to note is that the RsPoe class has special "create" and "destroy" functions. This is due to dynamic library memory limitations. In order to keep clean memory boundries between the application and the library, all memory managment is handled in the library. To make things easier, you can simply wrap the class instance in a smart pointer.

```c++
std::shared_ptr<rs::RsPoe> poe(rs::createRsPoe(), std::mem_fn(&rs::RsPoe::destroy));
if (!poe)
{
    std::cerr << "Failed to create instance of RsPoe" << std::endl;
    return 1;
}
```
## Error Handling
All errors are thrown as RsExceptions which contain an error code and message.

```c++
try {
    poe->getBudgetTotal();
} catch (const rs::RsException &ex) {
    std::cerr << "Failed to get budget: " << ex.what() << std::endl;
    return 1;
}
```

## Public Types

### PoeState
```c++
enum class rs::PoeState
```
---
| Constant      | Value | Description                                                   |
|---------------|-------|---------------------------------------------------------------|
| Disabled      | 0     | No power regardless of attached device.                       |
| Enabled       | 1     | Power applied regardless of attached device (Passive PoE).    |
| Auto          | 2     | Normal PoE operation (Active PoE).                            |
| Error         | 3     | Error retreiving state. Check getLastPoeError.                |

<br>

## Public Functions

### init
```c++
void RsPoe::init(const char *configFile)
```

Initializes the RsDio class with the appropriate hardware. Must be called before any other functions.

---

### Parameters
configFile - This should be a path to the XML file specific to your model.

<br>

### getPortState
```c++
rs::PoeState RsPoe::getPortState(int port)
```

Reads the state of `port`.

---

### Parameters
port - The number of the port to read the state from. Screen printed on the unit in the form of Lan `3`.

### Return value
Returns the [PoeState](#poestate) of port.

<br>

### setPortState
```c++
void RsPoe::setPortState(int port, rs::PoeState state)
```

Sets the state of `port` to `state`.

---

### Parameters
port - The number of the port to be set. Screen printed on the unit in the form of Lan `3`.
state - The desired [PoeState](#poestate) of port.

<br>

### getPortVoltage
```c++
float RsPoe::getPortVoltage(int port)
```

Gets the current output voltage of `port` in volts.

---

### Parameters
port - The number of the port to read the voltage from. Screen printed on the unit in the form of Lan `3`.

### Return value
Voltage of port in volts.

<br>

### getPortCurrent
```c++
float RsPoe::getPortCurrent(int port)
```

Gets the current current of `port` in amps.

---

### Parameters
port - The number of the port to read the current from. Screen printed on the unit in the form of Lan `3`.

### Return value
Current of port in amps.

<br>

### getPortPower
```c++
float RsPoe::getPortPower(int port)
```

Gets the current power draw of `port` in watts. *Same as [getPortVoltage](#getportvoltage) X [getPortCurrent](#getportcurrent)*

---

### Parameters
port - The number of the port to read the power from. Screen printed on the unit in the form of Lan `3`.

### Return value
Current power draw of port in watts.

<br>

### getBudgetConsumed
```c++
int RsPoe::getBudgetConsumed()
```

Gets the total watts being consumed from all ports. *If this exceeds the total budget, the PoE controller will start disabling ports based on priority until it's below the total budget*

---

### Return value
Total budget consumed in watts.

<br>

### getBudgetAvailable
```c++
int RsPoe::getBudgetAvailable()
```

Gets the watts available before reaching max budget.

---

### Return value
Remaining budget in watts.

<br>

### getBudgetTotal
```c++
int RsPoe::getBudgetTotal()
```
Gets the total watts the unit can handle. When this is exceeded the PoE controller will start shutting down ports to prevent an over current condition.

---

### Return value
Total available budget in watts.

<br>

### getLastError
```c++
std::error_code RsPoe::getLastError() const
```

Gets the [std::error_code](https://en.cppreference.com/w/cpp/error/error_code) for the last error that occurred. Cleared after a successful operation.

---

### Return value
The [std::error_code](https://en.cppreference.com/w/cpp/error/error_code) for the last error that occurred.

<br>

### getLastErrorString
```c++
std::string RsPoe::getLastErrorString() const
```

Gets the string containing information about the last error that occured. This will usually provide a bit more info about [getLastError](#getlasterror).

---

### Return value
String containing the last error.

<br>

### rsPoeVersion
```c++
const char *rs::rsPoeVersion()
```

---

### Return value
String containing the current library version
