#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stopwatch/stopwatch.h"
#include "str_table.h"
#include <papi.h>

#define STOPWATCH_INVALID_EVENT 1
#define INDENT_SPACING 4
#define STOPWATCH_NUM_TIMERS 2
#define STOPWATCH_MAX_FUNCTION_CALLS 500

// Structure used to hold readings for the measurement clock.
struct MeasurementReadings {
  char routine_name[NULL_TERM_MAX_ROUTINE_NAME_LEN];
  long long total_times_called;
  unsigned int stack_depth;

  long long total_events_measurements[STOPWATCH_MAX_EVENTS];
  long long start_events_measurements[STOPWATCH_MAX_EVENTS];

  long long total_timers_measurements[STOPWATCH_NUM_TIMERS]; // First index real cycles, second index real microseconds
  long long start_timers_measurements[STOPWATCH_NUM_TIMERS];
};

// Flag to signal initialization
static bool initialized_stopwatch = false;

// Each entry corresponds to a separate entry measurement
static struct MeasurementReadings readings[STOPWATCH_MAX_FUNCTION_CALLS];

// Holds each measurement event. Not all indices will be simultaneously used and hence the variable
// `num_registered_events`acts as a separator between the indices that represent actual registered events and garbage
// values.
static int events[STOPWATCH_MAX_EVENTS];

// Holds the intermediate results from PAPI. Mainly used as an intermediate to accumulate measurements. PAPI itself does
// have an accumulation feature but it resets the timers which is undesirable when it comes to nesting measurements.
static long long tmp_event_results[STOPWATCH_MAX_EVENTS];

// Number of events that are currently stored in the `events` variable.
static unsigned int num_registered_events = 0;

// Event set used by
static int event_set = PAPI_NULL;

// =====================================================================================================================
// Private helper functions definitions
// =====================================================================================================================
static int add_events(const enum StopwatchEvents events_to_add[], unsigned int num_of_events);

static unsigned int find_num_entries();

static int map_stopwatch_to_papi(enum StopwatchEvents stopwatch_event);

// =====================================================================================================================
// Public interface functions implementations
// =====================================================================================================================

// Initializes PAPI library. Should only be called once.
// Will return STOPWATCH_OK if successful
// Will return STOPWATCH_ERR if fail
int init_stopwatch(const enum StopwatchEvents events_to_add[], unsigned int num_of_events) {
  // Check if stopwatch is not already initialized
  if (!initialized_stopwatch) {
    // Reset the function names and times called to default values values
    for (unsigned int idx = 0; idx < STOPWATCH_MAX_FUNCTION_CALLS; idx++) {
      readings[idx].total_times_called = 0;
      memset(readings[idx].total_events_measurements, 0, sizeof(readings[idx].total_events_measurements));
      memset(readings[idx].total_timers_measurements, 0, sizeof(readings[idx].total_events_measurements));
      memset(readings[idx].routine_name, 0, sizeof(readings[idx].routine_name));
    }

    // Reset number of registered events
    num_registered_events = 0;

    // Initialize PAPI
    int ret_val = PAPI_library_init(PAPI_VER_CURRENT);
    if (ret_val != PAPI_VER_CURRENT) {
      destroy_stopwatch();
      return STOPWATCH_ERR;
    }

    ret_val = PAPI_create_eventset(&event_set);
    if (ret_val != PAPI_OK) {
      destroy_stopwatch();
      return STOPWATCH_ERR;
    }

    // Attempt to add each event to the event set. If not all can be added STOPWATCH_ERR will be returned
    ret_val = add_events(events_to_add, num_of_events);
    if (ret_val != STOPWATCH_OK) {
      destroy_stopwatch();
      return STOPWATCH_ERR;
    }

    // Start the monotonic clock
    ret_val = PAPI_start(event_set);
    if (ret_val != PAPI_OK) {
      destroy_stopwatch();
      return STOPWATCH_ERR;
    }

    initialized_stopwatch = true;

    return STOPWATCH_OK;
  }
  return STOPWATCH_ERR;
}

// CLean up resources used by PAPI and resets measurements regardless of the stage of execution. Should clean up the
// necessary elements on a failed or successfully initialization
void destroy_stopwatch() {
  // Regardless if the event set is running or not, calling stop should not produce side effects to state hence return
  // value is not checked.
  PAPI_stop(event_set, NULL);

  // Will remove all events from event set if there are any and should do nothing if there is not. Return value is not
  // checked as its return value does not effect execution
  PAPI_cleanup_eventset(event_set);

  PAPI_destroy_eventset(&event_set);

  PAPI_shutdown();

  initialized_stopwatch = false;
}

int record_start_measurements(int routine_call_num, const char *function_name, unsigned int stack_depth) {
  int PAPI_ret = PAPI_read(event_set, readings[routine_call_num].start_events_measurements);
  if (PAPI_ret != PAPI_OK) {
    return STOPWATCH_ERR;
  }
  readings[routine_call_num].start_timers_measurements[0] = PAPI_get_real_cyc();
  readings[routine_call_num].start_timers_measurements[1] = PAPI_get_real_usec();

  // Only log these values the first time it is called as there is a possibility of nesting.
  if (readings[routine_call_num].total_times_called == 0) {
    readings[routine_call_num].stack_depth = stack_depth;

    // Copy in the routine name and ensure the string is null terminated
    strncpy(readings[routine_call_num].routine_name, function_name, NULL_TERM_MAX_ROUTINE_NAME_LEN);
    readings[routine_call_num].routine_name[NULL_TERM_MAX_ROUTINE_NAME_LEN - 1] = '\0';
  }

  return STOPWATCH_OK;
}

int record_end_measurements(int routine_call_num) {
  int PAPI_ret = PAPI_read(event_set, tmp_event_results);
  if (PAPI_ret != PAPI_OK) {
    return STOPWATCH_ERR;
  }

  readings[routine_call_num].total_times_called++;

  // Accumulate the timer results
  readings[routine_call_num].total_timers_measurements[0] +=
      (PAPI_get_real_cyc() - readings[routine_call_num].start_timers_measurements[0]);
  readings[routine_call_num].total_timers_measurements[1] +=
      (PAPI_get_real_usec() - readings[routine_call_num].start_timers_measurements[1]);

  // Accumulate the event(s) results
  for (unsigned int idx = 0; idx < num_registered_events; idx++) {
    readings[routine_call_num].total_events_measurements[idx] +=
        (tmp_event_results[idx] - readings[routine_call_num].start_events_measurements[idx]);
  }

  return STOPWATCH_OK;
}

void print_measurement_results(struct MeasurementResult *result) {
  printf("Procedure name: %s\n", result->routine_name);
  printf("Total times run: %lld\n", result->total_times_called);
  printf("Total real cycles elapsed: %lld\n", result->total_real_cyc);
  printf("Total real microseconds elapsed: %lld\n", result->total_real_usec);
  for(unsigned int idx = 0; idx < result->num_of_events; idx++) {
    char event_code_string[PAPI_MAX_STR_LEN];
    PAPI_event_code_to_name(result->event_names[idx], event_code_string);
    printf("%s: %lld\n", event_code_string, result->total_event_values[idx]);
  }
}

int get_measurement_results(unsigned int routine_call_num, struct MeasurementResult *result) {
  if (routine_call_num >= STOPWATCH_MAX_FUNCTION_CALLS) {
    return STOPWATCH_ERR;
  }

  result->total_real_cyc = readings[routine_call_num].total_timers_measurements[0];
  result->total_real_usec = readings[routine_call_num].total_timers_measurements[1];
  result->num_of_events = num_registered_events;
  for(unsigned int idx = 0; idx < num_registered_events; idx++) {
    result->total_event_values[idx] = readings[routine_call_num].total_events_measurements[idx];
    result->event_names[idx] = events[idx];
  }

  result->total_times_called = readings[routine_call_num].total_times_called;
  result->stack_depth = readings[routine_call_num].stack_depth;

  // String already null terminated
  strncpy(result->routine_name, readings[routine_call_num].routine_name, NULL_TERM_MAX_ROUTINE_NAME_LEN);

  return STOPWATCH_OK;
}

// =====================================================================================================================
// Print results into a formatted table
// =====================================================================================================================

void print_result_table() {
  // Generate table
  // Additional 3 for id, name, times called
  const unsigned int columns = num_registered_events + STOPWATCH_NUM_TIMERS + 3;
  const unsigned int rows = find_num_entries() + 1; // Extra row for header

  struct StringTable *table = create_table(columns, rows, true, INDENT_SPACING);

  // Default table entries
  add_entry_str(table, "ID", (struct StringTableCellPos) {0, 0});
  add_entry_str(table, "NAME", (struct StringTableCellPos) {0, 1});
  add_entry_str(table, "TIMES CALLED", (struct StringTableCellPos) {0, 2});
  add_entry_str(table, "TOTAL REAL CYCLES", (struct StringTableCellPos) {0, 3});
  add_entry_str(table, "TOTAL REAL MICROSECONDS", (struct StringTableCellPos) {0, 4});

  // Entries for each event
  for (unsigned int entry_idx = 0; entry_idx < num_registered_events; entry_idx++) {
    const unsigned int effective_col_idx = columns - num_registered_events + entry_idx;

    char event_code_string[PAPI_MAX_STR_LEN];
    PAPI_event_code_to_name(events[entry_idx], event_code_string);
    add_entry_str(table, event_code_string, (struct StringTableCellPos) {0, effective_col_idx});
  }

  unsigned int row_cursor = 1;
  for (unsigned int idx = 0; idx < STOPWATCH_MAX_FUNCTION_CALLS; idx++) {
    if (readings[idx].total_times_called == 0) {
      continue;
    }
    // Default table measurement values
    add_entry_lld(table, idx, (struct StringTableCellPos) {row_cursor, 0});

    add_entry_str(table, readings[idx].routine_name, (struct StringTableCellPos) {row_cursor, 1});
    set_indent_lvl(table, readings[idx].stack_depth, (struct StringTableCellPos) {row_cursor, 1});

    add_entry_lld(table, readings[idx].total_times_called, (struct StringTableCellPos) {row_cursor, 2});
    add_entry_lld(table, readings[idx].total_timers_measurements[0], (struct StringTableCellPos) {row_cursor, 3});
    add_entry_lld(table, readings[idx].total_timers_measurements[1], (struct StringTableCellPos) {row_cursor, 4});

    // Event specific
    for (unsigned int entry_idx = 0; entry_idx < num_registered_events; entry_idx++) {
      const unsigned int effective_col_idx = columns - num_registered_events + entry_idx;
      add_entry_lld(table,
                    readings[idx].total_events_measurements[entry_idx],
                    (struct StringTableCellPos) {row_cursor, effective_col_idx});
    }
    row_cursor++;
  }

  // Format print table
  char *table_str = make_table_str(table);
  printf("%s\n", table_str);
  free(table_str);
  destroy_table(table);
}

// =====================================================================================================================
// Private helper functions implementation
// =====================================================================================================================


static int add_events(const enum StopwatchEvents events_to_add[], unsigned int num_of_events) {
  if (num_of_events > STOPWATCH_MAX_EVENTS) {
    return STOPWATCH_ERR;
  }
  for (unsigned int idx = 0; idx < num_of_events; idx++) {
    const int papi_event_code = map_stopwatch_to_papi(events_to_add[idx]);
    events[idx] = papi_event_code;
    const int papi_ret_val = PAPI_add_event(event_set, papi_event_code);
    if (papi_ret_val != PAPI_OK) {
      return STOPWATCH_ERR;
    }
    num_registered_events++;
  }
  return STOPWATCH_OK;
}

static int map_stopwatch_to_papi(enum StopwatchEvents stopwatch_event) {
  // An array using indices as hashing may produce a more cleaner solution
  switch (stopwatch_event) {
    case L1_CACHE_MISS:return PAPI_L1_TCM;
    case L2_CACHE_MISS:return PAPI_L2_TCM;
    case L3_CACHE_MISS:return PAPI_L3_TCM;
    case BRANCH_MISPREDICT:return PAPI_BR_MSP;
    case BRANCH_PREDICT:return PAPI_BR_PRC;
    case CYCLES_STALLED_RESOURCE:return PAPI_RES_STL;
    case TOTAL_CYCLES:return PAPI_TOT_CYC;
    case SP_FLOAT_OPS:return PAPI_SP_OPS;
    case DP_FLOAT_OPS:return PAPI_DP_OPS;
      // Execution should never hit this branch
    default:return STOPWATCH_INVALID_EVENT;
  }
}

static unsigned int find_num_entries() {
  unsigned int entries = 0;
  for (unsigned int idx = 0; idx < STOPWATCH_MAX_FUNCTION_CALLS; idx++) {
    if (readings[idx].total_times_called > 0) {
      entries++;
    }
  }
  return entries;
}
