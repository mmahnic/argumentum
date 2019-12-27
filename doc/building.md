# Building and consuming the library

The recommended way to consume the library is build and link the static
library.  A dynamic version is planned.

The library will also be available in VCPKG ([pull request](https://github.com/microsoft/vcpkg/pull/9478)).

If building is not an option, the library can be used as header-only by cloning
the repository and using the appropriate include path.

## Ubuntu/Debian: Build, install and use the static library

```
cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release
cd build
cmake --build .
sudo make install
```

### CMakeLists.txt

```
find_package( Argumentum CONFIG REQUIRED )
set( Argumentum_LIBRARIES Argumentum::argumentum-s )

set( CMAKE_CXX_STANDARD 17 )

add_executable( example
   main.cpp
   )
target_link_libraries( example
   ${Argumentum_LIBRARIES}
   )
```

### main.cpp

```
#include <argumentum/argparse.h>
using namespace argumentum;
```

## Vcpkg

In `vcpkg` directory:

```
./vcpkg install argumentum
```

### Use in your CMake project

```
cmake -H. -Bbuild -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```


## Header-only from a cloned repository

```
git submodule add https://github.com/mmahnic/argumentum.git 3rdparty/argumentum
```

### CMakeLists.txt

```
include_directories( 3rdparty/argumenutm/include )
```

