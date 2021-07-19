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

#### Custom Install Location:
The default location can be changed by setting the `CMAKE_INSTALL_PREFIX` cached variable to the path that `Stopwatch`
should be installed. Note that when installing in a custom location, CMake must be pointed to the installation directory
by setting the`Stopwatch_DIR` cache variable to `<install_path>/lib/cmake/Stopwatch`.

##### Example:
If `Stopwatch` is installed in `~/stopwatch_install`and `project_foo` depends on `Stopwatch` then the CMake cache for
`project_foo` must be generated as so assuming that the build directory is `build`

```sh
cmake -Bbuild -DStopwatch_DIR=~/stopwatch_install/lib/cmake/Stopwatch
```

### Optional CMake Configurations
- `-DBUILD_C_EXAMPLES=ON` will also build the C example programs. The default is OFF
- `-DBUILD_FORTRAN_EXAMPLES=ON` will also build the Fortran example programs. The default is OFF

## Using Stopwatch
In the CMakeLists.txt add:
- `find_package(Stopwatch REQUIRED)` to make the library available
- `target_link_libraries(target Stopwatch::Stopwatch)` to link with the library
- For C targets, add:
  ```c 
  #include <stopwatch/stopwatch.h>
  ```
  for the interface definitions
- For Fortran targets, add:
   ```fortran
   #include <stopwatch/fstopwatch.F03>
   ```
  for the `mod_stopwatch` that contains the Fortran bindings

At the moment, the Stopwatch interface should be used like so:
1. call `stopwatch_init` to initialize the appropriate structures and start the monotonic event timers. The events to be
   measured can be specified by passing in the desired events as an array in the `events_to_add` argument and also the
   array's length in the `num_of_events` argument.
2. wrap the function(s) to measure with the calls to `stopwatch_record_start_measurements` and `stopwatch_record_end_measurements`
3. call `stopwatch_destroy` to clean up the resources used

Example of measuring the performance of a loop of matrix multiplication:
```c
#include <stopwatch/stopwatch.h>
int main() {
  const enum StopwatchEvents events[] = {CYCLES_STALLED_RESOURCE, L1_CACHE_MISS}; // Events to measure
  stopwatch_init(events, sizeof(StopwatchEvents)/ sizeof(enum StopwatchEvents)); // Initialize stopwatch

  int N = 500; // Size of matrix
  int itercount = 10; // Number of iterations

  float (*A)[N] = initialize_mat(N);
  float (*B)[N] = initialize_mat(N);
  float (*C)[N] = initialize_mat(N);

  // Matrix multiply loop
  stopwatch_record_start_measurements(1, "total-loop", 0); // Record start time of the entire loop
  for (int iter = 0; iter < itercount; iter++) {
    memset(C, 0, sizeof(float) * N * N); // clear C array
    stopwatch_record_start_measurements(2, "single-cycle", 1); // read start time of a single cycle
    mat_mul(N, A, B, C); // Perform the multiplication C = A * B
    stopwatch_record_end_measurements(2); // read end time of a single cycle
  }
  stopwatch_record_end_measurements(1); // Record end measurements of the entire loop
  stopwatch_print_result_table(); // Prints the results in a table format
  stopwatch_destroy(); // Clean up resources used
}
```

The result should look similar to this:
```shell
|-----------------------------------------------------------------------------------------------------------------|
| ID | NAME             | TIMES CALLED | TOTAL REAL CYCLES | TOTAL REAL MICROSECONDS | PAPI_RES_STL | PAPI_L1_TCM |
|-----------------------------------------------------------------------------------------------------------------|
| 1  | total-loop       | 1            | 9837387310        | 4455471                 | 5911716339   | 951256697   |
| 2  |     single-cycle | 10           | 9835929672        | 4454818                 | 5910669863   | 951113873   |
|-----------------------------------------------------------------------------------------------------------------|
```

More examples can be found in the `examples` folder.
