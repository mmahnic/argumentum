#!/usr/bin/env bash

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
   cd $workdir

   git -c "protocol.file.allow=always" clone $argumentumgit argumentum

   cd $workdir/argumentum
   mkdir -p $builddir

   local testdir=$workdir/basic
   mkdir $testdir
   cd $here
   cp CMakeLists-installed.txt.in $testdir/CMakeLists.txt
   mkdir -p $testdir/src
   cp -r src-headeronly/* $testdir/src/

   cd $testdir
   mkdir -p $builddir
}

configure_argumentum() {
   cd $workdir/argumentum
   # local debug=--debug-output
   cmake -S . -B $builddir \
      -D ARGUMENTUM_BUILD_STATIC_LIBS=OFF \
      -D ARGUMENTUM_INSTALL_HEADERONLY=ON \
      -D CMAKE_INSTALL_PREFIX="$workdir/install" \
      $debug
}

build_argumentum() {
   cd $workdir/argumentum
   # local debug=--debug-output
   cmake --build $builddir $debug
}

install_argumentum() {
   cd $workdir/argumentum
   # local debug=--debug-output
   cmake --install $builddir $debug
}

configure() {
   cd $workdir/basic
   # local debug=--debug-output
   cmake -S . -B $builddir $debug
}

build() {
   cd $workdir/basic
   # local debug=--debug-output
   cmake --build $builddir $debug
}

create_test_dir
configure_argumentum
build_argumentum
install_argumentum

export CMAKE_PREFIX_PATH=$workdir/install

configure
build


