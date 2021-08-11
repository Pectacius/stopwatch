#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stopwatch/stopwatch.h"
#include "str_table.h"
#include "call_tree.h"
#include <papi.h>

#define INDENT_SPACING 4
#define STOPWATCH_MAX_FUNCTION_CALLS 500  // Maximum number of measurement entries

// Structure used to hold readings for the measurement clock.
struct MeasurementReadings {
  // Name of the routine being measured
  char routine_name[NULL_TERM_MAX_ROUTINE_NAME_LEN];
  // Number of times the routine has been called
  long long total_times_called;
  // ID of the procedure that called the current measured procedure
  size_t caller_routine_id;
  // Accumulated measurements of each event. Each index corresponds to one event
  long long total_events_measurements[STOPWATCH_MAX_EVENTS];
  // Start measurements of each event. Each index corresponds to one event
  long long start_events_measurements[STOPWATCH_MAX_EVENTS];
  // Accumulated values of total real microseconds elapsed
  long long total_real_us;
  // Start values of real microseconds
  long long start_real_us;
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
static size_t num_registered_events = 0;

// Event set used by
static int event_set = PAPI_NULL;

// =====================================================================================================================
// Private helper functions definitions
// =====================================================================================================================
static enum StopwatchStatus set_events();

static enum StopwatchStatus add_event(const char *event_to_add);

static size_t find_num_entries();

static void set_header(const struct StringTable *table);

static void set_body_row(const struct StringTable *table,
                         size_t row_num,
                         size_t routine_id,
                         size_t stack_depth,
                         struct MeasurementReadings reading);

// =====================================================================================================================
// Public interface functions implementations
// =====================================================================================================================

// Initializes PAPI library. Should only be called once.
// Will return STOPWATCH_OK if successful
// Will return STOPWATCH_ERR if fail
enum StopwatchStatus stopwatch_init() {
  // Check if stopwatch is not already initialized
  if (!initialized_stopwatch) {
    // Reset the function names and times called to default values values
    for (unsigned int idx = 0; idx < STOPWATCH_MAX_FUNCTION_CALLS; idx++) {
      readings[idx].total_times_called = 0;
      memset(readings[idx].total_events_measurements, 0, sizeof(readings[idx].total_events_measurements));
      readings[idx].total_real_us = 0;
      memset(readings[idx].routine_name, 0, sizeof(readings[idx].routine_name));
    }

    // Reset number of registered events
    num_registered_events = 0;

    // Initialize PAPI
    int ret_val = PAPI_library_init(PAPI_VER_CURRENT);
    if (ret_val != PAPI_VER_CURRENT) {
      stopwatch_destroy();
      return STOPWATCH_ERR;
    }

    ret_val = PAPI_create_eventset(&event_set);
    if (ret_val != PAPI_OK) {
      stopwatch_destroy();
      return STOPWATCH_ERR;
    }

    // Attempt to add each event selected in the environment variable to the event set. If not all can be added
    // STOPWATCH_ERR will be returned
    ret_val = set_events();
    if (ret_val != STOPWATCH_OK) {
      stopwatch_destroy();
      return ret_val;
    }

    // Start the monotonic clock
    ret_val = PAPI_start(event_set);
    if (ret_val != PAPI_OK) {
      stopwatch_destroy();
      return STOPWATCH_ERR;
    }

    initialized_stopwatch = true;

    return STOPWATCH_OK;
  }
  return STOPWATCH_ERR;
}

// CLean up resources used by PAPI and resets measurements regardless of the stage of execution. Should clean up the
// necessary elements on a failed or successfully initialization
void stopwatch_destroy() {
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

enum StopwatchStatus stopwatch_record_start_measurements(size_t routine_id,
                                                         const char *function_name,
                                                         size_t caller_routine_id) {
  int PAPI_ret = PAPI_read(event_set, readings[routine_id].start_events_measurements);
  if (PAPI_ret != PAPI_OK) {
    return STOPWATCH_ERR;
  }

  readings[routine_id].start_real_us = PAPI_get_real_usec();

  // Only log these values the first time it is called as there is a possibility of nesting.
  if (readings[routine_id].total_times_called == 0) {
    readings[routine_id].caller_routine_id = caller_routine_id;

    // Copy in the routine name and ensure the string is null terminated
    strncpy(readings[routine_id].routine_name, function_name, NULL_TERM_MAX_ROUTINE_NAME_LEN);
    readings[routine_id].routine_name[NULL_TERM_MAX_ROUTINE_NAME_LEN - 1] = '\0';
  }

  return STOPWATCH_OK;
}

enum StopwatchStatus stopwatch_record_end_measurements(size_t routine_id) {
  int PAPI_ret = PAPI_read(event_set, tmp_event_results);
  if (PAPI_ret != PAPI_OK) {
    return STOPWATCH_ERR;
  }

  readings[routine_id].total_times_called++;

  // Accumulate the timer results
  readings[routine_id].total_real_us +=
      (PAPI_get_real_usec() - readings[routine_id].start_real_us);

  // Accumulate the event(s) results
  for (unsigned int idx = 0; idx < num_registered_events; idx++) {
    readings[routine_id].total_events_measurements[idx] +=
        (tmp_event_results[idx] - readings[routine_id].start_events_measurements[idx]);
  }

  return STOPWATCH_OK;
}

void stopwatch_print_measurement_results(struct StopwatchMeasurementResult *result) {
  printf("Procedure name: %s\n", result->routine_name);
  printf("Total times run: %lld\n", result->total_times_called);
  printf("Total real microseconds elapsed: %lld\n", result->total_real_usec);
  for (unsigned int idx = 0; idx < result->num_of_events; idx++) {
    char event_code_string[PAPI_MAX_STR_LEN];
    PAPI_event_code_to_name(result->event_names[idx], event_code_string);
    printf("%s: %lld\n", event_code_string, result->total_event_values[idx]);
  }
}

enum StopwatchStatus stopwatch_get_measurement_results(size_t routine_id, struct StopwatchMeasurementResult *result) {
  if (routine_id >= STOPWATCH_MAX_FUNCTION_CALLS) {
    return STOPWATCH_ERR;
  }

  result->total_real_usec = readings[routine_id].total_real_us;
  result->num_of_events = num_registered_events;
  for (unsigned int idx = 0; idx < num_registered_events; idx++) {
    result->total_event_values[idx] = readings[routine_id].total_events_measurements[idx];
    result->event_names[idx] = events[idx];
  }

  result->total_times_called = readings[routine_id].total_times_called;
  result->caller_routine_id = readings[routine_id].caller_routine_id;

  // String already null terminated
  strncpy(result->routine_name, readings[routine_id].routine_name, NULL_TERM_MAX_ROUTINE_NAME_LEN);

  return STOPWATCH_OK;
}

// =====================================================================================================================
// Print results into a formatted table
// =====================================================================================================================

void stopwatch_print_result_table() {
  // Generate table
  const size_t num_static_cols = 4; // Additional 4 for id, name, times called and total usec
  const size_t num_functions = find_num_entries();
  const size_t columns = num_registered_events + num_static_cols;
  const size_t rows = num_functions + 1; // Extra row for header

  struct StringTable *table = create_table(columns, rows, true, INDENT_SPACING);

  set_header(table);

  if (num_functions > 0) {
    struct FunctionNode *function_list = malloc(sizeof(struct FunctionNode) * num_functions);
    size_t entry_num = 0;
    for (size_t idx = 0; idx < STOPWATCH_MAX_FUNCTION_CALLS; idx++) {
      if (readings[idx].total_times_called == 0) {
        continue;
      }
      function_list[entry_num].function_id = idx;
      function_list[entry_num].caller_id = readings[idx].caller_routine_id;
      entry_num++;
    }

    struct FunctionCallNode *call_tree = function_call_node_grow_tree_from_array(function_list, num_functions);
    free(function_list);
    function_list = NULL;
    struct FunctionCallTreeDFIter *iter = create_function_call_tree_DF_iter(call_tree);
    // Since the first function call is always a call to main and we do not want to print that, we skip that entry
    function_call_tree_DF_iter_next(iter);

    size_t row_cursor = 1;
    while (function_call_tree_DF_iter_has_next(iter)) {
      const struct FunctionCallNode *next = function_call_tree_DF_iter_next(iter);
      // Subtract from stack depth as we want the stack depth relative to the call to main where main has a depth of 0
      set_body_row(table, row_cursor, next->function_id, next->stack_depth - 1, readings[next->function_id]);
      row_cursor++;
    }

    destroy_function_call_tree_DF_iter(iter);
    iter = NULL;
    destroy_function_call_node(call_tree);
    call_tree = NULL;
  }

  // Format print table
  char *table_str = make_table_str(table);
  printf("%s\n", table_str);
  free(table_str);
  table_str = NULL;
  destroy_table(table);
  table = NULL;
}

enum StopwatchStatus stopwatch_result_to_csv(const char *file_name) {
  printf("Reached\n");
  FILE *output_file = fopen(file_name, "w+");
  if (output_file == NULL) {
    return STOPWATCH_INVALID_FILE;
  }

  // Write default header values
  fprintf(output_file, "%s,%s,%s,%s,%s", "ID", "NAME", "CALLER_ID", "TIMES_CALLED", "TOTAL_REAL_MICROSECONDS");
  // Write each selected event
  for (size_t idx = 0; idx < num_registered_events; idx++) {
    char event_code_str[PAPI_MAX_STR_LEN];
    PAPI_event_code_to_name(events[idx], event_code_str);
    fprintf(output_file, ",%s", event_code_str);
  }
  // New line
  fprintf(output_file, "\n");

  // Write contents
  for (size_t entry = 0; entry < STOPWATCH_MAX_FUNCTION_CALLS; entry++) {
    if (readings[entry].total_times_called > 0) {
      fprintf(output_file,
              "%zu,%s,%zu,%lld,%lld",
              entry,
              readings[entry].routine_name,
              readings[entry].caller_routine_id,
              readings[entry].total_times_called,
              readings[entry].total_real_us);
      for(size_t idx = 0; idx < num_registered_events; idx++) {
        fprintf(output_file, ",%lld", readings[entry].total_events_measurements[idx]);
      }
      fprintf(output_file, "\n");
    }
  }
  fclose(output_file);
  return STOPWATCH_OK;
}

// =====================================================================================================================
// Private helper functions implementation
// =====================================================================================================================
static enum StopwatchStatus set_events() {
  int ret_val;
  const char *event_env_val = getenv("STOPWATCH_EVENTS");
  // For if the environment variable exists
  if (event_env_val) {
    // A copy is made as strtok_r mutates the arguments
    char *env_var_copy_elem = strdup(event_env_val); // Copy of the env var for use to parse each element
    char *delimiter = ",";
    char *save_ptr;

    for (char *token = strtok_r(env_var_copy_elem, delimiter, &save_ptr); token != NULL;
         token = strtok_r(NULL, delimiter, &save_ptr)) {
      ret_val = add_event(token);
      if (ret_val != STOPWATCH_OK) {
        break;
      }
    }
    free(env_var_copy_elem);
    env_var_copy_elem = NULL;
  } else { // For if the environment variable does not exist
    const char *default_events[] = {"PAPI_TOT_CYC", "PAPI_TOT_INS"};
    for (size_t idx = 0; idx < sizeof(default_events) / sizeof(char *); idx++) {
      ret_val = add_event(default_events[idx]);
      if (ret_val != STOPWATCH_OK) {
        break;
      }
    }
  }
  return ret_val;
}

static enum StopwatchStatus add_event(const char *event_to_add) {
  // Prevent adding more events than maximum
  if (num_registered_events >= STOPWATCH_MAX_EVENTS) {
    return STOPWATCH_TOO_MANY_EVENTS;
  }
  int event_code = PAPI_NULL;

  // Attempt to convert string to valid event code
  if (PAPI_event_name_to_code(event_to_add, &event_code) != PAPI_OK) {
    return STOPWATCH_INVALID_EVENT;
  }

  // Attempt to add event code
  if (PAPI_add_event(event_set, event_code) != PAPI_OK) {
    return STOPWATCH_INVALID_EVENT_COMB;
  }
  events[num_registered_events] = event_code;
  num_registered_events++;

  return STOPWATCH_OK;
}

static size_t find_num_entries() {
  size_t entries = 0;
  for (size_t idx = 0; idx < STOPWATCH_MAX_FUNCTION_CALLS; idx++) {
    if (readings[idx].total_times_called > 0) {
      entries++;
    }
  }
  return entries;
}

static void set_header(const struct StringTable *table) {
  // Default table header entries
  add_entry_str(table, "ID", (struct StringTableCellPos) {0, 0});
  add_entry_str(table, "NAME", (struct StringTableCellPos) {0, 1});
  add_entry_str(table, "TIMES CALLED", (struct StringTableCellPos) {0, 2});
  add_entry_str(table, "TOTAL REAL MICROSECONDS", (struct StringTableCellPos) {0, 3});

  // Header entries for each measurement event
  for (unsigned int entry_idx = 0; entry_idx < num_registered_events; entry_idx++) {
    const size_t num_columns = table->width;
    const unsigned int effective_col_idx = num_columns - num_registered_events + entry_idx;

    char event_code_string[PAPI_MAX_STR_LEN];
    PAPI_event_code_to_name(events[entry_idx], event_code_string);
    add_entry_str(table, event_code_string, (struct StringTableCellPos) {0, effective_col_idx});
  }
}

static void set_body_row(const struct StringTable *table,
                         size_t row_num,
                         size_t routine_id,
                         size_t stack_depth,
                         struct MeasurementReadings reading) {
  // Default table row measurement values
  // All routine_id should be able to fit in long long without any overflow
  add_entry_lld(table, (long long) routine_id, (struct StringTableCellPos) {row_num, 0});
  add_entry_str(table, reading.routine_name, (struct StringTableCellPos) {row_num, 1});
  set_indent_lvl(table, stack_depth, (struct StringTableCellPos) {row_num, 1});

  add_entry_lld(table, reading.total_times_called, (struct StringTableCellPos) {row_num, 2});
  add_entry_lld(table, reading.total_real_us, (struct StringTableCellPos) {row_num, 3});

  // Event specific table row measurement values
  for (size_t entry_idx = 0; entry_idx < num_registered_events; entry_idx++) {
    const size_t columns = table->width;
    const size_t effective_col_idx = columns - num_registered_events + entry_idx;
    add_entry_lld(table,
                  reading.total_events_measurements[entry_idx],
                  (struct StringTableCellPos) {row_num, effective_col_idx});
  }
}
