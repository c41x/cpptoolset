mkdir debug
mkdir release

cd debug
cmake ../../ -G"Unix Makefiles" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug -DENABLE_SSSE3=ON
cd ..

cd release
cmake ../../ -G"Unix Makefiles" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release -DENABLE_SSSE3=ON
cd ../../ext

cd glfw
mkdir build
cd build
cmake ../ -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF
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

# cd assimp
# mkdir build
# cd build
# cmake ../ -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DASSIMP_BUILD_SAMPLES=OFF -DASSIMP_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=OFF
# cmake --build . --config Release
# cd ../..
