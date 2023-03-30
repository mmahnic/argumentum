#!/usr/bin/env bash

# Install the library through VCPKG then use it as a static library with find-package.

if [ "$1" == "-u" ]; then
   # Use a special repo and branch for testing before publishing on VCPKG.
   mode=update
   echo "Mode: Test before publishing to vcpkg."
fi

here=$(pwd)
thisscript=$(basename $0)
workdir=$here/xdata/$thisscript
argumentumgit="file://$(realpath ../../../argumentum)"
builddir=out/build

if [ -d $workdir ]; then
   rm -rf $workdir
fi

if [ ! -d $workdir ]; then
   mkdir -p $workdir
fi

create_test_dir() {
   local testdir=$workdir/basic
   mkdir $testdir
   cd $here
   cp CMakeLists-installed.txt.in $testdir/CMakeLists.txt
   cp -r src $testdir/

   cd $testdir
   mkdir -p $builddir
}

vcpkg_install_argumentum() {
   cd $workdir
   if [ "$mode" == "update" ]; then
      # Use this repo and branch when preparing a new release.
      git clone -b update_argumentum --single-branch https://github.com/mmahnic/vcpkg.git
   else
      git clone https://github.com/microsoft/vcpkg.git
   fi

   cd $workdir/vcpkg

   if [ "$OSTYPE" == "linux-gnu" ]; then
      bash bootstrap-vcpkg.sh
      ./vcpkg install argumentum
   elif [ "$OSTYPE" == "msys" ]; then
      cmd.exe /c bootstrap-vcpkg.bat
      ./vcpkg.exe install argumentum
      export VCPKG_DEFAULT_TRIPLET=x64-windows
   fi
}

configure() {
   cd $workdir/basic
   # local debug=--debug-output
   cmake -S . -B $builddir $debug -DCMAKE_TOOLCHAIN_FILE=$workdir/vcpkg/scripts/buildsystems/vcpkg.cmake
}

build() {
   cd $workdir/basic
   # local debug=--debug-output
   cmake --build $builddir $debug
}

export CMAKE_TOOLCHAIN_FILE=$workdir/vcpkg/scripts/buildsystems/vcpkg.cmake

create_test_dir
vcpkg_install_argumentum

configure
build

