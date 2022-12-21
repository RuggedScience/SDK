import os
import sys
from pathlib import Path

from cmake_build_extension import BuildExtension, CMakeExtension
from setuptools import setup

CIBW_CMAKE_OPTIONS = []
if "CIBUILDWHEEL" in os.environ and os.environ["CIBUILDWHEEL"] == "1":
    if sys.platform == "linux":
        CIBW_CMAKE_OPTIONS += ["-DCMAKE_INSTALL_LIBDIR=lib"]

package_data = {"rssdk": ["*.py", "*.pyi", "py.typed"]}
if os.name == "nt":
    package_data["rssdk"].append("*.dll")

setup(
    ext_modules=[
        CMakeExtension(
            name="rssdk",
            # Name of the resulting module
            install_prefix="rssdk",
            disable_editable=True,
            cmake_depends_on=["pybind11"],
            cmake_configure_options=[
                f"-DPython3_ROOT_DIR={Path(sys.prefix)}",
                "-DCALL_FROM_SETUP_PY:BOOL=ON",
                "-DBUILD_SHARED_LIBS:BOOL=OFF",
                "-DBUILD_PYTHON_BINDINGS:BOOL=ON",
                "-DINSTALL_XML:BOOL=OFF",
            ]
            + CIBW_CMAKE_OPTIONS,
        ),
    ],
    cmdclass=dict(build_ext=BuildExtension),
    package_dir={"": "extras/python"},
    include_package_data=False,
    package_data=package_data,
    zip_safe=False,
)
