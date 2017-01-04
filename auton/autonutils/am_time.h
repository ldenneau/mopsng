/*
 * File: am_time.h
 * Author: A.Moore, Pat Gunn, others
 * Created: Thu Feb 13 14:50:39 EST 2003
 * Portions gathered from all over the auton sources
 * Description: Auton utilities that deal with handling time-related stuff. 
 *   benchmarks, time structures, sleep functions, and all that fun. It all
 *   belongs here.
 *   ALSO, SEE EXTRA/DATES.H
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
   
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * */

#ifndef AM_TIME_H
#define AM_TIME_H

#include "standard.h"
#include "ambs.h"
#include "genarray.h"
#include "amiv.h"
#include "am_string_array.h"

	/* We have nice, constant-time functions to manipulate DATEs */
	/* Note that all the functions that manipulate "date"s used to
	 * live in extra/dates.h, and were written by Andrew */

/* Please see later on in the file for operations on dates */
typedef struct date
{
  int year;
  int month;
  int day;
} date;

typedef struct dates
{
  generic_array *genarr;
} dates;

/* Just returns the current Unix time (seconds since the Epoch).
 * Might return something different on Windows. */
int global_time(void);

/* These return the current date or time in a nice,
 * compact form suitable for printing.
 * They use a global, and return a pointer to it, so use them accordingly. */
char *curr_date(void);
char *curr_time(void);

struct timeval rusage_time(void); /* use getrusage to fill in the time struct.
                                     Unix only.  Does nothing on windows. */

double timeval_to_double(struct timeval); /* Converts a timeval into a double, units are usecs */
void timeval_copy(struct timeval* targ, struct timeval* src);


/* These set and return differences between timers with usec scale (actual
 * granularity depends on OS/hardware). This is wall-clock. */
void start_wc_timer(void);
double stop_wc_timer(void);

#ifdef WIN32
#define ITIMER_REAL     0
#define ITIMER_VIRTUAL  1
#define ITIMER_PROF     2
#endif /* WINDOWS */

  /* This is a more advanced set of timers that let you measure different
   * types of time:
   * ITIMER_REAL - Measures wall-clock time. 
   * ITIMER_VIRTUAL - Measures time spent within the process
   * ITIMER_PROF - ITIMER_VIRTUAL, plus time the kernel spends on behalf
   *   of the process
   * Note that it *IS* ok to have more than one of these timers running at
   *   once, *PROVIDED* they're different types.
   *
   * CAVEATS of using these functions:
   *   1) It uses signals. It doesn't restore the old ones. This is
   *      intentional -- it'll make errors happen more often when you're
   *      using signals too. Note that ITIMER_REAL uses a particularly
   *      bad signal -- SIGALRM, which on most Unices is used by sleep().
   *   2) It's not portable. It will not work on Windows, and there are
   *      probably some unices being sold today that lack the interface
   *   3) You need to call prepare_timers() one time before you use
   *      either of the main functions. */

void prepare_timers(void); /* Please call this before using the 2 itimer fns */
void start_itimer(int type); /* Pass something listed above like ITIMER_PROF */
double check_itimer(int type); /* Return elapsed time. */
double stop_itimer(int type); /* Don't try to stop a timer that doesn't exist */
double itimer_delta(int timer_type);
double itimer_mintime(int timer_type, double eps);
double itimer_benchmark(int timer_type, double eps, void (*testfn)(void));
double itimer_benchmark_with_data(int timer_type, double eps,
				  void (*testfn)(void *), void *data);

  /* these cause the program to sleep for an appropriate
   * number of seconds or usecs */
void am_sleep(int secs);
void am_usleep(int microsecs);

  /* Win32 lacks the gettimeofday() call, so we have a Win32 implementation */
#ifdef PC_PLATFORM
struct timezone { int tz_minuteswest; int tz_dsttime;}; /* Not used. */
int gettimeofday(struct timeval* tp, struct timezone* UNUSED);
#endif /* PC_PLATFORM */

/* Returns the month (1 <= month <= 12) represented by the string.
   Just looks at the first three chars of string, thus

     "decadent" will return 12 (= december)

     Returns -1 if string matches no month */
int name_to_month(char *s);

bool must_be_year(char *s); /* Badly named */

/* Returns TRUE if and only "year" is a leapyear */
bool is_leap_year(int year);

/* Input: 1 <= month <= 12.
   Output: Number of days in month in given year. */
int month_to_length(int year,int month);

/* Input: 1 <= month <= 12.
   Output: Pointer to static string (should not be freed) 
           representing month. */
char *month_to_name(int month);

/* day_of_week is a number such that 0 <= day_of_week <= 6 with

   0 represents Monday
   1 represents Tuesday
      :
   5 represents Saturday
   6 represents Sunday

   Input: day_of_week
   Output: Static pointer to string that names day_of_week
*/
char *day_of_week_to_name(int day_of_week);

/* Returns 0 for monday, 1 for tuesday, ... 5 for saturday, 6 for sunday */
int daycode_to_day_of_week(int daycode);

int month_length_non_leapyear(int month);

int leap_years_since_1800(int year);

/* MAKES and returns a string in "MMM-DD-YYYY" where MMM is the three-letter
   text name of the month */
char *mk_string_from_daycode(int daycode);

string_array *mk_string_array_from_daycodes(ivec *daycodes);

/* If parse error... returns -1 and MAKES and stores message in *r_errmess */
/* If OK sets *r_errmess to NULL */
int string_to_daycode(char *string,char **r_errmess);

/* If parse error calls my_error() */
int string_to_daycode_simple(char *string);

/* If parse error... returns -1 and MAKES and stores message in *r_errmess */
/* If OK sets *r_errmess to NULL */
int daycode_from_args(char *key,int argc,char *argv[],int default_daycode,
		      char **r_errmess);

/* If parse error... does a my_error and explains problem */
int daycode_from_args_simple(char *key,int argc,char *argv[],
			     int default_daycode);

/* ****************** functions on DATE objects **************** */
date *mk_date(int year,int month,int day); /* Many people the world over
					    wish that RL dating were this easy */
date *mk_date_knowing_year(char *s,char *year_string,char *month_string,
			   char *day_string,char **r_errmess);
void free_date(date *x);
void fprintf_date(FILE *s,char *m1,date *x,char *m2);
void pdate(date *x);
date *mk_copy_date(date *old);

/* Accessor */
int date_year(date *d);
/* Accessor */
int date_month(date *d);
/* Accessor */
int date_day(date *d);
/* a daycode is an integer representing a given date.

   Tomorrow is always represented by a daycode of one greater than today,
   thus given two dates, the number of days between them equals the
   difference of their daycodes. */
int date_to_daycode(date *d);

/* Returns number of days from a until b.
   0 if same date
   > 0 if b later than a
   < 0 if b earlier than a
*/
int days_between(date *a,date *b);

/* MAKES and returns a string in "MMM-DD-YYYY" where MMM is the three-letter
   text name of the month */
char *mk_string_from_date(date *d);


/* Parses a date. A date may be 1,2 or 3 components separated by 
   "ignore characters" where "ignore characters" are :-/ and space

   Figures out which of the components is the year by looking for 
   something between 1800 and max_year

   Then assume the earlier of the two remaining components is the month
   (thus failing to parse UK format dates).

   And the later of the two components is day of month.

   Can handle month being given by name (in any case) using the same
   rules as name_to_month() above.

   It DOES handle sates of the form DD-MMM-YYYY successfully, even those
   are UK style, but only if MMM is text format.

   If there's a parse error then this function returns NULL, and MAKES
   and RETURNS an error message (which must later be free_string()'d)
   in *r_errmess.

   If happy, returns date d, and sets *r_errmess to NULL.
*/
date *mk_date_from_string(char *s,char **r_errmess);

/* Returns 0 for monday, 1 for tuesday, ... 5 for saturday, 6 for sunday */
int date_to_day_of_week(date *d);

/* Same as mk_date_from_string above, except my_error()'s if parse error */
date *mk_date_from_string_simple(char *s);

/* Makes the date d such that date_to_daycode(d) = daycode */
date *mk_date_from_daycode(int daycode);


/* Change d so that it is "num_days" later.
   If num_days is -ve, moves to appropriate earlier date. */
void increment_date(date *d,int num_days);

int days_since_jan1(date *d);

/* End of DATE functions */

/* DATES functions */
dates *mk_empty_dates(void);

void add_to_dates(dates *array,date *element);

void add_pointer_to_dates(dates *array,date *element);

int dates_size(dates *array);

date *dates_ref(dates *array,int idx);
  
void fprintf_dates(FILE *s,char *m1,dates *array,char *m2);

void pdates(dates *array);

void free_dates(dates *array);

dates *mk_copy_dates(dates *array);

void dates_set(dates *array,int idx,date *d);

void dates_remove(dates *array,int idx);

/* End of DATES functions*/


/* ------------------------------------------------------------------ */
/* --- Scientific/Astronomical Dates -------------------------------- */
/* ------------------------------------------------------------------ */
/* Currently there is (limited) support for:                          */
/* 1) Julian Date (JD) - the number of (fractional) days since noon   */
/*    January 1, 2413 BCE in the Julian Calendar.                     */
/* 2) Modified Julian Date (MJD) - the total number of (fractional)   */
/*    days since midnight November 17, 1858.  Also the JD - 2400000.5.*/  
/* ------------------------------------------------------------------ */

/* Converts the date and time (given in fractional days) 
   to the Modified Julian Date (MJD). The MJD is the total 
   number of (fractional) days since midnight 
   November 17, 1858.  It is also the Julian date - 2400000.5.  
   This function currently only handles dates after 1752.

   Adapted from: "Astronomy on the Personal Computer"
                 by Montenbruck and Pfleger */
double date_time_to_MJD(int year, int month, int day, double frac_days);

double date_HMS_to_MJD(int year, int month, int day, int hrs, int min, double sec);

double frac_date_to_MJD(int year, int month, double day);


/* Converts a date to the Modified Julian Date */
double date_to_MJD(date *d);

/* This function converts the MJD to year, month, and (fractional) day 
   Adapted from: "Astronomy on the Personal Computer"
                  by Montenbruck and Pfleger */
void MJD_to_date(double mjd, int* year, int* month, double* day);

date* mk_MJD_date(double mjd);


/* Converts the date and time (given in fractional days) 
   to the Julian Date (JD).  The JD is the number of
   (fractional) days since noon January 1, 2413 BCE in the 
   Julian Calendar.  The function currently only handles the
   Gregorian Calendar (after 1752).

   Adapted from: "Astronomy on the Personal Computer"
   by Montenbruck and Pfleger */
double date_time_to_JD(int year, int month, int day, double frac_days);

/* Converts a date to the Julian Date */
double date_to_JD(date *d);

/* Converts a Julian date to the Modified Julian date */
double safe_JD_to_MJD(double JD);

/* Converts a Modified Julian Date to the Julian date */
double safe_MJD_to_JD(double MJD);

#ifdef AMFAST

#define JD_to_MJD(x)        (x-2400000.5)
#define MJD_to_JD(x)        (x+2400000.5)

#else

#define JD_to_MJD(x)        (safe_JD_to_MJD(x))
#define MJD_to_JD(x)        (safe_MJD_to_JD(x))

#endif



#endif /* AM_TIME_H */

