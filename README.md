# Stopwatch

API wrapper for [PAPI](https://icl.utk.edu/papi/)

### Dependencies
- PAPI (Instructions can be found [here](https://bitbucket.org/icl/papi/wiki/Downloading-and-Installing-PAPI.md)) or be
installed with a package manager with the name `libpapi-dev`

## Build Instructions

### Core Dependency - PAPI
Since the PAPI installation does to provide any CMake targets, Stopwatch attempts to find PAPI and create a target for
it to use in CMake builds. 

The search order is:
1.  Searches for `PAPI` using `pkg-config` if `pkg-config` exists on the system. This means on Cray systems, executing
    `module load papi` will allow `PAPI` to be found.
2.  Searches for `PAPI` using `find_path`. This means it will look into default locations for `papi.h` and `libpapi.a`.
    An extra hint using the shell variable `PAPI_DIR` is provided to assist with finding `PAPI` if `PAPI` is not
    installed in a default location i.e., when building `PAPI` from source. `PAPI_DIR` is identical to the `PAPI_DIR`
    that is specified in the 
    [build from source instructions](https://bitbucket.org/icl/papi/wiki/Downloading-and-Installing-PAPI.md).
    
    Diagram of a typical `PAPI` installation when building from source
    ```bash
    └── install_folder          # Installation directory of PAPI, AKA what <PAPI_DIR> should be set to
        ├── include             # Folder containing the public header
        │   └──  papi.h    
        ├── lib                 # Folder containing the built static/shared libs
        │   ├── libpapi.a
        │   ├── libpapi.so
        │   └── ....
        └── ...
    ```
    
    More info on how `find_path` searches can be found [here](https://cmake.org/cmake/help/latest/command/find_path.html)

### Building Stopwatch
Running
```shell
cd stopwatch
cmake -Bbuild
cmake --build build
```

Will build `Stopwatch` using the folder `build` as the build folder with the default build configurations which are:
- Build type is debug
- Default install prefix
- Build tests
- Do not build `C` or `Fortran` example programs

The build configurations can be changed by passing in extra flags when first running `cmake` to generate the build system
- Changing build type: `-DCMAKE_BUILD_TYPE=<type>` i.e, `Release`
- Changing install prefix: `-DCMAKE_INSTALL_PREFIX=<path>`, where `<path>` is where `Stopwatch` should be installed
- Do not build tests: `-DBUILD_TESTING=OFF`
- Build `C` examples: `-DBUILD_C_EXAMPLES=ON`
- Build `Fortran` examples: `-DBUILD_FORTRAN_EXAMPLES=ON`

### Installing Stopwatch
Running
```shell
sudo cmake --build build -- install
```
will install in the path specified by `CMAKE_INSTALL_PREFIX`

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
   measured is specified via the `STOPWATCH_EVENTS` environment variable. Each event should be delimited with a comma `,`.
   The events that can be added can be initially queried via the utility `papi_avail` that PAPI provides. If the
   `STOPWATCH_EVENTS` variable is unset, the default events of `PAPI_TOT_CYC` and `PAPI_TOT_INS` are used.
2. wrap the function(s) to measure with the calls to `stopwatch_record_start_measurements` and 
   `stopwatch_record_end_measurements`. For `stopwatch_record_start_measurements` the first argument is the unique ID
   for the routine that is to be measured. It is up to the user to ensure that this ID does not collide with another ID
   of another routine that is measured. Note that ID `0` is reserved for the `main` function. The second argument is the
   string representation of the routine name. The third argument is the ID of the caller of the current routine. Note
   that for routines that are called by the main function, the caller ID would be `0`. For 
   `stopwatch_record_end_measurements` the argument is the ID of the routine to complete the measurement for.
3. call `stopwatch_destroy` to clean up the resources used

### C Fortran Mappings
For `Fortran` usage, append the letter `F` to the start of each routine name to get the appropriate routine.


| C Function | Fortran Function / Subroutine |
| ---------- | --------------------------- |
| `stopwatch_init` | `Fstopwatch_init` |
| `stopwatch_destroy` | `Fstopwatch_destroy` |
| `stopwatch_record_start_measurements` | `Fstopwatch_record_start_measurements` |
| `stopwatch_record_end_measurements` | `Fstopwatch_record_end_measurements` |
| `stopwatch_print_measurement_results` | `Fstopwatch_print_measurement_results` |
| `stopwatch_print_result_table` | `Fstopwatch_print_result_table` |
| `stopwatch_result_to_csv` | `Fstopwatch_result_to_csv` |

Note that for the `C` routines that take a `char*` their equivalent `Fortran` routines must pass in an array of
characters where the last character is a `c_null_char` from the module `iso_c_binding` as `C` strings are null
terminated

The `C` `enum StopwatchStatus` values are defined as parameters in the `Fortran` equivalent.

### Error Codes
Some functions will return `enum StopwatchStatus` indicating the status of the function execution. List of possible
status codes and their respective meanings
- `STOPWATCH_OK` : Function executed successfully.
- `STOPWATCH_TOO_MANY_EVENTS` : Function executed unsuccessfully. Too many events were selected to be added.
- `STOPWATCH_INVALID_EVENT`: Function executed unsuccessfully. Event given is not a valid event.
- `STOPWATCH_INVALID_EVENT_COMB` : Function executed unsuccessfully. The specific combination of events could not be
  added. Either event(s) are not supported by the hardware, or the hardware cannot simultaneously measure all the
  selected events.
- `STOPWATCH_INVALID_FILE` : Function executed unsuccessfully. Path given is not valid.
- `STOPWATCH_ERR` : Function executed unsuccessfully. Error unrelated to selected events.


### Configurations
The environment variable `STOPWATCH_EVENTS` are used to configure which events are measured. Each event should be
delimited via a comma.

Example
```shell
export STOPWATCH_EVENTS=PAPI_SP_OPS,PAPI_TOT_INS
```

If the events specified in `STOPWATCH_EVENTS` are not valid `PAPI` events or if the hardware cannot support measuring
the event combination, then `stopwatch_init` **WILL NOT** return the `enum StopwatchStatus` `STOPWATCH_OK` meaning
execution beyond `stopwatch_init` is **undefined**. Make sure to always check the return status of `stopwatch_init`.

If `STOPWATCH_EVENTS` is not set, the default events used are `PAPI_TOT_CYC` and `PAPI_TOT_INS`

##### Example
Example of measuring the performance of a loop of matrix multiplication where the number of cycles stalled waiting for
resources, and the number of L1 cache misses are the selected events:
```c
#include <stopwatch/stopwatch.h>
int main() {
  stopwatch_init(); // Initialize stopwatch

  int N = 500; // Size of matrix
  int itercount = 10; // Number of iterations

  float (*A)[N] = initialize_mat(N);
  float (*B)[N] = initialize_mat(N);
  float (*C)[N] = initialize_mat(N);

  // Matrix multiply loop
  // Called by main routine so the caller ID argument is 0
  stopwatch_record_start_measurements(1, "total-loop", 0); // Record start time of the entire loop
  for (int iter = 0; iter < itercount; iter++) {
    memset(C, 0, sizeof(float) * N * N); // clear C array
    // Called by routine with ID zero so the caller ID argument is 1
    stopwatch_record_start_measurements(2, "single-cycle", 1); // read start time of a single cycle
    mat_mul(N, A, B, C); // Perform the multiplication C = A * B
    stopwatch_record_end_measurements(2); // read end time of a single cycle
  }
  stopwatch_record_end_measurements(1); // Record end measurements of the entire loop
  stopwatch_print_result_table(); // Prints the results in a table format
  stopwatch_destroy(); // Clean up resources used
}
```

Before running the binary, the environment variable `STOPWATCH_EVENTS` must be set. In this case it should be set to
`PAPI_RES_STL,PAPI_L1_TCM`.
```shell
export STOPWATCH_EVENTS=PAPI_RES_STL,PAPI_L1_TCM
```

The result should look similar to this:
```shell
|---------------------------------------------------------------------------------------------|
| ID | NAME             | TIMES CALLED | TOTAL REAL MICROSECONDS | PAPI_RES_STL | PAPI_L1_TCM |
|---------------------------------------------------------------------------------------------|
| 1  | total-loop       | 1            | 4455471                 | 5911716339   | 951256697   |
| 2  |     single-cycle | 10           | 4454818                 | 5910669863   | 951113873   |
|---------------------------------------------------------------------------------------------|
```

More examples can be found in the `examples` folder.
