# librspoe

The **librspoe** library is used to control the PoE ports on supported Rugged Science units. All library functions return a negative value or false to indicate an error occurred. If this happens it is best to use the [getLastError](#getlasterror) and [getLastErrorString](#getlasterrorstring) functions to see what caused the error. For more information on error handling see [Error Handling](/errors.md#error-handling).

## Basic Usage

```c++
#include <rspoe.h>
...
rs::RsPoe *poe = rs::createRsPoe();
if (!poe->setXmlFile('ecs9000.xml'))
{
    std::cerr << poe->getLastErrorString() << std::endl;
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

### setXmlFile
```c++
bool RsPoe::setXmlFile(const char *fileName)
```

Initializes the RsDio class with the appropriate hardware. Must be called before any other functions.

---

### Parameters
fileName - This should be a path to the XML file specific to your model.

### Return value
Returns true if the poe library was successfully initialized, otherwise returns false.

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
int RsPoe::setPortState(int port, rs::PoeState state)
```

Sets the state of `port` to `state`.

---

### Parameters
port - The number of the port to be set. Screen printed on the unit in the form of Lan `3`.
state - The desired [PoeState](#poestate) of port.

### Return value
Zero if sucessfully set. Negative value on error.

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
Voltage of port in volts on success. Negative value on error.

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
Current of port in amps on success. Negative value on error.

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
Power of port in watts on success. Negative value on error.

<br>

### getBudgetConsumed
```c++
int RsPoe::getBudgetConsumed()
```

Gets the total watts being consumed from all ports. *If this exceeds the total budget, the PoE controller will start disabling ports based on priority until it's below the total budget*

---

### Return value
Watts on success. Negative value on error.

<br>

### getBudgetAvailable
```c++
int RsPoe::getBudgetAvailable()
```

Gets the watts available before reaching max budget.

---

### Return value
Watts on success. Negative value on error.

<br>

### getBudgetTotal
```c++
int RsPoe::getBudgetTotal()
```
Gets the total watts the unit can handle. When this is exceeded the PoE controller will start shutting down ports to prevent an over current condition.

---

### Return value
Watts on success. Negative value on error.

<br>

### getLastError
```c++
std::error_code RsPoe::getLastError() const
```

Gets the [std::error_code](https://en.cppreference.com/w/cpp/error/error_code) for the last error that occurred. For more information on possible errors see [RsErrorCodes](./errors.md#RsErrorCode).

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
