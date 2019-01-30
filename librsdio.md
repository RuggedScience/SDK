# librsdio

The **librsdio** library is used to control the DIO on supported Rugged Science units. Before using any of the DIO functions, the `initDio` function must be called with the correct initialization file. If this is not done, all of the library functions will return an error without performing their task. All library functions return a negative value on error. If this happens it is best to use the `getLastDioError` function to see what caused the error.

## Public Types

### OutputMode
```c++
enum OutputMode
```
---
| Constant  | Value     | Description                       |
|-----------|-----------|-----------------------------------|
| ModePnp   | -1        | Pin state LOW = Open, HIGH = DC-  |
| ModeNpn   | -2        | Pin state LOW = Open, HIGH = DC+  |

<br>

## Public Functions

### initDio
```c++
bool initDio(const char *initFile)
```
---

### Parameters
initFile - This should be a path to the XML file specific to your model.

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

### setOutpuMode
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

### getLastDioError
```c++
const char *getLastDioError()
```
---

### Return value
Text containing the last error encountered by the library.