mkdir debug
mkdir release

cd debug
cmake ../../ -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug
cd ..

cd release
cmake ../../ -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
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
