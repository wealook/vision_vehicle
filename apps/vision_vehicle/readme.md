```bash
cmake .. -G"Unix Makefiles" -DARMLINUX=ON  -DCMAKE_TOOLCHAIN_FILE=CommonTool/make-deps/arm-gcc-toolchain.cmake
cmake --build . --target VisionVehicle
```


