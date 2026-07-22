# ![Rugged Science logo](./docs/rugged-logo.png)

The **Rugged Science SDK** is a set of cross-platform libraries and utilities for controlling the hardware on Rugged Science embedded PCs. This SDK is not model dependent meaning it will function the same across all supported Rugged Science models. This is achieved by having hardware specific config files (XML) that have to be provided to the SDK. The correct file must be passed to the libraries and utilities for the libraries to know how to interact with the underlying hardware. 

> **WARNING:** The correct XML file must be passed to the libraries and utilities for them to know how to interact with the underlying hardware. **Passing the incorrect XML file will result in undefined behavior and may harm the PC!** If you are unsure of the correct file to use, contact your sales rep.

## Quickstart (Command Line Utilities)
If you just want to control the DIO or PoE functions without writing any code, you can use the prebuilt command-line utilities. 

### 1. Download
Start by downloading the [Latest prebuilt release](https://github.com/ruggedscience/SDK/releases/latest). Look under the `Assets` dropdown for a zip file that starts with `rssdk` and download the appropriate file for your environment. 

* Windows: `rssdk-windows-2019.zip`
* Linux: `rssdk-ubuntu-20.04.zip`

If you are running on Windows, you will also need to download and install the [driver](./driver/dio-driver_x64.zip). Once downloaded, extract the files and run the `install.exe` file as an administrator to install it.

### Usage
Extract the release files and navigate to the `bin` folder. In there you will find two utilities: `rsdioctl` and `rspoectl`. These utilities can be used to query and control the DIO pins and PoE ports respectively. For example, to read the state of pin 1 on an ECS-9000 you would use the following command:

```bash
rsdioctl.exe .\ecs9000.xml state --pin 1 -h
```

Command Breakdown:
* `rsdioctl.exe`: The utility you are running.
* `.\ecs9000.xml`: The hardware config file to use.
* `state`: The command to perform (in this case, reading the current state).
* `--pin 1`: The target pin you want to query.
* `-h`: Formats the output to be human-readable ("high" or "low" instead of "1" or "0").

Below is an example of setting the state of pin 11 on the same unit using the shorthand syntax.
```bash
rsdioctl.exe .\ecs9000.xml -p 11 s=high
```

To see a full list of available commands and options, use the `--help` flag.
```bash
rsdioctl.exe --help
rspoectl.exe --help
```

# Configuration File
In the above example we assumed there was an `ecs9000.xml` file in the `bin` folder along with the utilities. You will need to manually download the actual hardware config file for your unit and pass that to the utility. Look in the [XML](./xml/) directory for a config file that matches your model number and download that. If you are unsure which config file to use, reach out to your sales rep for guidance.

## API References
* [DIO Library - librsdio](./docs/librsdio.md)
* [PoE Library - librspoe](./docs/librspoe.md)
* [Error Library - librserrors](./docs/errors.md)

# Building

## Prerequisites
The build process relies on the [CMake](https://cmake.org/) utility. This can be easily installed on most Linux distributions using `sudo apt install cmake` or `sudo yum install cmake`. For Windows, you can find the latest downloads [HERE](https://cmake.org/download/). Be sure to add the CMake directory to your PATH or use the full path when calling the cmake command. 

## Getting Started


1) Clone the repository using git and change directory into the newly created folder.
    ```console
    git clone https://github.com/ruggedscience/SDK
    cd SDK
    ```

2) Now all of the sources should be available to be built. It's best practice to create a separate build directory to isolate the build files from the source files.
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
