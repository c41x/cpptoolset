@echo off
pushd %~dp0
mkdir project_vs

cd project_vs
cmake ../../ -G"Visual Studio 14 2015 Win64"
cd ../../ext

cd glfw
mkdir build_vs
cd build_vs
echo Compiling GLFW
cmake ../ -G"Visual Studio 14 2015 Win64" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF
cmake --build . --config Release
cd ../..

cd zlib
mkdir build_vs
cd build_vs
echo Compiling zlib
cmake ../ -G"Visual Studio 14 2015 Win64" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cd ../..

cd libpng
mkdir build_vs
cd build_vs
echo Compiling libpng
cmake ../ -G"Visual Studio 14 2015 Win64" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cd ../..

cd libjpeg-turbo
mkdir build_vs
cd build_vs
echo Compiling libjpeg-turbo
cmake ../ -G"Visual Studio 14 2015 Win64" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cd ../..

rem assimp built in build/vs
cd assimp
mkdir build
cd build
mkdir vs
cd vs
echo Compiling assimp
cmake ../../ -G"Visual Studio 14 2015 Win64" -DCMAKE_BUILD_TYPE=Release -DASSIMP_BUILD_SAMPLES=OFF -DASSIMP_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=OFF
cmake --build . --config Release
cd ../../..

popd
