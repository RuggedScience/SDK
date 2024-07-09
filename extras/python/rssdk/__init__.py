import os

if os.name == 'nt':
    import ctypes
    # Manually load the driver dll. Windows will always check if a dll
    # is loaded in memory first before searching other directories.
    dir_path = os.path.dirname(os.path.realpath(__file__))
    dll = os.path.join(dir_path, 'drv.dll')
    hllDll = ctypes.WinDLL(dll)
        

from rssdk.rssdk import *