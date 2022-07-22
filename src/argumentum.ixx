module;

#define ARGUMENTUM_BUILD_MODULE
#define ARGUMENTUM_EXPORT export

#include <cassert>
#include <cerrno>

export module Argumentum;

import std.core;
import std.filesystem;
import std.memory;
import std.regex;

#include "argparser.cpp"