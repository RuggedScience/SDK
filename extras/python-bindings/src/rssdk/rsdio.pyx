__all__ = ['RsDio']

from enum import Enum

from libcpp cimport bool
from libcpp.string cimport string


cdef extern from "rsdio.h":
    cdef enum COutputMode "OutputMode":
        pass

    cdef cppclass CRsDio "RsDio":
        void destroy()
        bool setXmlFile(const char *, bool)
        int setOutputMode(int, COutputMode)
        int digitalRead(int, int)
        int digitalWrite(int, int, bool)
        string getLastError()
        string version()

    CRsDio* createRsDio() except +


class OutputMode(Enum):
    ModeError = 0
    ModePnp = -1
    ModeNpn = -2


cdef class RsDio:
    cdef CRsDio *_native
    def __cinit__(self):
        self._native = createRsDio()
    def __dealloc__(self):
        self._native.destroy()
    def setXmlFile(self, filename: str, debug=False) -> bool:
        return self._native.setXmlFile(filename.encode('utf-8'), debug)
    def setOutputMode(self, dio: int, mode: OutputMode):
        return self._native.setOutputMode(dio, mode.value)
    def digitalRead(self, dio: int, pin: int) -> int:
        return self._native.digitalRead(dio, pin)
    def digitalWrite(self, dio: int, pin: int, state: bool) -> int:
        return self._native.digitalWrite(dio, pin, state)
    def getLastError(self) -> str:
        return self._native.getLastError().decode('UTF-8')
    def version(self) -> str:
        return self._native.version().decode('UTF-8')