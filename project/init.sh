mkdir debug
mkdir release

cd debug
cmake ../../ -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug
cd ..

cd release
cmake ../../ -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cd ..

cd ../ext/glfw
mkdir build
cd build
cmake ../ -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
