@echo off
pushd %~dp0
mkdir debug
mkdir release

cd debug
cmake ../../ -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
cd ..

cd release
cmake ../../ -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cd ..

cd ../ext/glfw
mkdir build
cd build
cmake ../ -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
make

popd