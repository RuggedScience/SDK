# librsdio

The **librsdio** library is used to control the DIO on supported Rugged Science units. All library functions return a negative value on error except [`setXmlFile`](###setXmlFile) which returns false on error. If this happens it is best to use the [`getLastError`](###getLastError) function to see what caused the error.

## Basic Usage

```c++
#include <rsdio.h>
...
RsDio *dio = createRsDio();
if (!dio->setXmlFile('ecs9000.xml'))
{
    std::cerr << dio->getLastError() << std::endl;
    dio->destroy();
    return 1;
}

dio->setOutputMode(1, ModePnp);
dio->digitalWrite(1, 11, true);
dio->destroy();
```

One important thing to note is that the RsDio class has special "create" and "destroy" functions. This is due to dynamic library memory limitations. In order to keep clean memory boundries between the application and the library, all memory managment is handled in the library. To make things easier, you can simply wrap the class instance in a smart pointer.

```c++
std::shared_ptr<RsDio> dio(createRsDio(), std::mem_fn(&RsDio::destroy));
if (!dio)
{
    std::cerr << "Failed to create instance of RsDio" << std::endl;
    return 1;
}
```

## Public Types

### OutputMode
```c++
enum OutputMode
```
---
| Constant  | Value     | Description                       |
|-----------|-----------|-----------------------------------|
| ModePnp   | -1        | Pin state LOW = Open, HIGH = DC+  |
| ModeNpn   | -2        | Pin state LOW = Open, HIGH = DC-  |

<br>

## Public Functions

### setXmlFile
```c++
bool setXmlFile(const char *fileName)
```
---

### Parameters
fileName - This should be a path to the XML file specific to your model.

### Return value
Returns true if the dio was successfully initialized, otherwise returns false.

<br>

### digitalRead
```c++
int digitalRead(int dio, int pin)
```
---

### Parameters
dio - The number of the dio which is being read. Screen printed on the unit. Generally 1 or 2.  
pin - The number of the pin which is being read. Screen printed on the unit. Generally 1 through 20.

### Return value
The state of 'pin' read from 'dio'. Negative value on error.  
0 - LOW  
1 - HIGH  

<br>

### digitalWrite
```c++
int digitalWrite(int dio, int pin, bool state)
```
---

### Parameters
dio - The number of the dio which is being set. Screen printed on the unit. Generally 1 or 2.  
pin - The number of the pin which is being set. Screen printed on the unit. Generally 1 through 20.  
state - The state to which the supplied dio / pin should be set.  

### Return value
Zero on successful write. Negative value on error.  

<br>

### setOutputMode
```c++
int setOutputMode(int dio, Output mode)
```
---

### Parameters
dio - The number off the dio whos output mode should be set.  
mode - The mode which dio should be set to. [OutputMode](##OutputMode)  

### Return value
Zero on successful write. Negative value on error.  

<br>

### getLastError
```c++
std::string *getLastError()
```
---

### Return value
String containing the last error encountered by the library.

<br>

### version
```c++
std::string version()
```
---

### Return value
String containing the current library version
