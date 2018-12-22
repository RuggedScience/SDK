# SDK
SDK for Rugged Science embedded PCs

For pre-built libraries please see the "releases" tab above.

# Building
For best results, create a seperate build folder inside the root of this SDK. From inside of that folder run the following commands.  
 `cmake ..`  
 `cmake --build . --config release`  
   
For debug builds remove the `--config release`  
For static builds run `cmake .. -DBUILD_SHARED_LIBS=OFF` instead of `cmake ..`  

Windows requires 64-bit builds due to driver limitations. To do this you will need to specify the generator and architecture using the "-G" switch.  
  
`cmake -G "Visual Studio 15 2017 Win64" ..`  
`cmake --build .`

To obtain a copy of the Windows driver, please contact your Rugged Science sales representative.

Once the build process is finished, you will find a copy of the libraries and examples under the 'build' directory in their corresponding directories.

# Using
Both the DIO and POE libraries have an `init` function which must be run before any other functions. These init functions take one argument which should contain a path to the XML file that is specific to your unit. This file is supplied by Rugged Science and can be found in this repo under the 'xml' directory.

All the other functions should be self documenting. For any issues or questions, please contact your Rugged Science sales representative.