# Error Handling

This SDK uses [std::error_code](https://en.cppreference.com/w/cpp/error/error_code) for all errors. Internally these codes are used to populate and throw [std::system_error](https://en.cppreference.com/w/cpp/error/system_error). All exceptions are caught at the library boundry to prevent exceptions from crossing that boundry. The exception's [code](https://en.cppreference.com/w/cpp/error/system_error/code) and [message](https://en.cppreference.com/w/cpp/error/system_error/what) are stored in the library object and accessible through the [getLastError](/librsdio.md#getlasterror) and [getLastErrorString](/librsdio#getlasterrorstring) methods. 

## Examples
```c++
#include <rsdio.h>
#include <rssdk_errors.h>
...
rs::RsDio *dio = rs::createRsDio();
if (!dio->setXmlFile('ecs9000.xml'))
{
    if (dio->getLastError() == rs::RsSdkError::PermissionError)
        std::cerr << "Program must be run as root" << std::endl;
    else
        std::cerr << dio->getLastErrorString() << std::endl;

    dio->destroy();
    return 1;
}

if (!dio->setOutputMode(1, rs::OutputMode::Source))
{
    if (dio->getLastError() == rs::RsSdkError::FunctionNotSupported)
    {
        std::cerr << "This device does not support setting the output mode" << std::endl;
        return 1;
    }
}

int state = dio->digitalRead(1, 200);
if (state < 0)
{
    // Pin 200 doesn't exist.
    if (dio->getLastError() == rs::RsSdkError::InvalidArgument)
    {
        std::cerr << "Pin 200 doesn't exist" << std::endl;
        return 1;
    }
    // Standard error codes can be used for comparison as well
    if (dio->getLastError() == std::errc::invalid_argument)
    {
        ...
    }
}

```


## RsSdkError
```c++
enum class rs::RsSdkError
```

All error codes 

---
| Constant              |  Description                                                                                          |
|-----------------------|-------------------------------------------------------------------------------------------------------|
| NotInitialized        | [setXmlFile](/librsdio.md#setxmlfile) was never successfully called                                        |
| XmlParseError         | There was an error parsing the XML file                                                               |
| PermissionDenied      | Program was not run as root. *Linux only*                                                             |
| DeviceNotFound        | The controller described in the XML file could not be found                                           |
| DeviceBusy            | SMBus communication failed because the device is already in use                                       |
| FunctionNotSupported  | The current hardware does not support the requested function                                          |
| InvalidArgument       | The provided argument was not valid. Usually incorrect port / pin number                              |
| CommunicationError    | The data received from the controller was unexpected or invalid                                       |
| UnknownError          | An exception occurred that the library doesn't know how to handle. Hopefully this never happens...    |

<br>