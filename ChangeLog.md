
# Changelog

## [0.3.2]

### Added

- It is possible to collect parameters that will be forwarded to a subprocess.  The syntax is
  similar to the one used in GCC.
- Multiple names can be defined for meta-varaiables in `metavar()` by specifying a list of strings
  instead of a simple string.
- Added the method `add` as an alias for `add_parameter`.
- The method `absent` is now an alias for `default_value`.
- Added shell tests for various library usage scenarios.

### Changed

- Some targets are available only when the Argumentum project is top-level.

### Fixed

- The header-only version can be installed.
- The library can be used from a Git submodule using modern CMake.


## [0.3.0]

### Added

- It is now possible to add an instantiated instance of CommandOptions with `add_command`.  This
  makes it easier to access global options from within command options.  See the test
  shouldAccessParentOptionsFromCommand in `command_t.cpp` and the new example in the README file.
- The help formatter can be changed in parser configuration. See the test shouldChangeHelpFormatter
  in `parserconfig_t.cpp`.

### Changed

- The interface for defining program parameters has changed.  The parameters are defined through the
  `params()` method of the `argument_parser` class.  The old `argument_parser` methods for defining
  parameters are deprecated and will be removed.
- Fix: Treat negative numbers as numbers when they can not be mistaken for options.


## [Next]

### Fixed

- The optional<vector> targets are now filled correctly.

### Changed

- For options with vector targets the default count changed from `minagrs(0)` to `minargs(1)`.  For
  options with `optional<vector>` targets the default is still `minargs(0)`.
- When an option with a vector target has `minargs(0)` a flagValue is added to the vector only if
  the vector is empty.

