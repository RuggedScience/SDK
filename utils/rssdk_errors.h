#include <system_error>

enum class RsSdkError
{
    Success = 0,
    NotInitialized,
    XmlParseError,
    PermissionDenied,
    DeviceNotFound,
    DeviceBusy,
    FunctionNotSupported,
    InvalidArgument,
    CommunicationError,
    UnknownError,
};

namespace std
{
    template <> struct is_error_code_enum<RsSdkError> : true_type {};
}

extern std::error_code make_error_code(RsSdkError e);

struct RsSdkError_category : public std::error_category
{
    virtual const char *name() const noexcept override final { return "RsSdkError"; }

    virtual std::string message(int c) const override final
    {
        switch (static_cast<RsSdkError>(c))
        {
        case RsSdkError::Success:
            return "Success";
        case RsSdkError::NotInitialized:
            return "Object not initialized";
        case RsSdkError::XmlParseError:
            return "XML parsing error";
        case RsSdkError::PermissionDenied:
            return "Permission denied";
        case RsSdkError::DeviceNotFound:
            return "Device not found";
        case RsSdkError::DeviceBusy:
            return "Device busy";
        case RsSdkError::FunctionNotSupported:
            return "Function not supported";
        case RsSdkError::InvalidArgument:
            return "Invalid argument";
        case RsSdkError::CommunicationError:
            return "Communication error";
        default:
            return "Unknown Error";
        }
    }
    
    virtual std::error_condition default_error_condition(int c) const noexcept override final
    {
        switch (static_cast<RsSdkError>(c))
        {
        case RsSdkError::PermissionDenied:
            return std::make_error_condition(std::errc::operation_not_permitted);
        case RsSdkError::DeviceNotFound:
            return std::make_error_condition(std::errc::no_such_device);
        case RsSdkError::DeviceBusy:
            return std::make_error_condition(std::errc::device_or_resource_busy);
        case RsSdkError::FunctionNotSupported:
            return std::make_error_condition(std::errc::function_not_supported);
        case RsSdkError::InvalidArgument:
            return std::make_error_condition(std::errc::invalid_argument);
        default:
            return std::error_condition(c, *this);
        }
    }
};