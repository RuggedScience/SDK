# SDK
SDK for Rugged Science embedded PCs

For pre-built libraries please see the "releases" tab above.

# Building
For best results, create a seperate build folder inside the root of this SDK. From inside of that folder run the following commands.  
 `cmake .. -DCMAKE_INSTALL_PREFIX=../install`  
 `cmake --build . --config release  --target INSTALL`  
   
For debug builds remove the `--config release`  
For static builds run `cmake .. -DBUILD_SHARED_LIBS=OFF` instead of `cmake ..`  

If `-DCMAKE_INSTALL_PREFIX` is omitted, the files will be installed in the default location for the O.S.
Windows: "C:\Program Files\rssdk\"
Linux: "/usr/local"

By default the examples are installed to `${CMAKE_INSTALL_PREFIX}/bin`. This can be turned off with -DINSTALL_EXAMPLES=OFF

Windows requires 64-bit builds due to driver limitations. To do this you will need to specify the generator and architecture using the "-G" switch.  
  
`cmake -G "Visual Studio 15 2017 Win64" .. -DCMAKE_INSTALL_PREFIX=../install`  
`cmake --build . --target INSTALL`

To obtain a copy of the Windows driver, please contact your Rugged Science sales representative.

Once the build process is finished, you will find a copy of the libraries and examples inside of the install directory you supplied.

# Using
Both the DIO and POE libraries have an `init` function which must be run before any other functions. These init functions take one argument which should contain a path to the XML file that is specific to your unit. This file is supplied by Rugged Science and can be found in this repo under the 'xml' directory.

All the other functions should be self documenting. For any issues or questions, please contact your Rugged Science sales representative.