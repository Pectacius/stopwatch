#ifndef LIBSTOPWATCH_INCLUDE_STOPWATCH_FSTOPWATCH_H_
#define LIBSTOPWATCH_INCLUDE_STOPWATCH_FSTOPWATCH_H_

module mod_stopwatch

    use, intrinsic :: iso_c_binding
    implicit none

    ! Constants
    integer(c_int), parameter :: STOPWATCH_OK = 0
    integer(c_int), parameter :: STOPWATCH_TOO_MANY_EVENTS = 1
    integer(c_int), parameter :: STOPWATCH_INVALID_EVENT = 2
    integer(c_int), parameter :: STOPWATCH_INVALID_EVENT_COMB = 3
    integer(c_int), parameter :: STOPWATCH_ERR = 4

    integer(c_int), parameter :: STOPWATCH_MAX_EVENTS = 10
    integer(c_int), parameter :: NULL_TERM_MAX_ROUTINE_NAME_LEN = 16

    ! Structure for holding the measurements for a specific entry
    type, bind(c) :: StopwatchMeasurementResult
        integer(c_long_long) total_real_cyc                             ! This technically should never be negative
        integer(c_long_long) total_real_usec                            ! This technically should never be negative
        integer(c_long_long) total_event_values(STOPWATCH_MAX_EVENTS)   ! This technically should never be negative
        integer(c_long_long) total_tiems_called                         ! This technically should never be negative
        character(c_char) routine_name(NULL_TERM_MAX_ROUTINE_NAME_LEN)  ! This technically should never be negative
        integer(c_size_t) stack_depth
        integer(c_size_t) num_of_events
        integer(c_int) event_names(STOPWATCH_MAX_EVENTS)
    end type StopwatchMeasurementResult

    interface
        integer(c_int) function Fstopwatch_init() bind(c, name = 'stopwatch_init')
            import :: c_int
        end function Fstopwatch_init

        subroutine Fstopwatch_destroy() bind(c, name = 'stopwatch_destroy')
        end subroutine Fstopwatch_destroy

        integer(c_int) function Fstopwatch_record_start_measurements(routine_call_num, function_name, stack_depth) &
                       bind(c, name = 'stopwatch_record_start_measurements')
            ! Note that the c_null_char must be included at the end of the value of function_name
            import :: c_char, c_int, c_size_t
            integer(c_size_t), value, intent(in) :: routine_call_num
            character(c_char), intent(in) :: function_name
            integer(c_size_t), value, intent(in) :: stack_depth

        end function Fstopwatch_record_start_measurements

        integer(c_int) function Fstopwatch_record_end_measurements(routine_call_num) &
                       bind(c, name= 'stopwatch_record_end_measurements')
            import :: c_int, c_size_t
            integer(c_size_t), value, intent(in) :: routine_call_num

        end function Fstopwatch_record_end_measurements

        integer(c_int) function Fstopwatch_get_measurement_results(routine_call_num, result) &
                bind(c, name = 'stopwatch_get_measurement_results')
            import :: c_int, c_size_t, StopwatchMeasurementResult
            integer(c_size_t), value, intent(in) :: routine_call_num
            type(StopwatchMeasurementResult), intent(out) :: result

        end function Fstopwatch_get_measurement_results

        subroutine Fstopwatch_print_measurement_results(result) bind(c, name = 'stopwatch_print_measurement_results')
            import :: StopwatchMeasurementResult
            type(StopwatchMeasurementResult), intent(in) :: result
        end subroutine Fstopwatch_print_measurement_results

        subroutine Fstopwatch_print_result_table() bind(c, name = 'stopwatch_print_result_table')

        end subroutine Fstopwatch_print_result_table
    end interface

end module mod_stopwatch

#endif