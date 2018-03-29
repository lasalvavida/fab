# Fab

C++ Build Utility

## About
Fab is a rough cut of a next generation build system for C/C++.

:warning: This is alpha software, and likely won't work for your existing project,
use at your own risk. :warning:

### What does fab do differently?
The philosophy behind fab is that builds should be configured, not scripted.

90% of projects don't need a custom build script, and if they set up their build
in a more structured, standardized way, it would make code reuse and 
interoperability between C/C++ projects tremendously easier.

The goal is to make a build system whose build configurations:
  * Contain no platform/compiler specific options
  * Make it easy to link in dependencies
  * Allow you to depend on only a portion of a much larger project

## Build
### Build fab with CMake

```bash
mkdir build
cd build
cmake ..
make
```

### Build fab with fab
Fab is self-hosted, so once you build it, you can use it to build itself.

```
./build/fab
```