Parallel Processor
==================

Cross-platform command-line tool to parallelize file based execution tasks.  

Features
========

Parallel Processor was developed with file based data analysis tasks in mind.  
Tasks are expected to operate on input files to generate output files.  
Due to this some design decisions were made different than compared to those of  
GNU make, tup and other tools that aim to automate build processes. The overall  
design, however, is quite similar providing the following features:  
- cross-platform (tested for Windows and Linux)
- multi-processor support
- Unicode support
- static dependency analysis
- support for incomplete and incremental dependency resolution
- readable command output (not mixed like in GNU make for parallel tasks)
- source file selection via regular expressions
- explicit dependency definition
- progress indicator
- custom shell support by defining the command call syntax

Usage
=====

See [user manual](doc/pp-user-manual.pdf).  

Building
========

The following dependencies are given:  
- C99
- C++03
- Boost 1.54.0
- SQLite 3.8.11.1
- NSIS 2.46.5 Unicode (optional)

SQLite was built with SQLITE_ENABLE_UPDATE_DELETE_LIMIT=1 and  
SQLITE_ENABLE_COLUMN_METADATA=1.  
  
Edit Makefile to match your target system configuration.  
Building the program:  

    make

To build the setup (needs NSIS):  

    make setup

[![Linux GCC Build Status](https://img.shields.io/travis/daniel-starke/pp/master.svg?label=Linux)](https://travis-ci.org/daniel-starke/pp)
[![Windows LLVM/Clang Build Status](https://img.shields.io/appveyor/ci/danielstarke/pp/master.svg?label=Windows)](https://ci.appveyor.com/project/danielstarke/pp)    

MinGW GCC build fails in AppVeyor due to memory limitations.  
MinGW Clang/LLVM build fails when throwing an exception in the resulting executable.  
MSVC is not supported due to compiler bugs (unlimited memory consumption).

License
=======

See [copying file](doc/COPYING).  
