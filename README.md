# ![Rugged Science logo](https://www.ruggedscience.com/sites/default/files/RuggedScienceLogo.png)

The **Rugged Science SDK** is a set of cross platform libraries and utilities for controlling the hardware on Rugged Science embedded PCs. This SDK is not model dependent meaning it will function the same across all supported Rugged Science models. This is acheived by having individual XML files that contain hardware specific information. The correct file must be passed to the libraries and utilities for the libraries to know how to interact with the underlying hardware. 

*Passing the incorrect XML file will result in undefined behaviour and may harm the PC!*

**The latest prebuilt release can be found [HERE](https://github.com/ruggedscience/SDK/releases/latest). Be sure to install the driver!**  
**Looking for a Python package? Check out the Python [documentation](/extras/python/README.md).**

## API References
* [DIO Library - librsdio](./librsdio.md)
* [PoE Library - librspoe](./librspoe.md)
* [Error Library - librserrors](./errors.md)
* [DIO Utility - rsdioctl](./rsdioctl.md)
* [PoE Utility - rspoectl](./rspoectl.md)

# Building

## Prerequesites
The build process relies on the [CMake](https://cmake.org/) utility. This can be easily installed on most Linux distributions using `sudo apt install cmake` or `sudo yum install cmake`. For Windows, you can find the latest downloads [HERE](https://cmake.org/download/). Be sure to add the CMake directory to your PATH or use the full path when calling the cmake command. 

## Getting Started


1) Clone the repository using git and change directory into the newly created folder.
    ```console
    git clone https://github.com/ruggedscience/SDK
    cd SDK
    ```

2) Now all of the sources should be available to be built. It's best practice to create a seperate build directory to isolate the build files from the source files.
    ```console
    mkdir build
    cd build
    ```

3) From inside of that folder run the following commands.
    ```console
    cmake ..
    cmake --build .
    ```

This will result in all of the files being created in the build folder usually scattered across multiple directories. To make things easier to find you can specify an install folder that all of the files can be installed to.

```console
cmake -DCMAKE_INSTALL_PREFIX=install ..
cmake --build .
```

Doing this will put all of the necessary files into an install folder within your build directory. The files will be separated into subfolders by type.


## Build Type
Windows and Linux each behave a bit different in regards to the build type. Visual Studio will default to debug builds while gcc defaults to release builds.

To have Visual Studio build a release you will need to add the `--config release` flag as shown below.

```console
cmake --build . --config release
```

# Installing
As mentioned previously you can use the `CMAKE_INSTALL_PREFIX` option to set the installation location. If this options is omitted but the install command is invoked the files will be installed in standard locations based on the platform. 

Default locations:  
Windows: "C:\Program Files\rssdk\"  
Linux: "/usr/local"

```console
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
sudo cmake --install .
```

***NOTE:*** On Linux when installing to the default location you should run `sudo ldconfig` after installing the libraries to update the shared library cache.

# Cmake Build Options
| Option                    | Description                                                           |Default|
|---------------------------|-----------------------------------------------------------------------|-------|
| BUILD_SHARED_LIBS         | Build the libraries as shared libs (.dll / .so)                       | ON    |
| BUILD_UTILITIES           | Build the rsdioctl and rspoectl control utilities                     | OFF   |
| INSTALL_UTILITIES         | Install the utilities when the install command is invoked             | OFF   |
| INSTALL_SDK               | Install the SDK files when the install command is invoked             | ON    |
| BUILD_PYTHON_BINDINGS     | Build the Python bindings. See [docs](./extras/python/README.md)      | OFF   |
| INSTALL_PYTHON_BINDINGS   | Install the Python bindings package to the current Python interpreter | OFF   |
| BUILD_TESTS               | Build and enable all library tests                                    | OFF   |