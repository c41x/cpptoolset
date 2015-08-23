mkdir debug
mkdir release

cd debug
cmake ../../ -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -ENABLE_SSSE3=OFF
cd ..

cd release
cmake ../../ -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -ENABLE_SSSE3=OFF
cd ../../ext

cd glfw
mkdir build
cd build
cmake ../ -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=TRUE
cmake --build . --config Release
cd ../..

cd zlib
mkdir build
cd build
cmake ../ -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cd ../..

cd libpng
mkdir build
cd build
cmake ../ -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cd ../..

cd libjpeg-turbo
mkdir build
cd build
sh ../configure
make
mv .libs libs
cd ../..
