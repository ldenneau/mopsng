/* am_time.c 

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "am_time.h"
#include "am_time_private.h"
#include "amma.h"
#include "am_string.h"

#ifdef UNIX_PLATFORM  /* for rusage_time */
#include <sys/time.h>
#include <sys/resource.h>
#endif

/* Globals */
static double auton_stored_t_time = 0;
char datestr[10], timestr[10];
#ifdef UNIX_PLATFORM
struct itimer_state itimer_states[3];
static struct timeval tv_interval = {AUTON_IT_CONSTANT, 0};
static struct timeval tv_off = {0,0};
#endif /* UNIX_PLATFORM */


/* Functions */
double timeval_to_double(struct timeval this)
{
  double returner;

  returner = this.tv_usec + (1000000 * this.tv_sec);
  return returner;
}

void start_wc_timer(void)
{
  struct timeval ttv;
  gettimeofday(&ttv, NULL);
  auton_stored_t_time = timeval_to_double(ttv);
}

double stop_wc_timer(void)
{
  struct timeval ttv;
  gettimeofday(&ttv, NULL);
  return (timeval_to_double(ttv) - auton_stored_t_time);
}

int global_time(void)
{
  int result = (int) time((time_t *) NULL);
  return(result);
}

char *curr_date()
{
  struct tm *thetime;
  time_t secs = time(0);
  thetime = localtime(&secs);
  sprintf(datestr, "%02d-%02d-%02d",
	  (1900+ thetime->tm_year), 1+thetime->tm_mon, thetime->tm_mday);
  return datestr;
}

char *curr_time(void)
{
  struct tm *thetime;
  time_t secs = time(0);
  thetime = localtime(&secs);
  sprintf(timestr, "%02d:%02d:%02d",
	  thetime->tm_hour, thetime->tm_min, thetime->tm_sec);
  return timestr;
}


void timeval_copy(struct timeval* targ, struct timeval* src)
{
  targ->tv_sec = src->tv_sec;
  targ->tv_usec = src->tv_usec;
}


#ifdef UNIX_PLATFORM
void prepare_timers(void)
{
  int t_iter;

  for(t_iter=0;t_iter<3;t_iter++)
    {
    itimer_states[t_iter].iter_counter = 0;
    itimer_states[t_iter].active = 1;
    }
  itimer_states[0].signal_used = SIGALRM;
  itimer_states[1].signal_used = SIGVTALRM;
  itimer_states[2].signal_used = SIGPROF;
  strncpy(itimer_states[0].name, "Clock", 10);
  strncpy(itimer_states[1].name, "Proc", 10);
  strncpy(itimer_states[2].name, "Proc+Sys", 10);
}

void start_itimer(int type)
{
  int err;
  int itdex;

  itdex = itimet_to_index(type);
	/* Reinstall the signal */
  if(itimer_states[itdex].active == 0)
    {
    printf("start_timer(): Failed to call prepare_timers() first\n");
    exit(1);
    }
  if(itimer_states[itdex].active == 2)
    {
    printf("start_timer(): Attempted to install duplicate timer\n");
    exit(1);
    }
  itimer_states[itdex].active = 2;
  signal(itimer_states[itimet_to_index(type)].signal_used, *itimer_sig_hdlr);
	/* Now reset */

  itimer_states[itdex].iter_counter = 0;

  timeval_copy(&itimer_states[itdex].itim.it_interval, &tv_interval);
  timeval_copy(&itimer_states[itdex].itim.it_value, &tv_interval);
	/* And boot up the timer */
  err = setitimer(type, &(itimer_states[itdex].itim), NULL);
  if(err)
    {
    printf("start_timer(): Failed to install itimer\n");
    exit(1);
    }
}

double check_itimer(int type)
{
  double returner = 0.0;
  struct itimerval passed_holder;
  long p_sec, p_usec;
  int tidx; /* Which signal (index) we're handling */

  tidx = itimet_to_index(type);

#ifndef AMFAST
  if(itimer_states[tidx].active == 0)
    {
    printf("getitimer(): Failed to call prepare_timers() first\n");
    exit(1);
    }
  if(itimer_states[tidx].active == 1)
    {
    printf("getitimer(): Could find no %s timer\n", itimer_states[tidx].name);
    exit(1);
    }
#endif

  if(-1 == getitimer(type, &passed_holder) )
    {
    printf("getitimer(): failed!\n");exit(1);
    }

  p_sec = tv_interval.tv_sec - passed_holder.it_value.tv_sec;
  p_usec = tv_interval.tv_usec - passed_holder.it_value.tv_usec;

  returner += (itimer_states[tidx].iter_counter * AUTON_IT_CONSTANT * 1000000);
  returner += p_sec * 1000000;
  returner += p_usec;

  if(returner < 0) returner = 0;
  return returner;
}

double stop_itimer(int type)
{
  double returner = 0.0;
  struct itimerval passed_holder;
  long p_sec, p_usec;
  int tidx; /* Which signal (index) we're handling */

  tidx = itimet_to_index(type);
  if(itimer_states[tidx].active == 0)
    {
    printf("getitimer(): Failed to call prepare_timers() first\n");
    exit(1);
    }
  if(itimer_states[tidx].active == 1)
    {
    printf("getitimer(): Could find no %s timer\n", itimer_states[tidx].name);
    exit(1);
    }
  itimer_states[tidx].active = 1;
  if(-1 == getitimer(type, &passed_holder) )
    {
    printf("getitimer(): failed!\n");exit(1);
    }
  p_sec = tv_interval.tv_sec - passed_holder.it_value.tv_sec;
  p_usec = tv_interval.tv_usec - passed_holder.it_value.tv_usec;

/*  printf("Is %ld.%ld\n", tv_interval.tv_sec, tv_interval.tv_usec);
  printf("Blah %ld %ld\n", p_sec, p_usec); */
  returner += (itimer_states[tidx].iter_counter * AUTON_IT_CONSTANT * 1000000);
  returner += p_sec * 1000000;
  returner += p_usec;
	/* Turn it off */
  timeval_copy(&itimer_states[tidx].itim.it_interval,&tv_off);
  timeval_copy(&itimer_states[tidx].itim.it_value,&tv_off);
  setitimer(type, &(itimer_states[itimet_to_index(type)].itim), NULL);
  if(returner < 0) returner = 0;
  return returner;
}

/* timer_type should be ITIMER_REAL, ITIMER_VIRTUAL OR ITIMER_PROF. */
double itimer_delta(int timer_type)
{
  double t1, t2, delta;
  prepare_timers();
  start_itimer( timer_type);
  t1 = check_itimer( timer_type);
  while (1) {
    t2 = check_itimer( timer_type);
    if (t1 != t2) break;
  }

  delta = t2-t1;
#ifndef AMFAST
  printf( "itimer_delta: delta = %g microseconds\n", delta);
#endif

  stop_itimer( ITIMER_PROF);

  return delta;
}

/* Returns minimum amount of time that must pass before quantization errors
   make the relative error of the measurement less than eps.  The time is
   returned in microseconds, since this is what the itimer routines work in.
   The recommended way to run a microsecond timing is somewhat complicated
   to reduce loop overhead.  See itimer_benchmark() for a generic
   example.
*/
/* timer_type should be ITIMER_REAL, ITIMER_VIRTUAL OR ITIMER_PROF. */
double itimer_mintime(int timer_type, double eps)
{
  double delta, Tmin;
  delta = itimer_delta( timer_type);
  Tmin = delta * (1.0/eps - 1.0);
  return Tmin;
}

/* timer_type should be ITIMER_REAL, ITIMER_VIRTUAL OR ITIMER_PROF. */
double itimer_benchmark(int timer_type, double eps, void (*testfn)(void))
{
   int n, i;
   double Tmin, T;

   n = 1;
   Tmin = itimer_mintime( timer_type, eps);
   T = -1.0;
 
   /* Warm up cache and am_malloc. */
   testfn();

   while (T <= Tmin) {
     n <<= 1;
     start_itimer( timer_type);
     for (i=0; i < n; ++i) testfn();
     T = stop_itimer( timer_type);
   }

   return T / (double) n;  
}

/* timer_type should be ITIMER_REAL, ITIMER_VIRTUAL OR ITIMER_PROF. */
double itimer_benchmark_with_data(int timer_type, double eps,
				  void (*testfn)(void *), void *data)
{
   int n, i;
   double Tmin, T;

   n = 1;
   Tmin = itimer_mintime( timer_type, eps);
   T = -1.0;

   /* Warm up cache and am_malloc. */
   testfn( data);
 
   while (T <= Tmin) {
     n <<= 1;
     start_itimer( timer_type);
     for (i=0; i < n; ++i) testfn( data);
     T = stop_itimer( timer_type);
   }

   return T / (double) n;
}

/* use getrusage to fill in the time struct.*/
struct timeval rusage_time(void)
{
  struct timeval ret;
  struct rusage rusage;

  getrusage(RUSAGE_SELF, &rusage);
  memcpy(&ret, &rusage.ru_utime, sizeof(rusage.ru_utime));  
  return ret;
}

#else /* ! ON_UNIX */
void prepare_timers(void)
{
}

void start_itimer(int type)
{
printf("itimers are presently Unix-only. Ignoring request for a timer\n");
}

double check_itimer(int type)
{
  return -1.0;
}

double stop_itimer(int type)
{
return -1.0;
}

double itimer_delta(int timer_type)
{
  return -1.0;
}

double itimer_mintime(int timer_type, double eps)
{
  return -1.0;
}

double itimer_benchmark(int timer_type, double eps, void (*testfn)(void))
{
  return -1.0;
}

double itimer_benchmark_with_data(int timer_type, double eps,
				  void (*testfn)(void *), void *data)
{
  return -1.0;
}

struct timeval rusage_time(void)
{
  struct timeval ret = {0,0};
  printf("rusage_time call is presently unix only.  Ignoring.\n");
  return ret;
}

#endif /* ON_UNIX */


/**** On win32, sleep() doesn't exist. Instead, use these wrappers, which
      call Sleep(), a function with similar functionality but different
      specifics. For portability, always use am_sleep instead of sleep()
****/
#ifdef UNIX_PLATFORM
void am_sleep(int secs)
{
  sleep(secs);
}
#else /* ! UNIX_PLATFORM */
#include <windows.h>
void am_sleep(int secs)
{
  int millisecs = 1000 * secs;
  Sleep(millisecs);
}
#endif /* UNIX_PLATFORM */

#ifdef UNIX_PLATFORM
#include <unistd.h> /* for usleep */
void am_usleep(int microsecs)
{
  usleep(microsecs);
}
#else /* ! UNIX_PLATFORM */
#include <windows.h>
void am_usleep(int microsecs)
{
  int millisecs = microsecs/1000;
  Sleep(millisecs);
}
#endif /* UNIX_PLATFORM */





#ifdef PC_PLATFORM
/* Windows lacks gettimeofday() */

int gettimeofday(struct timeval* tp, struct timezone* UNUSED)
{ 
  struct _timeb tb;
  
  _ftime(&tb);
  if(tp != NULL)
  {
    tp->tv_sec = tb.time;
    tp->tv_usec = tb.millitm * 1000;
  }
  return 0;
}

#endif /* PC_PLATFORM */

#ifdef UNIX_PLATFORM
static void itimer_sig_hdlr(int sig_id)
{
  int it_index;

  it_index = sigid_to_index(sig_id);
  itimer_states[it_index].iter_counter += 1;
  signal(itimer_states[it_index].signal_used, *itimer_sig_hdlr);
}

static int sigid_to_index(int sig_id)
{
  switch(sig_id)
    {
    case SIGALRM:
    return 0;

    case SIGVTALRM:
    return 1;

    case SIGPROF:
    return 2;
    }
  printf("FATAL in timer code: Unrecognized signal translation requested: %d\n", sig_id);
  my_error("sigid_to_index() invalid params");
  /* Never reach here */
  return -1;
}

static int itimet_to_index(int itimer_type)
{
switch(itimer_type)
	{
	case ITIMER_REAL:
		return 0;
        case ITIMER_VIRTUAL:
		return 1;
	case ITIMER_PROF:
		return 2;
	}
  printf("FATAL in timer code: Unrecognized timer translation requested: %d\n", itimer_type);
  my_error("itimet_to_index() invalid params");
  /* Never reach here */
  return -1;
}

#endif /* UNIX_PLATFORM */


void free_date(date *x)
{
  AM_FREE(x,date);
}

void fprintf_date(FILE *s,char *m1,date *x,char *m2)
{
  char *buff;

  buff = mk_printf("%s -> year",m1);
  fprintf_int(s,buff,x->year,m2);
  free_string(buff);

  buff = mk_printf("%s -> day",m1);
  fprintf_int(s,buff,x->day,m2);
  free_string(buff);

  buff = mk_printf("%s -> month",m1);
  fprintf_int(s,buff,x->month,m2);
  free_string(buff);
}

void pdate(date *x)
{
  fprintf_date(stdout,"date",x,"\n");
}

date *mk_copy_date(date *old)
{
  date *nu = AM_MALLOC(date);

  nu -> year = old -> year;
  nu -> day = old -> day;
  nu -> month = old -> month;

  return nu;
}

int month_length_non_leapyear(int month)
{
  static int month_length[] = { 31 , 28 , 31 , 30 , 31 , 30 , 31 , 31 , 30 ,
                                31 , 30 , 31 };
  my_assert(sizeof(month_length) == 12 * sizeof(int));

  if ( month < 1 || month > 12 )
    my_error("month_length_non_leapyear: month < 1 or month > 12");

  return month_length[month-1];
}

/* Returns TRUE if and only "year" is a leapyear */

bool is_leap_year(int year)
{
  bool result;
  if ( year % 4 != 0 )
    result = FALSE;
  else if ( year % 400 == 0 )
    result = TRUE;
  else if ( year % 100 == 0 )
    result = FALSE;
  else
    result = TRUE;

  return result;
}

/* Input: 1 <= month <= 12.
   Output: Number of days in month in given year. */
int month_to_length(int year,int month)
{
  int result = month_length_non_leapyear(month);
  if ( month == 2 && is_leap_year(year) )
    result += 1;
  return result;
}

/* Input: 1 <= month <= 12.
   Output: Pointer to static string (should not be freed) 
           representing month. */
char *month_to_name(int month)
{
  static char *names[] = { "JAN" , "FEB" , "MAR" , "APR" , "MAY" , "JUN" , 
                           "JUL" , "AUG" , "SEP" , "OCT" , "NOV" , "DEC" };
  my_assert(sizeof(names) == sizeof(char *) * 12);
  my_assert_always(month >= 1 && month <= 12);
  
  return names[month-1];
}

#define MAX_YEAR 2500

/* Input: 1800 <= year <= MAX_YEAR, 1 <= month <= 12,
          1 <= day <= days in month

   Output: Representation of date. */
date *mk_date(int year,int month,int day)
{
  date *x = AM_MALLOC(date);

  if ( year > 80 && year < 100 )
  {
    static bool warned = FALSE;
    if ( !warned )
    {
      printf("*\n*\n*\n*\n*\n* DATE PARSE WARNING: I see a year number\n"
	     "* of %d. I will grudgingly assume you meant 19%d, but note\n"
	     "* that this may one day lead to Y2.1K problems\n",year,year);
      warned = TRUE;
      wait_for_key();
    }
    year += 1900;
  }

  if ( year < 20 && year >= 0 )
  {
    static bool warned = FALSE;
    if ( !warned )
    {
      printf("*\n*\n*\n*\n*\n* DATE PARSE WARNING: I see a year number\n"
	     "* of %d. I will grudgingly assume you meant 20%02d, but note\n"
	     "* that this may one day lead to Y2.1K problems\n",year,year);
      warned = TRUE;
      wait_for_key();
    }
    year += 2000;
  }

  if ( year < 1800 ) my_errorf("Can't make a date involving year %d. I only\n"
			       "work with years at or after 1800AD\n",year);

  if ( year > MAX_YEAR ) my_errorf("Can't make a date involving year %d. I only\n"
			       "work with years at or before MAX_YEARAD\n",year);

  if ( month < 1 || month > 12 )
    my_errorf("When making date, month should be >= 1 and <= 12. So %s is\n"
	      "no good.\n",month);

  if ( day < 1 || day > month_to_length(year,month) )
    my_errorf("Can't have day-of-month = %d for month %s of year %d\n",
	      day,month_to_name(month),year);

  x -> year = year;
  x -> day = day;
  x -> month = month;

  return x;
}

/* Accessor */
int date_year(date *d)
{
  return d->year;
}

/* Accessor */
int date_month(date *d)
{
  return d->month;
}

/* Accessor */
int date_day(date *d)
{
  return d->day;
}

int days_since_jan1(date *d)
{
  static int days_before_month[12];
  static bool days_before_month_defined = FALSE;
  if ( !days_before_month_defined )
  {
    int i;
    days_before_month[0] = 0;
    for ( i = 2 ; i <= 12 ; i++ )
      days_before_month[i-1] = days_before_month[i-2] + 
                               month_length_non_leapyear(i-1);
  }

  if ( d->month < 1 || d->month > 12 )
    my_error("days_since_jan1: month < 1 or month > 12");

  return days_before_month[d->month-1] + d->day-1 +
         ((is_leap_year(d->year) && d->month >= 3) ? 1 : 0);
}


/*
   Let X = { a : a >= lo && a < hi && a % n == 0 }
   This function returns |X| (efficiently)

   PRE: hi >= lo
*/
int num_round_between(int lo,int hi,int n)
{
  int q = lo % n;
  int start = (q == 0) ? lo : (lo + n - q); /* start = min member of X */
  int p = hi % n;
  int end = (p == 0) ? (hi - n) : (hi - p); /* end = max member of X */

  my_assert_always(hi >= lo);

  my_assert(end >= start - n);

  return (end + n - start) / n;
}

int leap_years_since_1800(int year)
{
  return num_round_between(1800,year,4) -
         num_round_between(1800,year,100) +
         num_round_between(1800,year,400);
}

/* a daycode is an integer representing a given date.

   Tomorrow is always represented by a daycode of one greater than today,
   thus given two dates, the number of days between them equals the
   difference of their daycodes. */
int date_to_daycode(date *d)
{
  int days = days_since_jan1(d);
  int years = d->year-1800;
  int leapyears = leap_years_since_1800(d->year);
  int result = days + 365 * years + leapyears;
               
  /*
  printf("days = %d\n",days);
  printf("years = %d\n",years);
  printf("leapyears = %d\n",leapyears);
  printf("result = %d\n",result);
  */
  return result;
}


/* If parse error... returns -1 and MAKES and stores message in *r_errmess */
/* If OK sets *r_errmess to NULL */
int string_to_daycode(char *string,char **r_errmess)
{
  date *d = mk_date_from_string(string,r_errmess);
  int result = (d == NULL) ? -1 : date_to_daycode(d);
  if ( d != NULL ) free_date(d);
  return result;
}

int string_to_daycode_simple(char *string)
{
  char *errmess = NULL;
  int result = string_to_daycode(string,&errmess);
  if ( errmess != NULL )
    my_errorf("%s\n",errmess);
  return result;
}

/* If parse error... returns -1 and MAKES and stores message in *r_errmess */
/* If OK sets *r_errmess to NULL */
int daycode_from_args(char *key,int argc,char *argv[],int default_daycode,
		      char **r_errmess)
{
  char *defstring = mk_string_from_daycode(default_daycode);
  char *s = string_from_args(key,argc,argv,defstring);
  int daycode = string_to_daycode(s,r_errmess);
  free_string(defstring);
  return daycode;
}

/* If parse error... does a my_error and explains problem */
int daycode_from_args_simple(char *key,int argc,char *argv[],
			     int default_daycode)
{
  char *errmess = NULL;
  int daycode = daycode_from_args(key,argc,argv,default_daycode,&errmess);

  if ( errmess != NULL )
  {
    my_errorf("There was a problem with the date you supplied on the\n"
	      "command line next to keyword %s.\nThe problem is:\n%s\n",
	      key,errmess);
  }

  return daycode;
}

/* Returns number of days from a until b.
   0 if same date
   > 0 if b later than a
   < 0 if b earlier than a
*/
int days_between(date *a,date *b)
{
  return date_to_daycode(b) - date_to_daycode(a);
}


/* day_of_week is a number such that 0 <= day_of_week <= 6 with

   0 represents Monday
   1 represents Tuesday
      :
   5 represents Saturday
   6 represents Sunday

   Input: day_of_week
   Output: Static pointer to string that names day_of_week
*/
char *day_of_week_to_name(int day_of_week)
{
  static char *names[] = { "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN" };
  my_assert(sizeof(names) == sizeof(char *) * 7);
  my_assert_always(day_of_week >= 0 && day_of_week < 7);
  
  return names[day_of_week];
}

/* MAKES and returns a string in "MMM-DD-YYYY" where MMM is the three-letter
   text name of the month */
char *mk_string_from_date(date *d)
{
  return mk_printf("%3s-%02d-%04d",month_to_name(d->month),d->day,d->year);
}

string_array *mk_string_array_from_daycodes(ivec *daycodes)
{
  int size = ivec_size(daycodes);
  string_array *result = mk_string_array(size);
  int i;

  for ( i = 0 ; i < size ; i++ )
  {
    char *string = mk_string_from_daycode(ivec_ref(daycodes,i));
    string_array_set(result,i,string);
    free_string(string);
  }

  return result;
}

/* MAKES and returns a string in "MMM-DD-YYYY" where MMM is the three-letter
   text name of the month */
char *mk_string_from_daycode(int daycode)
{
  date *d = mk_date_from_daycode(daycode);
  char *s = mk_string_from_date(d);
  free_date(d);
  return s;
}

/* Returns the month (1 <= month <= 12) represented by the string.
   Just looks at the first three chars of string, thus

     "decadent" will return 12 (= december)

     Returns -1 if string matches no month */
int name_to_month(char *s)
{
  int result = -1;
  if ( strlen(s) >= 3 )
  {
    int m;
    char c0 = upcase_char(s[0]);
    char c1 = upcase_char(s[1]);
    char c2 = upcase_char(s[2]);

    for ( m = 1 ; result < 0 && m <= 12 ; m++ )
    {
      char *name = month_to_name(m);
      if ( c0 == name[0] && c1 == name[1] && c2 == name[2] )
	result = m;
    }
  }
  return result;
}

date *mk_date_knowing_year(char *s,char *year_string,char *month_string,
			   char *day_string,char **r_errmess)
{
  int year = -77;
  date *d = NULL;

  *r_errmess = NULL;

  if ( *r_errmess == NULL && !is_a_number(year_string) )
    *r_errmess = mk_printf("Cannot find a year in this date: %s. The string\n"
			   "%s doesn't work as a year.\n",s,year_string);

  if ( *r_errmess == NULL )
  {
    year = atoi(year_string);
    if ( (year < 0) ||
         (year >= 20 && year <= 80) ||
         (year >= 100 && year < 1800) || 
	 (year > MAX_YEAR) )
      *r_errmess = mk_printf("I insist on years between 1800 and %d.\n"
			     "The year %s from date string %s does not work\n",
			     MAX_YEAR,year_string,s);
  }

  if ( *r_errmess == NULL )
  {
    int month = -77;
    char *use_day_string = day_string;

    if ( !is_a_number(day_string) )
    {
      month = name_to_month(day_string);
      if ( month >= 0 && is_a_number(month_string) )
	use_day_string = month_string;
      else
	*r_errmess = mk_printf("The day-of-month is %s in date-string %s.\n"
			       "But that is not a number.\n",day_string,s);
    }
    else
    {
      if ( is_a_number(month_string) )
	month = atoi(month_string);
      else
      {
	month = name_to_month(month_string);
	if ( month < 0 )
	  *r_errmess = mk_printf("The month in date-string %s is %s. But\n"
				 "that is not a month I have heard of\n",s,
				 month_string);
      }
    }

    if ( *r_errmess == NULL && (month < 1 || month > 12) )
      *r_errmess = mk_printf("The month in date-string %s is %s. But\n"
			     "it should be between 1 and 12\n",s,month_string);
    
    if ( *r_errmess == NULL && !is_a_number(use_day_string) )
	*r_errmess = mk_printf("The day-of-month is %s in the date-string %s\n"
			       "but that is not a number\n",use_day_string,s);

    if ( *r_errmess == NULL )
    {
      int day = atoi(use_day_string);
      if ( day < 1 || day > month_to_length(year,month) )
	*r_errmess = mk_printf("The day-of-month is %s in the date-string %s\n"
			       "but for month %s in year %d it should be\n"
			       "between 1 and %d\n",use_day_string,s,
			       month_to_name(month),year,
			       month_to_length(year,month));
      else
	d = mk_date(year,month,day);
    }
  }

  return d;
}

bool must_be_year(char *s)
{
  return is_a_number(s) && atoi(s) >= 1000;
}

/* Only attempts parsing strings of 7 characters of this form
   ddmmmyy where mmm is a string.

   PRE: String must be of length 7
*/
date *mk_parse_date_without_gaps(char *string,char **r_errmess)
{
  char dd[20];
  char mmm[20];
  char yyyy[20];

  *r_errmess = NULL;

  my_assert( (int)strlen(string) == 7 );

  dd[0] = string[0];
  dd[1] = string[1];
  dd[2] = '\0';
  mmm[0] = string[2];
  mmm[1] = string[3];
  mmm[2] = string[4];
  mmm[3] = '\0';
  yyyy[0] = '2';
  yyyy[1] = '0';
  yyyy[2] = string[5];
  yyyy[3] = string[6];
  yyyy[4] = '\0';

  return mk_date_knowing_year(string,yyyy,mmm,dd,r_errmess);
}

/* Only attempts parsing strings of 8 characters of this form
   yyyymmdd where mmm is a string.

   PRE: String must be of length 8
*/
date *mk_parse_date_with_eight_characters(char *string,char **r_errmess)
{
  char dd[20];
  char mm[20];
  char yyyy[20];

  *r_errmess = NULL;

  my_assert( (int)strlen(string) == 8 );

  dd[0] = string[6];
  dd[1] = string[7];
  dd[2] = '\0';
  mm[0] = string[4];
  mm[1] = string[5];
  mm[2] = '\0';
  yyyy[0] = string[0];
  yyyy[1] = string[1];
  yyyy[2] = string[2];
  yyyy[3] = string[3];
  yyyy[4] = '\0';

  return mk_date_knowing_year(string,yyyy,mm,dd,r_errmess);
}

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

   If a parse error returns NULL, and MAKES and RETURNS an error message
   (which must later be free_string()'d in *r_errmess.

   If happy, returns date d, and sets *r_errmess to NULL.
*/
date *mk_date_from_string(char *s,char **r_errmess)
{
  string_array *sa = mk_broken_string_using_seppers(s," -:/");
  int size = string_array_size(sa);
  date *d = NULL;

  *r_errmess = NULL;

  if ( size == 0 || size > 3 )
    *r_errmess = mk_printf("The string \"%s\" can't be parsed as a date",s);
  else 
  {
    char *s0 = (size>0) ? string_array_ref(sa,0) : NULL;
    char *s1 = (size>1) ? string_array_ref(sa,1) : NULL;
    char *s2 = (size>2) ? string_array_ref(sa,2) : NULL;
    
    if ( size == 3 )
    {
      if ( must_be_year(s0) )
	d = mk_date_knowing_year(s,s0,s1,s2,r_errmess);
      else if ( must_be_year(s1) )
	d = mk_date_knowing_year(s,s1,s0,s2,r_errmess);
      else
	d = mk_date_knowing_year(s,s2,s0,s1,r_errmess);
    }
    else if ( size == 2 )
    {
      if ( must_be_year(s0) )
	d = mk_date_knowing_year(s,s0,s1,"1",r_errmess);
      else
	d = mk_date_knowing_year(s,s1,s0,"1",r_errmess);
    }
    else if ( size == 1 )
    {
      if ( (int)strlen(s0) == 7 )
	d = mk_parse_date_without_gaps(s0,r_errmess);
      else if ( (int)strlen(s0) == 8 )
	d = mk_parse_date_with_eight_characters(s0,r_errmess);
      else
	d = mk_date_knowing_year(s,s0,"1","1",r_errmess);
    }
  }

  free_string_array(sa);

  my_assert(d == NULL || *r_errmess == NULL);
  my_assert(d != NULL || *r_errmess != NULL);

  return d;
}

/* Returns 0 for monday, 1 for tuesday, ... 5 for saturday, 6 for sunday */
int daycode_to_day_of_week(int daycode)
{
  int result = (daycode+2) % 7;
  return result;
}

/* Returns 0 for monday, 1 for tuesday, ... 5 for saturday, 6 for sunday */
int date_to_day_of_week(date *d)
{
  int daycode = date_to_daycode(d);
  return daycode_to_day_of_week(daycode);
}

/* Same as mk_date_from_string above, except my_error()'s if parse error */
date *mk_date_from_string_simple(char *s)
{
  char *errmess;
  date *d = mk_date_from_string(s,&errmess);
  if ( d == NULL ) my_error(errmess);
  return d;
}

#define MAX_DAYCODES ((MAX_YEAR + 1 - 1800) * 366)

int global_num_daycodes = -1;
int global_daycode_to_year[MAX_DAYCODES];
int global_daycode_to_month[MAX_DAYCODES];
int global_daycode_to_day[MAX_DAYCODES];

/* Makes the date d such that date_to_daycode(d) = daycode */
date *mk_date_from_daycode(int daycode)
{
  date *d;

  if ( global_num_daycodes < 0 )
  {
    int y;
    int my_daycode = 0;
    printf("Filling date cache...\n");
    for ( y = 1800 ; y <= MAX_YEAR ; y++ )
    {
      int m;
      for ( m = 1 ; m <= 12 ; m++ )
      {
	int day;
	int days_in_month = month_to_length(y,m);
	for ( day = 1 ; day <= days_in_month ; day++ )
	{
	  my_assert(my_daycode < MAX_DAYCODES);
	  global_daycode_to_year[my_daycode] = y;
	  global_daycode_to_month[my_daycode] = m;
	  global_daycode_to_day[my_daycode] = day;
	  my_daycode += 1;
	}
      }
    }
    printf("...finished filling date cache\n");
    global_num_daycodes = my_daycode;
  }

  if ( daycode < 0 )
    my_errorf("mk_date_from_daycode: daycode = %d < 0",daycode);
  else if ( daycode >= global_num_daycodes )
    my_errorf("mk_date_from_daycode: daycode = %d would go beyond MAX_YEAR",
	      daycode);

  d = mk_date(global_daycode_to_year[daycode],
	      global_daycode_to_month[daycode],
	      global_daycode_to_day[daycode]);

  my_assert(daycode == date_to_daycode(d));

  return d;
}

/* Change d so that it is "num_days" later.
   If num_days is -ve, moves to appropriate earlier date. */
void increment_date(date *d,int num_days)
{
  int old_daycode = date_to_daycode(d);
  int new_daycode = old_daycode + num_days;
  date *temp = mk_date_from_daycode(new_daycode);
  d->year = temp->year;
  d->month = temp->month;
  d->day = temp->day;
  free_date(temp);
}

/*
void date_main(int argc,char *argv[])
{
  char *s = string_from_args("date",argc,argv,NULL);
  if ( s == NULL )
    my_errorf("You must put date <string> on command line");
  else
  {
    date *d = mk_date_from_string_simple(s);
    char *s2 = mk_string_from_date(d);
    int daycode = date_to_daycode(d);
    int day_of_week = date_to_day_of_week(d);
    date *d2 = mk_date_from_daycode(daycode);
    char *d2_string = mk_string_from_date(d2);

    pdate(d);

    printf("daycode = %d\nday_of_week = %d\n",daycode,day_of_week);

    printf("The string %s is parsed as date %s, which is a %s\n",
	   s,s2,day_of_week_to_name(day_of_week));

    printf("mk_date_from_daycode(%d) = %s\n",daycode,d2_string);
	   
    free_string(s2);
    free_string(d2_string);
    free_date(d);
    free_date(d2);
  }
} */

/************ Begin dates *************/

static void void_free_date(void *data)
{
  free_date((date *)data);
}

static void *void_mk_copy_date(void *data)
{
  return (void *) mk_copy_date((date *)data);
}

static void void_fprintf_date(FILE *s,char *m1,void *data,char *m2)
{
  fprintf_date(s,m1,(date *) data,m2);
}

dates *mk_empty_dates(void)
{
  dates *array = AM_MALLOC(dates);
  array -> genarr = mk_empty_generic_array(void_free_date,
					   void_mk_copy_date,
					   void_fprintf_date);
  return array;
}

void add_to_dates(dates *array,date *element)
{
  add_to_generic_array(array->genarr,(void *)element);
}

void add_pointer_to_dates(dates *array,date *element)
{
  add_pointer_to_generic_array(array->genarr,(void *)element);
}

int dates_size(dates *array)
{
  return(generic_array_size(array->genarr));
}

date *dates_ref(dates *array,int idx)
{
  return (date *) generic_array_ref(array->genarr,idx);
}
  
void fprintf_dates(FILE *s,char *m1,dates *array,char *m2)
{
  fprintf_generic_array(s,m1,array->genarr,m2);
}

void pdates(dates *array)
{
  fprintf_dates(stdout,"dates",array,"\n");
}

void free_dates(dates *array)
{
  free_generic_array(array->genarr);
  AM_FREE(array,dates);
}

dates *mk_copy_dates(dates *array)
{
  dates *new_dates = AM_MALLOC(dates);
  new_dates -> genarr = mk_copy_generic_array(array->genarr);
  return new_dates;
}

void dates_set(dates *array,int idx,date *d)
{
  generic_array_set(array->genarr,idx,(void *) d);
}

void dates_remove(dates *array,int idx)
{
  generic_array_remove(array->genarr,idx);
}



/* Converts the date and time (given in fractional days) 
   to the Modified Julian Date (MJD). The MJD is the total 
   number of (fractional) days since midnight 
   November 17, 1858.  It is also the Julian date - 2400000.5.  

   Adapted from: "Astronomy on the Personal Computer"
                  by Montenbruck and Pfleger */
double date_time_to_MJD(int year, int month, int day, double frac_days) {
  double leapdays;
  double daysum = 0.0;

  /* Trap invalid dates... */
  my_assert((month > 0)&&(month <= 12));
  my_assert(year > 1752);
  
  if(month <= 2) {
    month += 12;
    year--;
  }
  month++;

  /* Count the number of leap days seen */
  leapdays = (year/400) - (year/100) + (year/4);

  /* First calculate the number of days to the current date */
  daysum += (365.0 * (double)year);
  daysum += leapdays;
  daysum += (double)((int)(30.6001 * month));
  daysum += (double)day;

  /* Subtract of the days to Nov 17, 1858 */
  daysum -= (679004.0);

  return daysum + frac_days;
}


double date_HMS_to_MJD(int year, int month, int day, int hrs, int min, double sec) {
  double frac_days = ((double)hrs)/24.0;

  frac_days += ((double)min)/1440.0;
  frac_days += sec/86400.0;

  return date_time_to_MJD(year,month,day,frac_days);
}


double frac_date_to_MJD(int year, int month, double day) {
  return date_time_to_MJD(year,month,(int)day,day - (double)((int)day));
}



/* Converts a date to the Modified Julian Date */
double date_to_MJD(date *d) {
  return date_time_to_MJD(date_year(d),date_month(d),date_day(d),0.0);
}


void MJD_to_date(double mjd, int* year, int* month, double* day) {
  long a,b,c,d,e,f;

  a = (long)(mjd+2400001.0);
  if(a < 2299161) {
    b = 0;
    c = a + 1524;
  } else {
    b = (long)(((double)a - 1867216.25)/36524.25);
    c = a + b - (b/4) + 1525;
  }

  d = (long)(((double)c-122.1)/365.25);
  e = 365*d + d/4;
  f = (long)( ((double)(c-e))/30.6001 );

  day[0]  = (double)(c - e - (int)(30.6001*(double)f));
  day[0] += (mjd - (double)((int)mjd));
  month[0] = f - 1 - 12*(f/14);
  year[0]  = d - 4715 - ((7+month[0])/10);
}


date* mk_MJD_date(double mjd) {
  date* res;
  int year = 0;
  int month = 0;
  double day = 0.0;

  MJD_to_date(mjd,&year,&month,&day);
  res = mk_date(year,month,(int)day);
  
  return res;
}


/* Converts the date and time (given in fractional days) 
   to the Julian Date (JD).  The JD is the number of
   (fractional) days since noon January 1, 2413 BCE in the 
   Julian Calendar.  The function currently only handles the
   Gregorian Calendar (after 1752).

   Adapted from: "Astronomy on the Personal Computer"
                  by Montenbruck and Pfleger */
double date_time_to_JD(int year, int month, int day, double frac_days) {
  double res = date_time_to_MJD(year,month,day,frac_days);
  return res + 2400000.5;
}


/* Converts a date to the Julian Date */
double date_to_JD(date *d) {
  return date_time_to_JD(date_year(d),date_month(d),date_day(d),0.0);
}


/* Converts a Julian date to the Modified Julian date */
double safe_JD_to_MJD(double JD) {
  return JD - 2400000.5;
}


/* Converts a Modified Julian Date to the Julian date */
double safe_MJD_to_JD(double MJD) {
  return MJD + 2400000.5;
}


