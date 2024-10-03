# CREST Client C++ Library.
This library implements the client API to interact with the CREST Server.

## Project structure
CrestApi project structure:

| Directory  | Description                                                |
|------------|------------------------------------------------------------|
| CrestApi   | Contains the header files for the library (CrestApi.h).    |
| data       | Contains the data files used in the CrestApi examples.     |
| doc        | Contains examples for the library.                         |
| scripts    | Contains the bash scripts used to create objects in the CREST Server. |
| src        | Contains the library source code (CrestApi.cxx).           |
| test       | Contains the ATLAS test (test.cxx). *(To be written later)*|
| CMakeList.txt | CMake file for the package CrestApi.                     |

## Installation
We provide this code as a standalone library.
It requires:
* the `nlhomann` JSON library (this is delivered within this package)
* the `CURL` library
* the `BOOST` library

### Standalone (lxplus)
* Create a build directory
```shell
mkdir build
```
* Create Makefiles
```shell
cd ./build
cmake .. 
```
* Compile library and examples
```shell
make
```
## Testing
The simple examples are in the `doc` directory. The executables will be created in the `build` directory.

* `crest_example_server`: this file utilizes a *CREST* server, so you need to provide a *URL* as argument. In general you can use `http://crest-undertow-api.web.cern.ch`

* `crest_example_fs` : this file utilizes the file system.


