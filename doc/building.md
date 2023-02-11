# Building and consuming the library

## Build, install and use the static library

Build and install the library from the sources:

```bash
cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release
cd build
cmake --build .
sudo make install
```

Use the static library in a project:

```cmake
# CMakeLists.txt

cmake_minimum_required( VERSION 3.1 )
project( Example VERSION 0.0.1 )

find_package( Argumentum CONFIG REQUIRED )
set( CMAKE_CXX_STANDARD 17 )

add_executable( example
   main.cpp
   )
target_link_libraries( example
   PRIVATE
   Argumentum::argumentum
   )
```


```C++
// main.cpp:

#include <argumentum/argparse.h>
using namespace argumentum;
```

## Install and use the header-only library

Differences compared to the use of the installed static library:

- Define `-DARGUMENTUM_INSTALL_HEADERONLY=ON` when calling `cmake`.
- The static library doesn't have to be built when using `cmake`:
  `-DARGUMENTUM_BUILD_STATIC_LIBS=OFF`.
- Link to the target `Argumentum::headers`.
- Use the header `<argumentum/argparse-h.h>`.

Build and install the library from the sources:

```bash
cmake -H. -Bbuild -DARGUMENTUM_INSTALL_HEADERONLY=ON -DARGUMENTUM_BUILD_STATIC_LIBS=OFF
cd build
cmake --build .
sudo make install
```

Use the header-only library in a project:

```cmake
# CMakeLists.txt:

cmake_minimum_required( VERSION 3.1 )
project( Example VERSION 0.0.1 )

find_package( Argumentum CONFIG REQUIRED )
set( CMAKE_CXX_STANDARD 17 )

add_executable( example
   main.cpp
   )
target_link_libraries( example
   PRIVATE
   Argumentum::headers
   )
```


```C++
// main.cpp:

#include <argumentum/argparse-h.h>
using namespace argumentum;
```


## Vcpkg

In `vcpkg` directory:

```bash
./vcpkg install argumentum
```

Use in your CMake project:

```bash
cmake -H. -Bbuild -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```


## Using the library from a cloned repository without installing

Download the library with `git clone`

```bash
git clone https://github.com/mmahnic/argumentum.git external/argumentum
```

or with `git submodule` inside your project's git repository

```bash
git submodule add https://github.com/mmahnic/argumentum.git external/argumentum
```

### Use the static library from a cloned repository

Add the library to a project as a subdirectory:

```cmake
# CMakeLists.txt:

cmake_minimum_required( VERSION 3.1 )
project( Example VERSION 0.0.1 )

add_subdirectory( external/argumentum )

set( CMAKE_CXX_STANDARD 17 )

add_executable( example main.cpp )
target_link_libraries( example 
   PRIVATE
   Argumentum::argumentum
   )
```


```C++
// main.cpp:

#include <argumentum/argparse.h>
using namespace argumentum;
```

To build the project, building of the static library must be enabled:

```bash
cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release -DARGUMENTUM_BUILD_STATIC_LIBS=ON
cmake --build build
```


### Use the Header-only version from a cloned repository

Differences compared to the use of the cloned static library:

- Link to the target `Argumentum::headers`.
- Use the header `<argumentum/argparse-h.h>`.
- The static library doesn't have to be built when using `cmake`:
  `-DARGUMENTUM_BUILD_STATIC_LIBS=OFF`.

```cmake
# CMakeLists.txt:

cmake_minimum_required( VERSION 3.1 )
project( Example VERSION 0.0.1 )

add_subdirectory( external/argumentum )

set( CMAKE_CXX_STANDARD 17 )

add_executable( example main.cpp )
target_link_libraries( example 
   PRIVATE 
   Argumentum::headers
   )
```


```C++
// main.cpp:

#include <argumentum/argparse-h.h>
using namespace argumentum;
```

Build with `cmake`:

```bash
cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Or compile from the command line if not using CMake:

```bash
g++ -Wall -Werror -std=c++17 -o example -I $(pwd)/external/argumentum/include main.cpp
```


## Using the library with CMake FetchContent

The library can also be used with `FetchContent` without cloning the library in advance.

```cmake
cmake_minimum_required( VERSION 3.11 )
project( Example VERSION 0.0.1 )

set( CMAKE_CXX_STANDARD 17 )

include(FetchContent)

FetchContent_Declare(Argumentum
   GIT_REPOSITORY https://github.com/mmahnic/argumentum.git
   GIT_TAG master
)
FetchContent_MakeAvailable(Argumentum)

add_executable( example main.cpp )
target_link_libraries( example 
   PRIVATE 
   Argumentum::argumentum
   # With header-only: Argumentum::headers
   )
```


```C++
// main.cpp:

#include <argumentum/argparse.h>
// With header-only: #include <argumentum/argparse-h.h>

using namespace argumentum;
```

To use the static version of the library, building of the static version must be enabled with
`-DARGUMENTUM_BUILD_STATIC_LIBS=ON`:

```bash
cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release -DARGUMENTUM_BUILD_STATIC_LIBS=ON
cmake --build build
```
