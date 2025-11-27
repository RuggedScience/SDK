# librsdio

The **librsdio** library is used to control the DIO on supported Rugged Science units. 

## Basic Usage

```c++
#include <rsdio.h>
...
rs::RsDio *dio = rs::createRsDio();
try {
    dio->init('ecs9000.xml');
} catch (const rs::RsException &ex) {
    std::cerr << ex.what() << std::endl;
    dio->destroy();
    return 1;
}

dio->setOutputMode(1, rs::OutputMode::Source);
dio->digitalWrite(1, 11, true);
dio->destroy();
```

One important thing to note is that the RsDio class has special "create" and "destroy" functions. This is due to dynamic library memory limitations. In order to keep clean memory boundries between the application and the library, all memory managment is handled in the library. To make things easier, you can simply wrap the class instance in a smart pointer.

```c++
std::shared_ptr<rs::RsDio> dio(rs::createRsDio(), std::mem_fn(&rs::RsDio::destroy));
if (!dio)
{
    std::cerr << "Failed to create instance of RsDio" << std::endl;
    return 1;
}
```

## Error Handling
All errors are thrown as RsExceptions which contain an error code and message.

```c++
try {
    dio->setOutputMode(rs::OutputMode::Sink);
} catch (const rs::RsException &ex) {
    // We don't care if this model doesn't support setting it.
    if (ex.code() != rs::RsErrorCode::UnsupportedFunction) {
        std::cerr << "Failed to set output mode: " << ex.what() << std::endl;
    }
}
```

## Public Types

### OutputMode
```c++
enum class rs::OutputMode
```
---
| Constant  | Value     | Description                       |
|-----------|-----------|-----------------------------------|
| Source    | -1        | Pin state LOW = Open, HIGH = DC+  |
| Sink      | -2        | Pin state LOW = Open, HIGH = DC-  |

<br>

### PinDirection
```c++
enum class rs::PinDirection
```
---
| Constant  |
|-----------|
| Input     |
| Output    |

<br>


## Public Functions

### init
```c++
void RsDio::init(const char *configFile)
```

Initializes the RsDio class with the appropriate hardware. Must be called before any other functions.

---

### Parameters
configFile - This should be a path to the XML file specific to your model.

<br>

### digitalRead
```c++
bool RsDio::digitalRead(int dio, int pin)
```

Reads the state of `pin` on `dio`. Can be used on either input or output pins.

---

### Parameters
dio - The number of the dio which is being read. Screen printed on the unit. Generally 1 or 2.  
pin - The number of the pin which is being read. Screen printed on the unit. Generally 1 through 20.

**NOTE:** Not all pins are input / output pins. See the user manual of your specific unit for pinout information.

### Return value
The state of `pin` read from `dio`.

<br>

### digitalWrite
```c++
void RsDio::digitalWrite(int dio, int pin, bool state)
```

Sets the state of `pin` on `dio` to `state`. Can only be used on pins in output mode.

---

### Parameters
dio - The number of the dio which is being set. Screen printed on the unit. Generally 1 or 2.  
pin - The number of the pin which is being set. Screen printed on the unit. Generally 1 through 20.  
state - The state to which the supplied dio / pin should be set.  

<br>

### setPinDirection
```c++
void RsDio::setPinDirection(int dio, int pin, rs::PinDirection dir)
```

Sets the direction of `pin` on `dio` to `dir`. Will cause an error if the pin does not support the provided direction.

---

### Parameters
dio - The number of the dio which is being set. Screen printed on the unit. Generally 1 or 2.  
pin - The number of the pin which is being set. Screen printed on the unit. Generally 1 through 20.  
dir - The direction to which the supplied dio / pin should be set.  

<br>

### getPinDirection
```c++
rs::PinDirection RsDio::getPinDirection(int dio, int pin)
```

Gets the direction of `pin` on `dio`.

---

### Parameters
dio - The number of the dio which is being set. Screen printed on the unit. Generally 1 or 2.  
pin - The number of the pin which is being set. Screen printed on the unit. Generally 1 through 20.  

<br>

### setOutputMode
```c++
void RsDio::setOutputMode(int dio, rs::OutputMode mode)
```

Sets the output mode of `dio` to `mode`.  
**NOTE:** Not all hardware supports switching output modes.

---

### Parameters
dio - The number off the dio whos output mode should be set.  
mode - The mode which dio should be set to. [OutputMode](#outputmode)


<br>

### getLastError
```c++
std::error_code RsDio::getLastError() const
```

Gets the [std::error_code](https://en.cppreference.com/w/cpp/error/error_code) for the last error that occurred. Cleared after a successful operation.

---

### Return value
The [std::error_code](https://en.cppreference.com/w/cpp/error/error_code) for the last error that occurred.

<br>

### getLastErrorString
```c++
std::string RsDio::getLastErrorString() const
```

Gets the string containing information about the last error that occured. This will usually provide a bit more info about [getLastError](#getlasterror).

---

### Return value
String containing the last error.

<br>

### rsDioVersion
```c++
const char *rs::rsDioVersion()
```

---

### Return value
String containing the current library version
