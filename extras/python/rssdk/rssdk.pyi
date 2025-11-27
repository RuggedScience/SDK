"""
Rugged Science SDK for monitoring and controlling DIO and PoE
"""
from __future__ import annotations
import typing
__all__: list[str] = ['FileNotFound', 'HardwareError', 'InvalidParameter', 'NotInitialized', 'OutputMode', 'PermissionError', 'PinDirection', 'PinInfo', 'PoeState', 'RsDio', 'RsErrorCode', 'RsPoe', 'UnknownError', 'UnsupportedFunction', 'XmlParseError']
class OutputMode:
    """
    Members:
    
      Source
    
      Sink
    """
    Sink: typing.ClassVar[OutputMode]  # value = <OutputMode.Sink: -2>
    Source: typing.ClassVar[OutputMode]  # value = <OutputMode.Source: -1>
    __members__: typing.ClassVar[dict[str, OutputMode]]  # value = {'Source': <OutputMode.Source: -1>, 'Sink': <OutputMode.Sink: -2>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: typing.SupportsInt) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class PinDirection:
    """
    Members:
    
      Input
    
      Output
    """
    Input: typing.ClassVar[PinDirection]  # value = <PinDirection.Input: 0>
    Output: typing.ClassVar[PinDirection]  # value = <PinDirection.Output: 1>
    __members__: typing.ClassVar[dict[str, PinDirection]]  # value = {'Input': <PinDirection.Input: 0>, 'Output': <PinDirection.Output: 1>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: typing.SupportsInt) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class PinInfo:
    supportsInput: bool
    supportsOutput: bool
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: bool, arg1: bool) -> None:
        ...
class PoeState:
    """
    Members:
    
      Disabled
    
      Enabled
    
      Auto
    """
    Auto: typing.ClassVar[PoeState]  # value = <PoeState.Auto: 2>
    Disabled: typing.ClassVar[PoeState]  # value = <PoeState.Disabled: 0>
    Enabled: typing.ClassVar[PoeState]  # value = <PoeState.Enabled: 1>
    __members__: typing.ClassVar[dict[str, PoeState]]  # value = {'Disabled': <PoeState.Disabled: 0>, 'Enabled': <PoeState.Enabled: 1>, 'Auto': <PoeState.Auto: 2>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: typing.SupportsInt) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class RsDio:
    def __init__(self) -> None:
        ...
    def canSetOutputMode(self, dio: typing.SupportsInt) -> bool:
        """
        Check if DIO has programmable output mode.
        """
    def digitalRead(self, dio: typing.SupportsInt, pin: typing.SupportsInt) -> bool:
        """
        Read the state of a single DIO pin
        """
    def digitalWrite(self, dio: typing.SupportsInt, pin: typing.SupportsInt, state: bool) -> None:
        """
        Set the state of a single DIO pin
        """
    def getOutputMode(self, dio: typing.SupportsInt) -> OutputMode:
        ...
    def getPinDirection(self, dio: typing.SupportsInt, pin: typing.SupportsInt) -> PinDirection:
        """
        Get the direction of a single DIO pin
        """
    def getPinList(self) -> dict[int, dict[int, PinInfo]]:
        """
        Get the list of DIO pins as a map
        """
    def init(self, configFile: str) -> None:
        """
        Initialize the hardware with the given config file.
        """
    def readAll(self, dio: typing.SupportsInt) -> dict[int, bool]:
        """
        Read the state of all pins on the specified DIO bank
        """
    def setOutputMode(self, dio: typing.SupportsInt, mode: OutputMode) -> None:
        """
        Set the output mode of a single DIO bank
        """
    def setPinDirection(self, dio: typing.SupportsInt, pin: typing.SupportsInt, dir: PinDirection) -> None:
        """
        Set the direction of a single DIO pin
        """
class RsErrorCode:
    """
    Members:
    
      NotInitialized
    
      FileNotFound
    
      XmlParseError
    
      HardwareError
    
      PermissionError
    
      UnsupportedFunction
    
      InvalidParameter
    
      UnknownError
    """
    FileNotFound: typing.ClassVar[RsErrorCode]  # value = <RsErrorCode.FileNotFound: 2>
    HardwareError: typing.ClassVar[RsErrorCode]  # value = <RsErrorCode.HardwareError: 4>
    InvalidParameter: typing.ClassVar[RsErrorCode]  # value = <RsErrorCode.InvalidParameter: 7>
    NotInitialized: typing.ClassVar[RsErrorCode]  # value = <RsErrorCode.NotInitialized: 1>
    PermissionError: typing.ClassVar[RsErrorCode]  # value = <RsErrorCode.PermissionError: 5>
    UnknownError: typing.ClassVar[RsErrorCode]  # value = <RsErrorCode.UnknownError: 8>
    UnsupportedFunction: typing.ClassVar[RsErrorCode]  # value = <RsErrorCode.UnsupportedFunction: 6>
    XmlParseError: typing.ClassVar[RsErrorCode]  # value = <RsErrorCode.XmlParseError: 3>
    __members__: typing.ClassVar[dict[str, RsErrorCode]]  # value = {'NotInitialized': <RsErrorCode.NotInitialized: 1>, 'FileNotFound': <RsErrorCode.FileNotFound: 2>, 'XmlParseError': <RsErrorCode.XmlParseError: 3>, 'HardwareError': <RsErrorCode.HardwareError: 4>, 'PermissionError': <RsErrorCode.PermissionError: 5>, 'UnsupportedFunction': <RsErrorCode.UnsupportedFunction: 6>, 'InvalidParameter': <RsErrorCode.InvalidParameter: 7>, 'UnknownError': <RsErrorCode.UnknownError: 8>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: typing.SupportsInt) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class RsPoe:
    def __init__(self) -> None:
        ...
    def getBudgetAvailable(self) -> int:
        """
        Get the current remaining budget for all ports in watts
        """
    def getBudgetConsumed(self) -> int:
        """
        Get the current consumed budget of all ports in watts
        """
    def getBudgetTotal(self) -> int:
        """
        Get the max power in watts that all ports can consume
        """
    def getPortCurrent(self, port: typing.SupportsInt) -> float:
        """
        Get the current output of port in amps
        """
    def getPortList(self) -> list[int]:
        """
        Get the list of available ports
        """
    def getPortPower(self, port: typing.SupportsInt) -> float:
        """
        Get the power output of port in watts
        """
    def getPortState(self, port: typing.SupportsInt) -> PoeState:
        """
        Get the state of port
        """
    def getPortVoltage(self, port: typing.SupportsInt) -> float:
        """
        Get the voltage output of port in volts
        """
    def init(self, configFile: str) -> None:
        """
        Initialize the hardware with the given config file.
        """
    def setPortState(self, port: typing.SupportsInt, state: PoeState) -> None:
        """
        Set the state of port
        """
FileNotFound: RsErrorCode  # value = <RsErrorCode.FileNotFound: 2>
HardwareError: RsErrorCode  # value = <RsErrorCode.HardwareError: 4>
InvalidParameter: RsErrorCode  # value = <RsErrorCode.InvalidParameter: 7>
NotInitialized: RsErrorCode  # value = <RsErrorCode.NotInitialized: 1>
PermissionError: RsErrorCode  # value = <RsErrorCode.PermissionError: 5>
UnknownError: RsErrorCode  # value = <RsErrorCode.UnknownError: 8>
UnsupportedFunction: RsErrorCode  # value = <RsErrorCode.UnsupportedFunction: 6>
XmlParseError: RsErrorCode  # value = <RsErrorCode.XmlParseError: 3>
