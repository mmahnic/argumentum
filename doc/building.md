# Building and consuming the library

The recommended way to consume the library is build and link the static
library.  A dynamic version is planned.

The library is also available in VCPKG (`./vcpkg install argumentum`).

If building is not an option, the library can be used as header-only by cloning
the repository and using the appropriate include path.


## Ubuntu/Debian: Build, install and use the static library

```
cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release
cd build
cmake --build .
sudo make install
```

CMakeLists.txt:

```
find_package( Argumentum CONFIG REQUIRED )
set( CMAKE_CXX_STANDARD 17 )

add_executable( example
   main.cpp
   )
target_link_libraries( example
   Argumentum::argumentum
   )
```

main.cpp:

```
#include <argumentum/argparse.h>
using namespace argumentum;
```


## Vcpkg

In `vcpkg` directory:

```
./vcpkg install argumentum
```

Use in your CMake project:

```
cmake -H. -Bbuild -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```


## Header-only from a cloned repository

```
git clone https://github.com/mmahnic/argumentum.git 3rdparty/argumentum
```

or

```
git submodule add https://github.com/mmahnic/argumentum.git 3rdparty/argumentum
```

CMakeLists.txt:

```
include_directories( 3rdparty/argumenutm/include )
```


main.cpp:

```
#include <argumentum/argparse-h.h>
using namespace argumentum;
```

Note that the header-only version has a separate header `<argumentum/argparse-h.h>`.

Compile from the command line:

```
g++ -Wall -Werror -std=c++17 -o example -I $(pwd)/argumentum/include main.cpp
```


## Use the static library from a cloned repository

```
git clone https://github.com/mmahnic/argumentum.git 3rdparty/argumentum
```

or

```
git submodule add https://github.com/mmahnic/argumentum.git 3rdparty/argumentum
```

CMakeLists.txt:

```
cmake_minimum_required( VERSION 3.1 )
project( Basic VERSION 0.0.1 )

add_subdirectory( 3rdparty/argumentum )

set( CMAKE_CXX_STANDARD 17 )

include_directories( 3rdparty/argumentum/include )

add_executable( example main.cpp )
target_link_libraries( example argumentum )
add_dependencies( example argumentum )
```


main.cpp:

```
#include <argumentum/argparse.h>
using namespace argumentum;
```

Compile from the command line:

```
cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release
cd build
cmake --build .
```

