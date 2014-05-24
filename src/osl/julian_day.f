	! From http://www.lanl.gov/Caesar/node491.html#Julian_Day_Code
	!    (public domain, as US government code)
	function Julian_Day (Year, Month, Day, Calendar, Debug)

	! Use association information.

	use Caesar_Numbers_Module, only: four 

	! Input variables.

	type(integer), intent(in) :: Year                   ! Input year.
	type(integer), intent(in) :: Month                  ! Input month.
	type(integer), intent(in) :: Day                    ! Input day.
	type(character,*), intent(in), optional :: Calendar ! Julian or 
		                                        	  ! Gregorian calendar.
	type(logical), intent(in), optional :: Debug        ! Debug toggle. 

	! Output variables.

	type(integer) :: Julian_Day ! Julian Day.

	! Internal variables.

	type(character,9) :: A_Calendar       ! Actual calendar.
	type(logical) :: A_Debug              ! Actual debug toggle. 
	type(integer) :: Julian_Day_Constant  ! Shift for new zero date.
	type(integer) :: Julian_Year          ! Julian Year (includes zero).
	type(integer) :: Julian_Month         ! Julian Month (4-15).
	type(integer) :: Shifted_Julian_Year  ! Julian Year + 8000.

	!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	! Set actual debug toggle.

	if (PRESENT(Debug)) then
		A_Debug = Debug
	else
		A_Debug = .true.
	end if

	! Verify requirements.

	if (A_Debug) then
		VERIFY(Valid_State(Year),4)         ! Year is valid.
		VERIFY(Valid_State(Month),4)        ! Month is valid.
		VERIFY(Valid_State(Day),4)          ! Day is valid.
		VERIFY(Year/=0,2)                   ! Year is non-zero.
		VERIFY(Month.InInterval.(/1,12/),2) ! Month is between 1 and 12.
		VERIFY(Day.InInterval.(/1,31/),2)   ! Day is between 1 and 31.
		! Year is after the first Julian day occurred (1 January 4713 BC).
		VERIFY(Year >= -4713,4)
	end if

	! More specific requirements on Day (for higher verification levels).

	if (A_Debug) then
		if (Month==4 .or. Month==6 .or. Month==9 .or. Month==11) then
		  ! Day is between 1 and 30 for April, June, September and November.
		  VERIFY(Day.InInterval.(/1,30/),6)
		else if (Month==2) then
		  ! Day is between 1 and 29 for February.
		  VERIFY(Day.InInterval.(/1,29/),6)
		end if
	end if

	! Set calendar to use.

	if (PRESENT(Calendar)) then
		A_Calendar = Calendar
	else
		A_Calendar = 'Gregorian'
	end if

	! Further requirements that are dependent on the calendar.

	if (A_Debug) then
		if (A_Calendar == 'Gregorian') then
		  ! Warn if year is before Gregorian dates were adopted in the
		  ! first countries in Europe (4 October 1582 CE).
		  IF_NOT_UNIT_TEST WARN_IF(Year < 1582,6)
		  ! Warn if year is before Gregorian dates were adopted in
		  ! England and the American Colonies (14 September 1752 CE).
		  IF_NOT_UNIT_TEST WARN_IF(Year < 1752,8)
		else if (A_Calendar /= 'Julian') then
		  ! Only Julian and Gregorian calendars allowed.
		  VERIFY(.false.,0)
		end if
	end if

	! Convert to Julian (CE) Year, which includes a zero year:
	!
	!                           BC  AD
	!     Year:     -4  -3  -2  -1  1  2  3  4
	! Julian Year:  -3  -2  -1   0  1  2  3  4

	if (Year > 0) then
		Julian_Year = Year
	else
		Julian_Year = Year + 1
	end if

	! Convert to Julian Month for ease in calculating month days and
	! dealing with February.
	!
	!     Month:      1   2  3  4  5  6  7  8   9  10  11  12
	! Julian Month:  14  15  4  5  6  7  8  9  10  11  12  13
	!
	! For consistency, modify Julian_Year to start with March.

	if (Month < 3) then
		Julian_Month = Month + 13
		Julian_Year  = Julian_Year - 1
	else
		Julian_Month = Month + 1
	end if

	! Calculate Julian_Year shifted by 8000 years to avoid problems 
	! with leap years in negative years.

	Shifted_Julian_Year = Julian_Year + 8000

	! Adjustment constant -- this is the number of days from the start
	! of the Julian Day numbering system, 1 January 4713 BC, to the new 
	! zero date which is used for calculations. Due to the month shifting,
	! the new zero date is 30 October 2 BC.

	Julian_Day_Constant = - ( &
		  + 365*(-4713) &                          ! 365 days/year.
		  + int(30.6001*14) &                      ! Trick to get days/month.
		  + int((-4713+8000)/4) - 2000 &           ! Leap days.
		  + 1 &                                    ! Day of the month.
		  )

	! Calculate Julian Day.

	if (A_Calendar == 'Julian') then

		Julian_Day = Julian_Day_Constant &          ! Adjustment constant.
		  + 365*(Julian_Year) &                     ! 365 days/year.
		  + int(30.6001*Julian_Month) &             ! Trick to get days/month.
		  + int(Shifted_Julian_Year/4) - 2000 &     ! Leap days.
		  + Day                                     ! Day of the month.

	else if (A_Calendar == 'Gregorian') then

		Julian_Day = Julian_Day_Constant &          ! Adjustment constant.
		  + 365*Julian_Year &                       ! 365 days/year.
		  + int(30.6001*Julian_Month) &             ! Trick to get days/month.
		  + int(Shifted_Julian_Year/4)   - 2000 &   ! Leap days.
		  - int(Shifted_Julian_Year/100) + 80 &     ! Gregorian leap 
		  + int(Shifted_Julian_Year/400) - 20 &     !   day adjustment.
		  + 2 &                                     ! Difference between 
		                                	    !   Gregorian and Julian
		                                	    !   calendars at 0 CE.
		  + Day                                     ! Day of the month.

	end if

	! Verify guarantees.

	if (A_Debug) then
		VERIFY(Valid_State(Julian_Day),4)  ! Julian_Day is valid.
		VERIFY(Julian_Day >= 0,4)          ! Julian_Day is non-negative.
	end if

	return
	end function Julian_Day
