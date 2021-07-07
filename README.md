# Stopwatch

API wrapper for [PAPI](https://icl.utk.edu/papi/)

### Dependencies
- PAPI (Instructions can be found [here](https://bitbucket.org/icl/papi/wiki/Downloading-and-Installing-PAPI.md)) or be
installed with a package manager with the name `libpapi-dev`

## How to build
Since the PAPI installation does to provide any CMake targets, Stopwatch attempts to find PAPI and create a target for
it to use in CMake builds. It will look into default locations such as `/usr/lib`. If installing PAPI with a package 
manager, PAPI should be found with no issues. Support for non-orthodox installation directories that usually come from
building from source will come later...

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
- call `init_event_timers` to initialize the appropriate structures and start the monotonic event timers
- wrap the function(s) to measure with the calls to `record_start_measurements` and `record_end_measurements`
- call `destroy_event_timers` to clean up the resources used

Example of measuring the performance of `foo`:
```c
init_event_timers();
record_start_measurements(<foo_call_num>, "foo", 0);
foo();
record_end_measurements(<foos_call_num>);
destroy_event_timers();
```
