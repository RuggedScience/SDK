# librsdio

The **librsdio** library is used to control the DIO on supported Rugged Science units. All library functions return either a negative value or false to indicate an error occurred. If this happens it is best to use the [getLastError](#getlasterror) and [getLastErrorString](#getlasterrorstring) functions to see what caused the error. For more information on error handling see [Error Handling](/errors.md#error-handling).

## Basic Usage

```c++
#include <rsdio.h>
...
rs::RsDio *dio = rs::createRsDio();
if (!dio->setXmlFile('ecs9000.xml'))
{
    std::cerr << dio->getLastErrorString() << std::endl;
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

## Public Functions

### setXmlFile
```c++
bool RsDio::setXmlFile(const char *fileName)
```

Initializes the RsDio class with the appropriate hardware. Must be called before any other functions.

---

### Parameters
fileName - This should be a path to the XML file specific to your model.

### Return value
Returns true if the dio library was successfully initialized, otherwise returns false.

<br>

### digitalRead
```c++
int RsDio::digitalRead(int dio, int pin)
```

Reads the state of `pin` on `dio`. Can be used on either input or output pins.

---

### Parameters
dio - The number of the dio which is being read. Screen printed on the unit. Generally 1 or 2.  
pin - The number of the pin which is being read. Screen printed on the unit. Generally 1 through 20.

**NOTE:** Not all pins are input / output pins. See the user manual of your specific unit for pinout information.

### Return value
The state of `pin` read from `dio`. Negative value on error.  
0 - LOW  
1 - HIGH  

<br>

### digitalWrite
```c++
bool RsDio::digitalWrite(int dio, int pin, bool state)
```

Sets the state of `pin` on `dio` to `state`. Can only be used on pins in output mode.

---

### Parameters
dio - The number of the dio which is being set. Screen printed on the unit. Generally 1 or 2.  
pin - The number of the pin which is being set. Screen printed on the unit. Generally 1 through 20.  
state - The state to which the supplied dio / pin should be set.  

### Return value
True on success and false on failure. 

<br>

### setOutputMode
```c++
bool RsDio::setOutputMode(int dio, rs::OutputMode mode)
```

Sets the output mode of `dio` to `mode`.  
**NOTE:** Not all hardware supports switching output modes.

---

### Parameters
dio - The number off the dio whos output mode should be set.  
mode - The mode which dio should be set to. [OutputMode](#outputmode)

### Return value
True on success and false on failure. 

<br>

### getLastError
```c++
std::error_code RsDio::getLastError() const
```

Gets the [std::error_code](https://en.cppreference.com/w/cpp/error/error_code) for the last error that occurred. For more information on possible errors see [RsSdkErrors](./errors.md#rssdkerror).

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
