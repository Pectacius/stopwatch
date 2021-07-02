# Stopwatch

API wrapper around [PAPI](https://icl.utk.edu/papi/)

### Dependencies
- PAPI (Instructions can be found [here](https://bitbucket.org/icl/papi/wiki/Downloading-and-Installing-PAPI.md))

## How to build
Since the PAPI installation does to provide any CMake targets, Stopwatch attempts to find PAPI and create a target for
it to use in CMake builds. It will first search the environment variable `PAPI_DIR` which the PAPI installation guide
recommends to set if installing in a non-orthodox location. Default locations such as `/usr/local` will be checked after
the environment variable `PAPI_DIR` is checked.

### Building Stopwatch
```shell
cd stopwatch
cmake -Bbuild
cmake --build build
```

To also build the examples which currently includes some matrix multiplication, specify `cmake -Bbuild -DBUILD_EXAMPLES=ON`
instead of `cmake -Bbuild`

### Installing Stopwatch
Running
```shell
sudo cmake --build build -- install
```
will install in the default location of `usr/local`

The default location can be changed by setting the `CMAKE_INSTALL_PREFIX` cached variable to the path that Stopwatch
should be installed.

## Using Stopwatch
In the CMakeLists.txt add:
- `find_package(Stopwatch REQUIRED)` to make the library available
- `target_link_libraries(target Stopwatch::Stopwatch)` to link with the library

At the moment, the Stopwatch interface should be used like so:
- call `init_stopwatch` to initialize the appropriate structures
- call `start_stopwatch` to start the monotonic clock
- to measure the performance of the function `foo`:
```c
read_stopwatch(start_reading);
foo();
read_stopwatch(end_reading);
```
where `start_reading` is of type `StopwatchReadings` and holds the state of the monotonic clock before `foo` executes and
`end_reading` is also of type `StopwatchReadings` which holds the state of the monotonic clock after `foo` is done executing.
The difference would be the actual value of `foo`'s performance.