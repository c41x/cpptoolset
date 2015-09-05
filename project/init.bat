@echo off
pushd %~dp0
mkdir debug
mkdir release

cd debug
cmake ../../ -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
cd ..

cd release
cmake ../../ -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cd ../../ext

cd glfw
mkdir build
cd build
echo Compiling GLFW
cmake ../ -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF
cmake --build . --config Release
cd ../..

cd zlib
mkdir build
cd build
echo Compiling zlib
cmake ../ -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cd ../..

cd libpng
mkdir build
cd build
echo Compiling libpng
cmake ../ -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cd ../..

cd libjpeg-turbo
mkdir build
cd build
echo Compiling libjpeg-turbo
cmake ../ -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cd ../..

cd assimp
mkdir build
cd build
echo Compiling assimp
cmake ../ -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DASSIMP_BUILD_SAMPLES=OFF -DASSIMP_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=OFF
cmake --build . --config Release
cd ../..

popd
