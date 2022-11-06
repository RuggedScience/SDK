# ![Rugged Science logo](https://www.ruggedscience.com/sites/default/files/RuggedScienceLogo.png)

The **Rugged Science SDK** is a set of cross platform libraries and utilities for controlling the hardware on Rugged Science embedded PCs. This SDK is not model dependent meaning it will function the same across all supported Rugged Science models. This is acheived by having individual XML files that contain hardware specific information. The correct file must be passed to the libraries and utilities for the libraries to know how to interact with the underlying hardware. 

*Passing the incorrect XML file will result in undefined behaviour and may harm the PC!*

**The latest prebuilt release can be found [HERE](../../releases/latest).**

## API References
* [DIO Library - librsdio](./librsdio.md)
* [PoE Library - librspoe](./librspoe.md)
* [DIO Utility - rsdioctl](./rsdioctl.md)
* [PoE Utility - rspoectl](./rspoectl.md)

# Building

## Prerequesites
The build process relies on the [CMake](https://cmake.org/) utility. This can be easily installed on most Linux distributions using `sudo apt install cmake` or `sudo yum install cmake`. For Windows, you can find the latest downloads [HERE](https://cmake.org/download/). Be sure to add the CMake directory to your PATH or use the full path when calling the cmake command.

## Getting Started

For best results, create a seperate build folder inside the root of this SDK.

`mkdir build`\
`cd build`

From inside of that folder run the following commands.  

 `cmake ..`\
 `cmake --build .`

### Static Builds
For static builds add the  `-DBUILD_SHARED_LIBS=OFF` flag. *Useful when you are just looking to build the control utilities (rspoectl and rsdioctl).* 

`cmake .. -DBUILD_SHARED_LIBS=OFF`

Windows requires 64-bit builds due to driver limitations. To do this you will need to specify the generator and architecture using the "-G" switch.
  
`cmake -G "Visual Studio 15 2017 Win64" ..`\
`cmake --build .`

**The Windows drivers must be installed prior to using this SDK. They can be found in the latest release under `Windows\X64\Drivers\`.**

## Debug and Release
Windows and Linux each behave a bit different in regards to the build type. Visual Studio will default to debug builds while gcc defaults to release builds.

To have Visual Studio build a release you will need to add the `--config release` flag as shown below.

`cmake -G "Visual Studio 15 2017 Win64" ..`\
`cmake --build . --config release`

In most other situations you will need to use the `-DCMAKE_BUILD_TYPE` switch shown below.

`cmake -DCMAKE_BUILD_TYPE=Debug ..`  
`cmake --build .`

# Installing
Use the `-DCMAKE_INSTALL_PREFIX=` option in the first command to set the installation location. Then use `--target install` during the build command.
Default locations:  
Windows: "C:\Program Files\rssdk\"  
Linux: "/usr/local"

`cmake .. -DCMAKE_INSTALL_PREFIX=../install`  
`cmake --build . --target install`

***NOTE:*** On Linux you should run `sudo ldconfig` after installing the libraries to update the shared library cache.

By default the control utilities are installed to `${CMAKE_INSTALL_PREFIX}/bin`. This can be turned off with `-DINSTALL_UTILITIES=OFF`.

Once the build process is finished, you will find a copy of the libraries and examples inside of the build directory or the install directory.

# Python Bindings
A Python package for this SDK can be built by setting the `BUILD_PYTHON_BINDINGS` flag when running the cmake config command.

`cmake .. -DBUILD_PYTHON_BINDINGS=ON`  
`cmake --build .`  

On Windows you need to specify a single configuration when building the Python bindings.

`cmake -DBUILD_SHARED_LIBS=OFF -DBUILD_PYTHON_BINDINGS=ON -DCMAKE_CONFIGURATION_TYPES:STRING=Release ..`  
`cmake --build . --config release`  

This requires that the current Python environment has the build package installed. This can be done using `python -m pip install build`.

This should result in an `rssdk-*.whl` file in the build folder which can be installed using `python -m pip install ./rssdk-*.whl`

For examples showing how to use the Python bindings see the [documentation](./extras/python-bindings/README.md).

## Troubleshooting
If you get an error about not being able to find the shared library when you try to import the module there are two things you can do. Either [install](#installing) the libraries or do a [static](#static-builds) build.
