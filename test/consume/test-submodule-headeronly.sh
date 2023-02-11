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

create_test_git() {
   cd $here
   cp .gitignore $workdir/
   cp CMakeLists.txt.in $workdir/CMakeLists.txt
   mkdir -p $workdir/src
   cp -r src-headeronly/* $workdir/src/

   cd $workdir
   mkdir -p $builddir

   git init
   git add .gitignore
   git commit -m "Add gitignore"
   git add src
   git commit -m "Initial import"
}

add_submodule() {
   cd $workdir
   git -c "protocol.file.allow=always" submodule add -- $argumentumgit external/argumentum
   git submodule set-branch -b improve_cmake -- external/argumentum
   git submodule update --init
}

configure() {
   cd $workdir
   # local debug=--debug-output
   cmake -S . -B $builddir -D ARGUMENTUM_BUILD_STATIC_LIBS=OFF $debug
}

build() {
   cd $workdir
   # local debug=--debug-output
   cmake --build $builddir $debug
}

create_test_git
add_submodule
configure
build

