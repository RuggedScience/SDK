import os

if os.name == 'nt':
    dir_path = os.path.dirname(os.path.realpath(__file__))
    # If add_dll_directory exists (Python >= 3.8) let's use it
    # to tell Windows where to look for the drv.dll
    try:
        os.add_dll_directory(dir_path)
    # If not, manually load the dll. Windows will always check if a dll
    # is loaded in memory first before searching other directories.
    except AttributeError:
        import ctypes
        dll = os.path.join(dir_path, 'drv.dll')
        hllDll = ctypes.WinDLL(dll)

from rssdk.rssdk import *