#!/usr/bin/env bash

here=$(pwd)
workdir=xdata/test-fetchcontent-headeronly
argumentumgit="file://$(realpath ../../../argumentum)"
builddir=out/build

if [ -d $workdir ]; then
   rm -rf $workdir
fi

if [ ! -d $workdir ]; then
   mkdir -p $workdir
fi

create_test_git() {
   cp .gitignore $workdir/
   sed -e "s#@THIS_REPO@#$argumentumgit#" CMakeLists-fetchcontent.txt.in > $workdir/CMakeLists.txt
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

configure() {
   # local debug=--debug-output
   cmake -S . -B $builddir -D ARGUMENTUM_BUILD_STATIC_LIBS=OFF $debug
}

build() {
   # local debug=--debug-output
   cmake --build $builddir $debug
}

git config --global protocol.file.allow always
create_test_git
configure
build
git config --global --unset protocol.file.allow

