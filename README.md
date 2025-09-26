```
mkdir build
cd build
cmake ..
```

You will probably need to resolve a bunch of cmake_minimum_version errors when building with CMake for windows.

Just go into build/_deps, find the offending dependency, and change the minimum version of that CMakeLists.txt 
