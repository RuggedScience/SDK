rmdir /S /Q build
mkdir build
pushd build
cmake -G "Visual Studio 15 2017 Win64" ..
cmake --build . --config Release
popd