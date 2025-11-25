# Error Handling

This SDK uses [std::error_code](https://en.cppreference.com/w/cpp/error/error_code) for all errors. Internally these codes are used to populate and throw [std::system_error](https://en.cppreference.com/w/cpp/error/system_error). All exceptions are caught at the library boundry to prevent exceptions from crossing that boundry. The exception's [code](https://en.cppreference.com/w/cpp/error/system_error/code) and [message](https://en.cppreference.com/w/cpp/error/system_error/what) are stored in the library object and accessible through the [getLastError](./librsdio.md#getlasterror) and [getLastErrorString](./librsdio#getlasterrorstring) methods.

```c++
#include <rsdio.h>
#include <rserrors.h>
...
std::shared_ptr<rs::RsDio> dio(rs::createRsDio(), std::mem_fn(&rs::RsDio::destroy));
if (!dio->setXmlFile('ecs9000.xml'))
{
    if (dio->getLastError() == std::errc::operation_not_permitted)
        std::cerr << "Program must be run as root" << std::endl;
    else if (dio->getLastError() == RsErrorCode::XmlParseError)
        std::cerr << "Failed to parse XML file" << std::endl;
    else
        std::cerr << dio->getLastErrorString() << std::endl;

    return 1;
}

```

In the above example the error code is being compared directly to another error code. A better approach is to compare the error code to an [error condition](https://en.cppreference.com/w/cpp/error/error_condition). Error condtions group multiple codes into a single condition to help create separate execution paths. For example, say you are running the same program on multiple devices with different hardware. You may want to handle or ignore errors when the hardware doesn't support a specific function and log the error if it is more serious.

```c++
if (!dio->setOutputMode(rs::OutputMode::Source))
{
    // Fatal unrecoverable error occured. Log it and terminate.
    if (dio->getLastError() == RsErrorCondition::HardwareError)
    {
        logError(dio->getLastErrorString());
        exit(1);
    }
    // This computer doesn't have that feature.
    else if (dio->getLastError() == RsErrorCondition::UnsupportedFunction)
    {
        ...
    }
}
```

## RsErrorCode
```c++
enum class RsErrorCode
```

---
| Constant              |  Description                                                                                          |
|-----------------------|-------------------------------------------------------------------------------------------------------|
| NotInitialized        | [setXmlFile](/docs/librsdio.md#setxmlfile) was never successfully called                                   |
| XmlParseError         | There was an error parsing the XML file                                                               |
| UnknownError          | An exception occurred that the library doesn't know how to handle. Hopefully this never happens...    |

## RsErrorCondition
```c++
enum class RsErrorCondition
```

---
| Constant              |  Description                                                                  |
|-----------------------|-------------------------------------------------------------------------------|
| HardwareError         | A low level error occured while communicating with the hardware               |
| UnsupportedFunction   | This unit doesn't support this functionality                                  |
| PermissionErrror      | Permission was denied trying to communicate with the hardware *Linux only*    |