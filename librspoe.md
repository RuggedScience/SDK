# librspoe

The **librspoe** library is used to control the PoE ports on supported Rugged Science units. All library functions return a negative value on error except [`setXmlFile`](###setXmlFile) which returns false on error. If this happens it is best to use the [`getLastError`](###getLastError) function to see what caused the error.

## Basic Usage

```c++

RsPoe *poe = createRsPoe();
if (!poe->setXmlFile('ecs9000.xml'))
{
    std::cerr << poe->getLastError() << std::endl;
    poe->destroy();
    return 1;
}

poe->setPortState(3, StateEnabled);
float power = poe->getPortPower(3);
if (power < 0)
    std::cerr << poe->getLastError() << std::endl;
else
    std::cout << "Port 3 power usage: " << power << std::endl;

poe->destroy();
```

One important thing to note is that the RsPoe class has special "create" and "destroy" functions. This is due to dynamic library memory limitations. In order to keep clean memory boundries between the application and the library, all memory managment is handled in the library. To make things easier, you can simply wrap the class instance in a smart pointer.

```c++
std::shared_ptr<RsPoe> poe(createRsPoe(), std::mem_fn(&RsPoe::destroy));
if (!poe)
{
    std::cerr << "Failed to create instance of RsPoe" << std::endl;
    return 1;
}
```

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

### getLastError
```c++
const char *getLastError()
```
---

### Return value
Text containing the last error encountered by the library.
