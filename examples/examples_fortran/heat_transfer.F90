#include "stopwatch/fstopwatch.F03"

! Computes heat distribution on a square plane (2D) where a heat source is placed on two edges
program heat_transfer
    use iso_c_binding
    use mod_stopwatch
    implicit none

    integer(kind=4), parameter :: num_time_steps = 1000
    integer(kind=4), parameter :: num_elem = 1000 ! Number of finite elements per side of the plane
    real(kind=8), dimension(num_elem, num_elem) :: plane

    integer :: ret_val
    integer :: i

    ! Initial conditions
    ! Boundary values: top elements
    plane = 0
    plane(:,1) = 10.0
    plane(1,:) = 10.0

    ! Initialize stopwatch
    ret_val = Fstopwatch_init()
    if (ret_val /= STOPWATCH_OK) then
        print *, "Error initializing stopwatch"
        stop -1
    end if

    ! Start measure time step loop
    ret_val = Fstopwatch_record_start_measurements(int(1, c_size_t), 'Time step loop' // c_null_char, int(0, c_size_t))
    if (ret_val /= STOPWATCH_OK) then
        print *, "Error recording start measurement"
        stop -1
    end if

    ! Time step loop
    do i = 1, num_time_steps
        ! Start measure stencil
        ret_val = Fstopwatch_record_start_measurements(int(2, c_size_t), 'Stencil' // c_null_char, int(1, c_size_t))
        if (ret_val /= STOPWATCH_OK) then
            print *, "Error recording start measurement"
            stop -1
        end if
        ! For simplicity, it is assumed that the temperature of each finite element to be the average of the temperatures
        ! of the elements above, below, left and right of the element
        plane(2:num_elem-1, 2:num_elem-1) = (plane(1:num_elem-2, 2:num_elem-1)  &
                                           + plane(3:num_elem, 2:num_elem-1)    &
                                           + plane(2:num_elem-1, 1:num_elem-2) &
                                           + plane(2:num_elem-1, 3:num_elem)) / 4
        ! End measure stencil
        ret_val = Fstopwatch_record_end_measurements(int(2, c_size_t))
        if (ret_val /= STOPWATCH_OK) then
            print *, "Error recording end measurement"
            stop -1
        end if
    end do
    ! End measure tim step loop
    ret_val = Fstopwatch_record_end_measurements(int(1, c_size_t))
    if (ret_val /= STOPWATCH_OK) then
        print *, "Error recording end measurement"
        stop -1
    end if

!    do i = 1, num_elem
!       print '(10f10.2)', plane(i,:) !  This somewhat needs to be changes as number of entries increase....
!    end do

    ! Print measurement results
    call Fstopwatch_print_result_table()

    ! Clean up resources
    call Fstopwatch_destroy()

end program heat_transfer