# Python SDK
This is a wrapper around the [Rugged Science SDK](https://github.com/RuggedScience/SDK) with the only difference being that exceptions are automatically thrown. There are no `getLastError` or `getLastErrorString` functions.


## Installing
The package can be installed either by [compiling the sources](#compiling) or installing via `python -m pip install rssdk`.  

## Dio Example
```python
from rssdk import RsDio, OutputMode

dio = RsDio()
try:
    dio.init("ecs9000.xml")
except Exception as e:
    print(e)
    exit(1)

dio.setOutputMode(1, OutputMode.Source)

dio.digitalRead(1, 1)
dio.digitalWrite(1, 11, True)

```

## PoE Example
```python
from rssdk import RsPoe, PoeState

poe = RsPoe()
try:
    poe.init("ecs9000.xml")
except Exception as e:
    print(e)
    exit(1)

poe.getPortState(3)
poe.setPortState(PoeState.StateDisabled)
```

# Compiling
The Python bindings are built against the standard C++ SDK libraries using [pybind11](https://pybind11.readthedocs.io/en/stable/). There are two ways to compile them. Either using Python's build system or by using cmake directly. Both options assume you have have already cloned the repository as shown in the step below.

***NOTE: It is best to [enable long paths](https://learn.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation?tabs=registry#enable-long-paths-in-windows-10-version-1607-and-later) in Windows when compiling using the Python build system.***

```console
git clone https://github.com/ruggedscience/SDK
cd SDK
```


## Python Build
Using Python's build system is the recommended way to build the bindings. It will produce a wheel that can be installed using `pip` and handles the installation of the required build modules. It will install all of the modules inside an isolated virtual environment that will be deleted after compilation is finished. 

1) Install the Python build module.
    ```console
    python -m pip install build
    ```

2) Build the source distribution and wheel.
    ```console
    python -m build
    ```

3) Install the newly built wheel.
    ```console
    python -m pip install ./dist/<name of wheel file>.whl
    ```
