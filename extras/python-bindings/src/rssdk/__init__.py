__all__ = ['RsDio', 'OutputMode', 'RsPoe', 'PoeState']

import os

# On Windows we will install the driver DLL with this module.
# Make sure we tell Windows to look here.
if os.name == 'nt':
    dir_path = os.path.dirname(os.path.realpath(__file__))
    os.add_dll_directory(dir_path)

from .rsdio import RsDio, OutputMode
from .rspoe import RsPoe, PoeState