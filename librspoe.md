# librspoe

The **librspoe** library is used to control the PoE ports on supported Rugged Science units. Before using any of the PoE functions, the `initPoe` function must be called with the correct initialization file. If this is not done, all of the library functions will return an error without performing their task. All library functions return a negative value on error. If this happens it is best to use the `getLastPoeError` function to see what caused the error.

## Public Types

### PoeState
```c++
enum PoeState
```
---
| Constant      | Value | Description                                   |
|---------------|-------|-----------------------------------------------|
| StateDisabled | 0     | No power regardless of attached device        |
| StateEnabled  | 1     | Power applied regardless of attached device   |
| StateAuto     | 2     | Normal PoE operation                          |
| StateError    | 3     | Error retreiving state. Check getLastPoeError |

<br>

## Public Functions

### initPoe
```c++
bool initPoe(const char *initFile)
```
---

### Parameters
initFile - This should be a path to the XML file specific to your model.

### Return value
Returns true if the dio was successfully initialized, otherwise returns false.

<br>

### getPortState
```c++
PoeState getPortState(int port)
```
---

### Parameters
port - The number of the port to read the state from. *Screen printed on the unit*

### Return value
Returns the [PoeState](##Public-Types) of port.

<br>

### setPortState
```c++
int setPortState(int port, PoeState state)
```
---

### Parameters
port - The number of the port to be set.\
state - The desired [PoeState](##Public-Types) of port.

### Return value
Zero if sucessfully set. Negative value on error.

<br>

### getPortVoltage
```c++
float getPortVoltage(int port)
```
---

### Parameters
port - The number of the port to read the voltage from. *Screen printed on the unit*

### Return value
Voltage of port in volts. Negative value on error.

<br>

### getPortCurrent
```c++
float getPortCurrent(int port)
```
---

### Parameters
port - The number of the port to read the current from. *Screen printed on the unit*

### Return value
Current of port in amps. Negative value on error.

<br>

### getPortPower
```c++
float getPortPower(int port)
```
---

### Parameters
port - The number of the port to read the power from. *Screen printed on the unit*

### Return value
Power of port in watts. Negative value on error.

<br>

### getBudgetConsumed
```c++
int getBudgetConsumed()
```
---

### Return value
Total watts being consumed from all ports. Negative value on error.\
*If this exceeds the total budget, the PoE controller will start disabling ports based on priority until it's below the total budget*

<br>

### getBudgetAvailable
```c++
int getBudgetAvailable()
```
---

### Return value
Watts available before reaching max budget. Negative value on error.

<br>

### getBudgetTotal
```c++
int getBudgetTotal()
```
---

### Return value
Total watts the unit can handle. Negative value on error.

<br>

### getLastPoeError
```c++
const char *getLastPoeError()
```
---

### Return value
Text containing the last error encountered by the library.