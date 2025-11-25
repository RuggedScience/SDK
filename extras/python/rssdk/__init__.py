"""
RSSDK: Python Bindings for the Rugged Science SDK.
"""

import os

if os.name == 'nt':
    import ctypes
    # Manually load the driver dll. Windows will always check if a dll
    # is loaded in memory first before searching other directories.
    dir_path = os.path.dirname(os.path.realpath(__file__))
    dll = os.path.join(dir_path, 'drv.dll')
    hllDll = ctypes.WinDLL(dll)
        

# Import the Low-Level C++ Implementation
# We assume the compiled module is named '_rssdk' and sits next to this file.
try:
    from .rssdk import *
except ImportError:
    raise ImportError("Could not import the '_rssdk' C++ extension. "
                      "Ensure the package was built and installed correctly.")

del rssdk