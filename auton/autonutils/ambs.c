/* **
   File:            ambs.c
   Author:          Andrew William Moore
   Created:         7th Feb 1990
   Updated:         8 Dec 96
   Description:     Standard andrew tools. These are trivial and tedious.

   Copyright 1996, Schenley Park Research.

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

/* ------------------- includes ------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <setjmp.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include "amma.h"
#include "ambs.h"
#include "backtrace.h"
#include "am_string.h"
#include "am_file.h"
#include "am_time.h"
#include "internal_state.h"

#ifdef OLD_SPR
#include "adgui.h"
#endif

/* ------------------- definitions ----------------------------*/
#ifdef PC_MVIS_PLATFORM
extern char GetcBuf;
extern HWND hEditIn;
extern HANDLE KeyPress;
extern CRITICAL_SECTION critsect;
#endif

#define PD_A 55
#define PD_B 24
typedef struct pd_state
{
  int cursor;
  unsigned int val[PD_A];
} pd_state;

static pd_state Pd_state;
static bool Pd_state_defined = FALSE;

static bool Currently_saving_state = FALSE;
static pd_state Saved_pd_state;

/* name of the log file for messages */
#define MAX_LOGFILE_NAME_LENGTH 100
#define AM_LOGFILE "AM_LOGFILE"
static char g_logfilename[MAX_LOGFILE_NAME_LENGTH] = "";

/* Private prototypes  -- A lot of these maybe should be public */
static void copy_pd_state(pd_state *src,pd_state *dst);
static unsigned int pd_random_29bit(void);
static double basic_range_random(double lo, double hi);
static double basic_gen_gauss(void);
static int very_basic_index_of_arg(const char *opt_string, int argc, char *argv[]);
static int basic_index_of_arg(const char *opt_string, int argc, char *argv[]);
static char *basic_string_from_args(const char *key, int argc, char *argv[], char *default_value);
static FILE *open_logfile(void);
static void btinfo_to_logfile(btinfo *bt);

/* The VC++ version on the PCs uses a different (dialog/menu-based)
   interface, so does not include this code.... */
/* #ifndef PC_MVIS_PLATFORM */

int Ignore_next_n = 0; /* used by amma, so not static, but also not exported */

static void do_wait_for_key(bool really_wait);
static void wait_for_key_shell(bool really_wait);

static char *get_user_input_shell(const char *message, const char *def_reply);
/* ---------------------------------------------------------------*/

/* exported, so that its possible to find out if a library was
   compiled in fast mode.  For example, used by ASL to make sure auton
   algs compiled in debug mode aren't run from an ASL gui compiled in
   fast mode - which would crash.
*/
bool am_is_fastmode()
{
  #ifdef AMFAST
  return TRUE;
  #endif
  return FALSE;
}

/* -------- message logging ------------ */
bool set_logfile_name(const char *str)
{
  bool ret = TRUE;
  if (strlen(str) < MAX_LOGFILE_NAME_LENGTH){
    strcpy(g_logfilename, str);
  }
  else {
    my_errorf("set_logfile_name called with arg longer than %d characters.\n",
	      MAX_LOGFILE_NAME_LENGTH);
    ret = FALSE;
  }
  return ret;
}

static FILE *open_logfile(void)
{
  char *filename;
  FILE *outfile = NULL;
  static bool have_warned = FALSE;

  filename = getenv(AM_LOGFILE);
  if (!filename) filename = g_logfilename;

  if (filename && strcmp(filename, "") != 0)
  {
    outfile = fopen(filename, "a");
    if (!outfile && !have_warned){
      char *env_str = " in environment variable AM_LOGFILE ";
      printf("Warning: logfile name specified as '%s'\n %s"
	     "but I can't open this file.\n", filename, 
	     getenv(AM_LOGFILE) ? env_str : "");
      have_warned = TRUE;
    }
  }
  return outfile;
}

void my_logmsg(const char *format, ...)
{
  FILE *outfile = NULL;
  static bool wrote_header = FALSE;

  outfile = open_logfile();
  if (outfile){
    /* write the header */
    if (! wrote_header)
    {
      fprintf(outfile, "\n======= Logging started: %s (%s) =======\n\n", 
	      curr_date(), curr_time());
      wrote_header = TRUE;
    }

    /* write the message */
    {
      VA_MAGIC_INTO_BIG_ARRAY;
      fprintf(outfile, "%s", big_array);
      if (strrchr(big_array, '\n') != (big_array + strlen(big_array) - 1)){
	fprintf(outfile,"\n");
      }
    }
    fclose(outfile);
  }
}


    
  

/* helper function for logging backtrace info */
void btinfo_to_logfile(btinfo *bt)
{
  FILE *outfile = open_logfile();
  if (outfile){
    fprintf_btinfo( outfile, "\n", bt, "\n");
    fclose(outfile);
  }
}

void my_info(const char *format, ...)
{
  VA_MAGIC_INTO_BIG_ARRAY;
  my_printf(0, "NOTE: %s\n",big_array);
}

void my_infof(const char *format, ...)
{
  VA_MAGIC_INTO_BIG_ARRAY;
  my_info("%s", big_array);
}

/*-----------------------------------------------------*/
void my_breakpoint(void)
{
#ifndef AMFAST
  my_printf(0, "I suggest you set a breakpoint at the function "
	    "my_breakpoint()\n");
#endif

#ifdef BTDEBUG
  {
    btinfo *bt;
    bt = mk_btinfo( 20 /* max num stack frames */);
    btinfo_to_logfile(bt);
    fprintf_btinfo( stderr, "\n", bt, "\n");
    free_btinfo( bt);
  }
#endif

  /* skip */
}

void my_warning(const char *format, ...)
{
  VA_MAGIC_INTO_BIG_ARRAY;
  my_warning_no_wait(big_array);
  wait_for_key();
}

void my_warning_no_wait(const char *format, ...)
{
  VA_MAGIC_INTO_BIG_ARRAY;
  my_printf(0, "Warning! %s\n",big_array);
}

/* my_error calls the my_error function pointer which in turn
   calls the original my_error (renamed to my_error_default).  Purpose
   is to allow some programs to override the my_error behavior to 
   avoid the stack trace and exit.
*/
void my_error(const char *string)
{
  my_error_ptr_t error_ptr = am_get_my_error_ptr();
  error_ptr(string);
}
void my_error_default(const char *string)
{
  /* AWM: I put this back so that I can see the printed output in
     the gray window before
     jumping into the debugger. */
  my_printf(0, "***** Auton software error:\n***** %s\n",string);
  /* If you want color, exchange the previous print for the following print. */
  /* printf("\033[00m\033[01;37;45m***** Auton software error:\n***** "
     "%s\033[00m\n",string); */

#ifndef AMFAST
  my_breakpoint();
  my_printf(0, "***** Auton software error:\n***** %s\n", string);
  my_printf(0, "Will now exit (if debugging I suggest you set a breakpoint "
	 "at my_breakpoint)\n");
#ifndef ASL_BUILD
  really_wait_for_key();
#endif
#endif


  exit(-1);
}
void my_errorf(const char *format, ...)
{
  VA_MAGIC_INTO_BIG_ARRAY;
  my_error(big_array);
}



/* my_print calls the my_print_ptr function pointer which in turn
   calls my_print_default (unless the my_print function has been reassigned.)
   Purpose  is to allow some programs to override the my_print behavior
*/
void my_print(int level, const char *string)
{
  am_internal_state_t *state_ptr = am_get_internal_state();
  state_ptr->print_ptr(level, string);
}
void my_print_default(int level, const char *string)
{
  if (level <= Global_get_verbosity()){
    printf("%s", string);
    
    /* log it */
    my_logmsg("%s", string);
  }
}
void my_printf(int level, const char *format, ...)
{
  if (level <= Global_get_verbosity())
  {
    /* write the message */
    VA_MAGIC_INTO_BIG_ARRAY;
    my_print(level, big_array);
  }
}
void my_printf_stderr(const char *format, ...)
{
  /* write the message */
  VA_MAGIC_INTO_BIG_ARRAY;
  fprintf(stderr, "%s", big_array);
  
  /* log it */
  my_logmsg("%s", big_array);
}


/* Make sure the assert statement has no side effects that you need
   because it won't be executed in fast mode */
void my_assert_always(bool b)
{
  if ( !b )
    my_error("my_assert failed");
}

void status_report(const char *message)
{
  my_printf(1, "%s\n",message);
}

/* Really want to remove this and just use the state_ptr->Verbosity.  Access
   should only be via Global_set_verbosity and my_printf.
   For now, over 1200 lines of code use this directly, so we have
   to keep it. */
double Verbosity = 0.0;     

int Global_get_verbosity(void)
{
  /* Get verbosity from global internal_state */
  am_internal_state_t *state_ptr = am_get_internal_state();
  int ret = state_ptr->Verbosity;
  
  return ret;
}
void Global_set_verbosity(int level)
{
  am_internal_state_t *state_ptr = am_get_internal_state();
  Verbosity = level;

  state_ptr->Verbosity = level;
  if (state_ptr->Verbosity <= 0) state_ptr->Verbosity = 0;
  if (state_ptr->Verbosity >= 5) state_ptr->Verbosity = 5;
}

char *safe_malloc(unsigned size)
{
  char *res;

  if ( size == 0 )
    res = NULL;      /* Doesn't matter what we return, but lets be tidy */
  else
  {
    res = (char *) malloc(size);
    if ( res == NULL )
      my_error("Safe malloc wasn't.\n");
  }
  return(res);
}

FILE *safe_fopen(const char *fname, const char *access)
{
  FILE *s = fopen(fname, access);
  if ( s == NULL )
  {
    perror( "safe_fopen");
    my_printf(0, "Could not fopen('%s','%s')\n",fname,access);
    my_error("safe_fopen() failed");
  }
  return(s);
}

bool eq_string(const char *s1, const char *s2)
{
  my_assert(s1 != NULL);
  my_assert(s2 != NULL);
  return(strcmp(s1,s2) == 0 );
}

bool eq_string_with_length(const char *s1, const char *s2, int n)
{
  return(strncmp(s1,s2,n) == 0 );
}

/*
   New Random Functions
   Author:      Darrell Kindred + a few mods by Justin Boyan
                 + tiny final adjustments by Andrew Moore
   Created:     Sun. Mar  3, 1996 16:22
   Description: the random number generator recommended by Persi Diaconis
                in his lecture at CMU, February 1996.

		The numbers (x_n) in the sequence are generated by
		  x_n = x_{n-PD_A} * x_{n-PD_B}   (mod 2^32)
		where PD_A=55, PD_B=24.  This gives a pseudorandom
		sequence of ODD integers in the interval [1, 2^32 - 1].

   Modified:    Mon Feb 17 13:42:19 EST 1997
                Scott identified hideous correlations in the low-order bit.
		As a temporary hack, I've killed the lowest 3 bits of
		each number generated.  This seems to help, but we should
		look into getting a more trustworthy RNG.  persi_diaconis--
		The old version is backed up in xdamut/backup/97-02-17.ambs.c
*/




void copy_pd_state(pd_state *src,pd_state *dst)
{
  int i;
  dst -> cursor = src -> cursor;
  for ( i = 0 ; i < PD_A ; i++ )
    dst -> val[i] = src -> val[i];
}

void push_current_am_srand_state(void)
{
  if ( Currently_saving_state )
    my_error("save current am_srand_state: already being pushed. Implement a real stack.");
  copy_pd_state(&Pd_state,&Saved_pd_state);
  Currently_saving_state = TRUE;
}

void pop_current_am_srand_state(void)
{
  if ( !Currently_saving_state )
    my_error("pop_current am_srand_state: nothing pushed.");
  copy_pd_state(&Saved_pd_state,&Pd_state);
  Currently_saving_state = FALSE;
}

unsigned int pd_random_29bit(void) 
{
    int c = Pd_state.cursor;
    if (!Pd_state_defined) 
    {
      /* not seeded */
      am_srand(5297);
    }
    /* this assumes 32-bit unsigned int */
    Pd_state.val[c] *= Pd_state.val[(c + PD_A - PD_B) % PD_A];

    Pd_state.cursor = ((c + 1) % PD_A);

    return Pd_state.val[c] >> 3;
}

unsigned int pd_random_32bit(void) 
{
    unsigned int x = pd_random_29bit(), y = pd_random_29bit();
    return x ^ (y<<3);
}


/**********************************************************************
 * interface for the outside world...  (jab)
 **********************************************************************/


/* Pass in a single seed, which can be any unsigned int */
void am_srand(int seed)
{
  unsigned int us_seed = (unsigned int) seed & 0x3FFFFFFF;
  unsigned int i;
  int four = 4;

    /* this code requires 32-bit ints */
  if (sizeof(unsigned int) != four) 
    my_error("this code requires 32-bit integers");

    /* make it odd */
  us_seed = us_seed * 2 + 1;
    /* add +0,+2,+4,... */

  for (i=0; i<PD_A; i++) {
    Pd_state.val[i] = us_seed + 2 * i;
  }
  Pd_state.cursor = 0;
  Pd_state_defined = TRUE;

    /* Throw out the first few numbers (enough to be sure that 
     * all values in the state have "wrapped around" at least twice). */
    for (i=0; i<400+(us_seed % 100); i++) {
	(void) pd_random_29bit();
    }
}

void am_randomize(void)
{
    time_t my_time = time(NULL);
    char *time_string = ctime(&my_time);
    int i = 0;
    int seed = 0;
    
    while ( time_string[i] != '\0' )  {
	seed += i * (int) (time_string[i]);
	i += 1;
    }
    am_srand(seed);
}

double pd_random_double(void) 
{
  /* returns a double chosen uniformly from the interval [0,1) */
  double r29 = pd_random_29bit();
  return (r29 / ((unsigned int) (1<<29)));
}


double basic_range_random(double lo, double hi)
{
    return lo + (hi-lo)*pd_random_double();
}

static int rrcalls = 0;

double range_random(double lo, double hi)
{
  double result = basic_range_random(lo,hi);
  /*  if ( rrcalls < 10 )
  //    my_printf(0, "rrcall %d: range_random(%g,%g) = %g\n",rrcalls,lo,hi,result);
  */
  rrcalls += 1;
  return result;
}

int int_random(int n)
{
  int result = -1;
  if ( n < 1 ) 
  {
    my_errorf("You called int_random(%d), %d is <=0)", n, n);
    return(0);
  }
  else
    result = pd_random_29bit() % n;

  return(result);
}

static int GG_iset = 0;
static double GG_gset = -77.77;

static int Gcalls = 0;

double basic_gen_gauss(void)
{
  double fac = 0.0;
  double r = 0.0;
  double v1 = 0.0;
  double v2 = 0.0;

  double result = -777.777;

  if  (GG_iset == 0) 
  {
    bool ok = FALSE;
    while ( !ok )
    {
      v1=2.0*range_random(0.0,1.0)-1.0;
      v2=2.0*range_random(0.0,1.0)-1.0;
      r=v1*v1+v2*v2;
      ok = r < 1.0;
    }
    fac=sqrt(-2.0*log(r)/r);
    GG_gset=v1*fac;
    GG_iset=1;
    result = v2*fac;

    if (  result < -10000.0 ) 
      my_error("gen_gauss nuts");
  } 
  else 
  {
    GG_iset=0;
    return GG_gset;
  }

  return result;
}

double gen_gauss(void)
{
  double result = basic_gen_gauss();

  Gcalls += 1;
  return result;
}

double real_min(double x, double y)
{
  return( (x < y) ? x : y );
}

double real_max(double x, double y)
{
  return( (x < y) ? y : x );
}

double real_abs(double x)
{
  return( (x < 0.0) ? -x : x );
}

double real_square(double x)
{
  return( x * x );
}

double real_cube(double x)
{
  return( x * x * x );
}

int int_min(int x, int y)
{
  return( (x < y) ? x : y );
}

int int_max(int x, int y)
{
  return( (x < y) ? y : x );
}

int int_abs(int x)
{
  return( (x < 0) ? -x : x );
}

int int_square(int x)
{
  return( x * x );
}

int int_cube(int x)
{
  return( x * x * x );
}

long long_min(long x, long y)
{
  return( (x < y) ? x : y );
}

long long_max(long x, long y)
{
  return( (x < y) ? y : x );
}

long long_abs(long x)
{
  return( (x < 0) ? -x : x );
}

long long_square(long x)
{
  return( x * x );
}

long long_cube(long x)
{
  return( x * x * x );
}

bool am_isnan( double val)
{
  if (val < 0.0 || val >= 0.0) return FALSE;
  else return TRUE;
}

bool am_isinf( double val)
{
  if (val + 1.0 == val) return TRUE;
  else return FALSE;
}

bool am_isnum( double val)
{
  return !(am_isnan( val) || am_isinf( val));
}

double am_copysign(double x, double y)
{
#ifdef PC_PLATFORM
  return _copysign(x,y);
#else
  return copysign(x,y);
#endif
}

char *input_string(const char *mess, char *string, int string_size)
{
  int i;
  int c = '\n';

  while ( c == ' ' || c == '\n' )
  {
    if ( c == '\n' )
      my_printf(0, "%s",mess);
    c = getchar();
  }

  for ( i = 0 ; i < string_size-1 && c != ' ' && c != '\n' && c != EOF ; i++ )
  {
    string[i] = (char) c;
    c = getchar();
  }

  string[i] = '\0';
  return(string);
}

double input_realnum(const char *mess)
{
  char double_name[100];
  bool happy = FALSE;
  double result;
  
  while ( !happy )
  {
    (void) input_string(mess,double_name,100);
    happy = 1 == sscanf(double_name,"%lf",&result);
  }

  return(result);
}

int input_int(const char *mess)
{
  double x = input_realnum(mess);
  int result = my_irint(x);
  if ( real_abs(x-result) > 0.001 )
    my_printf(0, "input_int(): Warning.. truncated your doubleing point number\n");
  return(result);
}

bool input_bool(const char *mess)
{
  int c = '\n';
  int cig;
  while ( c != 'n' && c != 'N' && c != 'y' && c != 'Y' )
  {
    c = '\n';
    while ( c == ' ' || c == '\n' )
    {
      if ( c == '\n' )
        my_printf(0, "%s",mess);
      c = getchar();
    }
    cig = c;
    while ( cig != '\n' && cig != ' ' )
      cig = getchar();
  }

  return(c == 'y' || c == 'Y');
}

int very_basic_index_of_arg(const char *opt_string, int argc, char *argv[])
/* Searches for the string among the argv[]
   arguments. If it finds it, returns the index,
   in the argv[] array, of that following argument.
   Otherwise it returns -1.

   argv[] may contain NULL.
*/
{
  int i;
  int result = -1;
  for ( i = 0 ; result < 0 && i < argc ; i++ )
    if ( argv[i] != NULL && strcmp(argv[i],opt_string) == 0 )
      result = i;

  if ( Verbosity > 350.0 )
  {
    if ( result >= 0 )
      my_printf(0, "index_of_arg(): Key `%s' from in argument number %d\n",
             argv[result],result
            );
    else
      my_printf(0, "index_of_arg(): Key `%s' not found\n",opt_string);
  }

  return(result);
}

#define MAX_ARG_CHARS 200

int basic_index_of_arg(const char *opt_string, int argc, char *argv[])
/* Searches for the string among the argv[]
   arguments. If it finds it, returns the index,
   in the argv[] array, of that following argument.
   Otherwise it returns -1.

   argv[] may contain NULL.

   It is INSENSITIVE TO THE INITIAL - sign in front of a key
*/
{
  int result;

  if ( Verbosity > 2000.0 )
  {
    int i;
    my_printf_stderr("Welcome to basic_index_of_arg\n");
    my_printf_stderr("opt_string = %s\n",
		     (opt_string==NULL)?"NULL" : opt_string);
    my_printf_stderr("argc = %d\n",argc);
    for ( i = 0 ; i < argc ; i++ )
      my_printf_stderr("argv[%d] = %s\n",i,
		 (argv[i]==NULL) ? "NULL" : argv[i]);
  }

  result = very_basic_index_of_arg(opt_string,argc,argv);
  if ( result < 0 )
  {
    int i;
    char other_key[MAX_ARG_CHARS+2];
    if ( opt_string[0] == '-' )
    {
      for ( i = 0 ; i < MAX_ARG_CHARS && opt_string[i+1] != '\0' ; i++ )
        other_key[i] = opt_string[i+1];
      other_key[i] = '\0';
    }
    else
    {
      other_key[0] = '-';
      for ( i = 0 ; i < MAX_ARG_CHARS && opt_string[i] != '\0' ; i++ )
        other_key[i+1] = opt_string[i];
      other_key[i+1] = '\0';
    }

    result = very_basic_index_of_arg(other_key,argc,argv);
  }

  return(result);
}
    
char *basic_string_from_args(const char *key, int argc, char *argv[], char *default_value)
{
  int idx = basic_index_of_arg(key,argc,argv);
  char *result;

  if ( idx >= 0 && idx < argc-1 )
    result = argv[idx+1];
  else
    result = default_value;

  return(result);
}

/**** Now for public, talkative, ones ****/

bool arghelp_wanted(int argc, char *argv[])
{
  bool result = basic_index_of_arg("arghelp",argc,argv) >= 0 ||
                basic_index_of_arg("-arghelp",argc,argv) >= 0;
  return(result);
}

int index_of_arg(const char *opt_string, int argc, char *argv[])
/* Searches for the string among the argv[]
   arguments. If it finds it, returns the index,
   in the argv[] array, of that following argument.
   Otherwise it returns -1.

   argv[] may contain NULL.
*/
{
  int result = basic_index_of_arg(opt_string,argc,argv);
  if ( arghelp_wanted(argc,argv) )
    my_printf(0, "ARGV %21s %8s         %8s %s on commandline\n",
	      opt_string,"","",
	      (result >= 0) ? "   " : "not"
	      );

  return(result);
}

char *mk_string_from_args(const char *key,int argc,char *argv[],char *default_value)
{
  return (mk_copy_string(string_from_args(key,argc,argv,default_value)));
}

char *string_from_args(const char *key, int argc, char *argv[], char *default_value)
{
  char *result = basic_string_from_args(key,argc,argv,default_value);
  if ( arghelp_wanted(argc,argv) )
  {
    my_printf(0, "ARGV %21s %8s default %8s ",
	      key,"<string>",
	      (default_value == NULL) ? "NULL" : default_value
	      );
    if ( (default_value == NULL && result == NULL) || 
         (default_value != NULL && eq_string(default_value,result))
       )
      my_printf(0, "\n");
    else
      my_printf(0, "commandline %s\n",result);
  }

  return(result);
}

char *string_from_args_insist(const char *key,int argc,char *argv[])
{
  char *s = string_from_args(key,argc,argv,NULL);
  if ( s == NULL )
    my_errorf("I insist that keyword %s is on the command line followed\n"
	      "by some value. Please add %s <value> to the command line\n",
	      key,key);
  return s;
}

double double_from_args(const char *key, int argc, char *argv[], double default_value)
{
  const char *string = basic_string_from_args(key,argc,argv,(char *)NULL);
  double result = (string == NULL) ? default_value : atof(string);

  if ( arghelp_wanted(argc,argv) )
  {
    my_printf(0, "ARGV %21s %8s default %8g ",
	      key,"<double>",default_value
	      );
    if ( string == NULL )
      my_printf(0, "\n");
    else
      my_printf(0, "commandline %g\n",result);
  }

  return(result);
}

int int_from_args(const char *key, int argc, char *argv[], int default_value)
{
  const char *string = basic_string_from_args(key,argc,argv,(char *)NULL);
  int result = (string == NULL) ? default_value : atoi(string);

  if ( arghelp_wanted(argc,argv) )
  {
    my_printf(0,"ARGV %21s %8s default %8d ",
           key,"<int>",default_value
          );
    if ( string == NULL )
      my_printf(0,"\n");
    else
      my_printf(0,"commandline %d\n",result);
  }

  return(result);
}

bool bool_from_args(const char *key, int argc, char *argv[], bool default_value)
/*
   Searches for two adjacent lexical items in argv in which the first 
   item equals the string 'key'.

   Returns TRUE if and only if the second item is a string beginning
   with any of the following characters: t T y Y 1
   (this is meant to accomodate any reasonable user idea of words for
    specifying boolean Truthhood)


   If doesn't find anything matching key then returns default_value.
*/
{
  const char *string = basic_string_from_args(key,argc,argv,(char *)NULL);
  bool result = (string == NULL) ? default_value : 
                (string[0] == 't' || string[0] == 'T' ||
                 string[0] == 'y' || string[0] == 'Y' ||
                 string[0] == '1'
                );

  if ( arghelp_wanted(argc,argv) )
  {
    my_printf(0,"ARGV %21s %8s default %8s ",
           key,"<bool>",(default_value) ? "TRUE" : "FALSE"
          );
    if ( string == NULL )
      my_printf(0,"\n");
    else
      my_printf(0,"commandline %s\n",(result)?"TRUE":"FALSE");
  }

  return(result);
}

int my_irint(double x)
/* Returns the closest integer to x */
{
  int returner = (int)floor(x + 0.5);
  return returner;
}

void fprintf_int(FILE *s,const char *m1,int x,const char *m2)
{
  fprintf(s,"%s = %d%s",m1,x,m2);
}

void fprintf_ulong(FILE *s, const char *m1, unsigned long x, const char *m2)
{
  fprintf(s,"%s = %lu%s",m1,(unsigned long)x,m2);
}

void fprintf_realnum(FILE *s,const char *m1,double x,const char *m2)
{
  fprintf(s,"%s = %g%s",m1,x,m2);
}

void fprintf_float(FILE *s,const char *m1,double x,const char *m2)
{
  fprintf_realnum(s,m1,x,m2);
}

void fprintf_double(FILE *s,const char *m1,double x,const char *m2)
{
  fprintf_realnum(s,m1,x,m2);
}

void fprintf_bool(FILE *s,const char *m1,bool x,const char *m2)
{
  fprintf(s,"%s = %s%s",m1,(x)?"True":"False",m2);
}

void fprintf_string(FILE *s,const char *m1,const char *x,const char *m2)
{
  fprintf(s,"%s = \"%s\"%s",m1,x,m2);
}

int index_of_char(const char *s,char c)
/*
   Returns the least index i such that s[i] == c.
   If no such index exists in string, returns -1
*/
{
  int result = -1;

  if ( s != NULL )
  {
    int i;
    for ( i = 0 ; result < 0 && s[i] != '\0' ; i++ )
      if ( s[i] == c ) result = i;
  }

  return(result);
}

bool char_is_in(const char *s,char c)
{
  bool result = index_of_char(s,c) >= 0;
  return(result);
}

/* This returns the number of occurences of character c in string s. 
   It is legal for s to be NULL (in which case the result will be 0).
   
   Added by Mary on 8 Dec 96.
*/
int num_of_char_in_string(const char *s, char c)
{
  int res = 0;

  if (s != NULL)
  {
    int i;

    for (i = 0; i < (int) strlen(s); i++)
      if (s[i] == c) res++;
  }
  return(res);
}

FILE *am_fopen(const char *filename,const char *mode)
{
  FILE *s = NULL;
  bool all_whitespace = TRUE;
  int i;

  if ( filename == NULL )
	  my_error("am_fopen: Called with filename == NULL");
  
  for ( i = 0 ; filename[i] != '\0' && all_whitespace ; i++ )
	all_whitespace = filename[i] == ' ';

  if ( !all_whitespace )
	s = fopen(filename,mode);
  return(s);
}

bool is_all_digits(const char *string)
{
        if (!string || !*string)
                return FALSE;  /* null pointer, or pointer to null char */
        while (*string)
                if (!isdigit(*string++))
                        return FALSE;  /* found a non-digit! */
        return TRUE;  /* must all be digits */
}

bool is_a_number(const char *string)
/*
   uses a finite state machine with 8 states.
   These are the symbols in the FSM alphabet:
    S     + or -
    P     .
    D     0 or 1 or .. or 9
    E     e or E
    X     anything else

  In the following descriptions of states, any unmentioned
  symbol goes to an absorbing, non-accepting, error state
       State 1: S --> 2 , D --> 4 , P --> 3
       State 2: P --> 3 , D --> 4 
       State 3: D --> 5
    *  State 4: D --> 4 , E --> 6 , P --> 5
    *  State 5: D --> 5 , E --> 6
       State 6: D --> 8 , S --> 7
       State 7: D --> 8
    *  State 8: D --> 8
  The starred states are acceptors.
*/
{
  int state = 1;
  int i = 0;
  bool err_state = FALSE;
  bool result;

  while ( string[i] != '\0' && !err_state )
  {
    char c = string[i];
    char symbol = ( c == '.' ) ? 'P' :
                  ( c == 'e' || c == 'E' ) ? 'E' :
                  ( c == '+' || c == '-' ) ? 'S' :
                  ( c >= '0' && c <= '9' ) ? 'D' : 'X';
/*
    printf("i = %d , c = %c , symbol = %c , state = %d\n",i,c,symbol,state);
*/
    if ( state == 1 )
    {
      if ( symbol == 'S' ) state = 2;
      else if ( symbol == 'D' ) state = 4;
      else if ( symbol == 'P' ) state = 3;
      else err_state = TRUE;
    }
    else if ( state == 2 )
    {
      if      ( symbol == 'P' ) state = 3 ;
      else if ( symbol == 'D' ) state = 4 ;
      else err_state = TRUE;
    }
    else if ( state == 3 )
    {
      if      ( symbol == 'D' ) state = 5;
      else err_state = TRUE;
    }
    else if ( state == 4 )
    {
      if      ( symbol == 'D' ) state = 4 ;
      else if ( symbol == 'E' ) state = 6 ;
      else if ( symbol == 'P' ) state = 5;
      else err_state = TRUE;
    }
    else if ( state == 5 )
    {
      if      ( symbol == 'D' ) state = 5 ;
      else if ( symbol == 'E' ) state = 6;
      else err_state = TRUE;
    }
    else if ( state == 6 )
    {
      if      ( symbol == 'D' ) state = 8 ;
      else if ( symbol == 'S' ) state = 7;
      else err_state = TRUE;
    }
    else if ( state == 7 )
    {
      if      ( symbol == 'D' ) state = 8;
      else err_state = TRUE;
    }
    else if ( state == 8 )
    {
      if      ( symbol == 'D' ) state = 8;
      else err_state = TRUE;
    }

    i += 1;
  }

  result = !err_state && ( state==4 || state==5 || state==8 );
  return(result);
}

bool bool_from_string(const char *s,bool *r_ok)
{
  bool result = FALSE;
  *r_ok = FALSE;

  if ( s != NULL && s[0] != '\0' )
  {
    char c = s[0];
    if ( c >= 'a' && c <= 'z' ) c += 'A' - 'a';
    if ( c == 'Y' || c == 'T' || c == '1' )
    {
      *r_ok = TRUE;
      result = TRUE;
    }
    if ( c == 'N' || c == 'F' || c == '0' )
    {
      *r_ok = TRUE;
      result = FALSE;
    }
  }

  return(result);
}

/*------------------------------------------------*/
int int_from_string(const char *s,bool *r_ok)
{
  int result;
  if ( is_a_number(s) )
  {
    result = atoi(s);
    *r_ok = TRUE;
  }
  else
  {
    result = 0;
    *r_ok = FALSE;
  }
  return(result);
}

/*------------------------------------------------*/
double double_from_string(const char *s,bool *r_ok)
{
  double result;
  if ( is_a_number(s) )
  {
    result = atof(s);
    *r_ok = TRUE;
  }
  else
  {
    result = 0;
    *r_ok = FALSE;
  }
  return(result);
}

void sensible_limits( double xlo, double xhi, double *res_lo, double *res_hi, double *res_delta )
{
  double scale,rel_hi,rel_delta;

  if ( xlo > xhi )
  {
    double temp = xlo; xlo = xhi; xhi = temp;
  }

  if ( xhi - xlo < 1e-50 )
  {
    double xmid = (xlo + xhi)/2.0;
    xlo = xmid - 1e-50;
    xhi = xmid + 1e-50;
  }
    
  scale = pow(10.0,ceil(-log10(xhi - xlo)));
  rel_hi = scale * (xhi - xlo);

  rel_delta = ( rel_hi < 1.5 ) ? 0.2 :
              ( rel_hi < 2.5 ) ? 0.5 :
              ( rel_hi < 5.0 ) ? 1.0 :
              2.0;

  *res_delta = rel_delta / scale;
  *res_lo = *res_delta * floor(xlo / *res_delta);
  *res_hi = *res_delta * ceil(xhi / *res_delta);
}

#if 0
int next_highest_power_of_two(int n)
{
  int result;

  if (n == 0)
    return (0);
  result = 1;
  n--;
  while (n)
  {
    result <<= 1;
    n >>= 1;
  }
  return result;
}
#endif

#if 1
/* This version is about 3 times as fast.  -jab  */
int next_highest_power_of_two(int n)
{
  int result;

  if (n<=0) return 0;
  if (n<=1) return 1;
  if (n<=2) return 2;
  if (n<=4) return 4;
  if (n<=8) return 8;
  if (n<=16) return 16;
  if (n<=32) return 32;
  if (n<=64) return 64;
  if (n<=128) return 128;
  if (n<=256) return 256;
  if (n<=512) return 512;
  if (n<=1024) return 1024;
  if (n<=2048) return 2048;
  if (n<=4096) return 4096;
  if (n<=8192) return 8192;
  if (n<=16384) return 16384;
  if (n<=32768) return 32768;

  result = 65536;
  n--;
  n >>= 16;
  
  while (n)
  {
    result <<= 1;
    n >>= 1;
  }
  return result;
}
#endif

double roundest_number_between(double lo,double hi)
{
  double d = hi - lo;
  double result = -77777.7;
  if ( d < 0.0 )
    my_error("iouadbciusabc");
  else if ( d < 1e-20 )
    result = (lo + hi)/2;
  else
  {
    double dist = 1.0;
    double factors[] = { 0.5 , 0.2 , 0.1 , 0.05 };
    int num_factors = sizeof(factors)/sizeof(double);
    int i;
    bool ok = FALSE;
    lo += 0.001 * d;
    hi -= 0.001 * d;
    while ( dist > d )
      dist *= 0.1;
    while ( dist < d )
      dist *= 10.0;
    /* dist is now a power of ten between 1 and 10 times the size of d */   
    for ( i = 0 ; !ok && i < num_factors ; i++ )
    {
      double scale = dist * factors[i];
      double a1 = (lo + hi)/2;
      double a2 = a1 / scale;
      double a3 = floor(0.5 + a2);
      double a4 = a3 * scale;
      if ( a4 >= lo && a4 <= hi )
      {
        ok = TRUE;
        result = a4;
      }
    }

    if ( !ok ) my_error("Logically MUST be okay");
  }
  return result;
}

bool doubles_very_close(double a,double b)
{
  bool result;

  if ( a == b ) 
    result = TRUE;
  else if ( a == 0.0 )
    result = fabs(b) < 1e-10;
  else if ( b == 0.0 )
    result = fabs(a) < 1e-10;
  else if ( a < 0.0 && b > 0.0 )
    result = fabs(a) < 1e-8 && fabs(b) < 1e-8;
  else if ( a > 0.0 && b < 0.0 )
    result = fabs(a) < 1e-8 && fabs(b) < 1e-8;
  else
  {
    double x = real_min(fabs(a),fabs(b));
    double y = real_max(fabs(a),fabs(b));
    double eps = 1e-7;
    /* result = (y - x)/y < eps */
    result = y - x < y * eps;
  }

  return result;
}

char *am_mktemp( char pattern[])
{
  /* Mimics glibc version.  This means that it overwrites the pattern
     string.  I'm re-implementing this version because it is the most
     restrictive.  From the man page at the time of writing:
  
  The  mktemp()  function  generates a unique temporary file
  name from pattern.  The last six characters  of  pattern
  must  be  XXXXXX and these are replaced with a string that
  makes the filename unique. Since it will be modified, tem-
  plate  must  not  be  a  string  constant,  but  should be
  declared as a character array.

  RETURN VALUE
  The mktemp() function returns NULL on error (pattern  did
  not  end  in  XXXXXX) and pattern otherwise.  If the call
  was successful, the last six bytes of pattern  will  have
  been  modified  in  such  a way that the resulting name is
  unique (does not exist already). If the  call  was  unsuc-
  cessful, pattern is made an empty string.

  */

  /* Strategy: simply loop over all 6-character alphanumeric strings,
     stopping at the first available string.

     numbers:    48-57       10
     upper-case: 65-90       26
     lower-case: 97-122      26
                            ---
                             62
  */

  int i, maxtries, val;
  char charmap[62], *pos, *posend;
  FILE *f;

  /* Initialize random number generator and set maxtries.  Use a random
     number of tries to reduce the posibility of trying all remaining
     options (which would be predictable. */
  srand( time(NULL));
  maxtries = 100 + 1000 * (int)(rand() / (RAND_MAX+1.0));

  /* Create charmap. */
  for (i=0; i<10; ++i) charmap[i] = i+48;
  for (i=10; i<36; ++i) charmap[i] = i+55;
  for (i=36; i<62; ++i) charmap[i] = i+61;

  /* Check that pattern has exactly six X chars at the end. */
  posend = pattern + strlen( pattern); /* pos points to end of string. */
  pos = posend - 7;
  if (*pos++ == 'X') return NULL;
  while (pos < posend) if (*pos++ != 'X') return NULL;

  /* Try random filenames.  Use fopen() to test for existence. */
  for (i=0; i<maxtries; ++i) {
    pos = posend - 6;    
    while (pos < posend) {
      val = (int) (62.0 * (rand() / (RAND_MAX+1.0)));
      *pos++ = charmap[val];;
    }
    f = fopen( pattern, "r");
    if (f == NULL && errno == ENOENT) return pattern;
    else if (f != NULL) fclose( f);
  }

  return NULL;
}

/*call this to find a place to create temp files.  
Example of cross platform code to generate a temp file name:
  char *tmp_file;
  char *tmp_dir;

  tmp_dir = mk_valid_temp_dir_string();
  tmp_file = mk_join_path2(tmp_dir, "foo.XXXXXX");
  am_mktemp(tmp_file);

  Now tmp_file should be a valid unique filename.
*/
char *mk_valid_temp_dir_string(void)
{
    char *ret = NULL;

#if defined PC_PLATFORM
    char *tmp_env = getenv("TMP"); /*will usually be set on windows*/
    if (tmp_env && am_isdir(tmp_env))
    {
	ret = mk_copy_string(tmp_env);
    }
#else
    if (am_isdir("/tmp"))
    {
	ret = mk_copy_string("/tmp");
    }
#endif
    if (!ret)
    {
	/*use the current directory*/
	ret = mk_getcwd();
    }

    return ret;
}

/*convenience function for generating a valid tmp filename*/
char *mk_unique_tmpfile_path(const char *filename_prefix)
{
  char *tmp_file;
  char *tmp_dir;
  char *name;

  /*get a valid place to put the temp file*/
  tmp_dir = mk_valid_temp_dir_string();

  /*generate the temp file name*/
  name = mk_printf("%s.XXXXXX", filename_prefix);
  tmp_file = mk_join_path2(tmp_dir, name);
  am_mktemp(tmp_file);

  /*cleanup*/
  free_string(tmp_dir);
  free_string(name);

  return tmp_file;
}



/* Returns TRUE if and only if x is NaN or Inf (i.e. returns FALSE
   if and only if x is a completely legal number) */         
bool is_ill_defined(double x)
{
  return am_isnan(x);
}

/* Returns TRUE iff n is a power of two. Note that 0 is not defined
   to be a power of two. */
bool is_power_of_two(int x)
{
  bool result = x > 0;
  while ( result && x > 1 )
  {
    result = (x & 1) == 0;
    x = x >> 1;
  }
  return result;
}

/*------------------ user interaction code ------------------------*/

/* by default the uimode is kUI_Shell (interact with the user at the
   command shell).*/
void Global_set_ui_mode(ui_mode mode)
{
  am_internal_state_t *state_ptr = am_get_internal_state();
  if (mode >= 0 && mode < kNumUIModes)
    {
      state_ptr->user_interaction_mode = mode;
    }
  else
    {
      my_printf_stderr("Error in set_ui_mode(%d).  %d is an invalid "
		       "mode number.\n", mode, mode);
    }
}

int Global_get_ui_mode(void)
{
  am_internal_state_t *state_ptr = am_get_internal_state();
  return state_ptr->user_interaction_mode;
}

/* -------------------*/
/* ---------------Wait for key code-------------------- */

/*main functions - called by outside world*/
void wait_for_key(void)
{
    if ( Ignore_next_n > 0 )
	Ignore_next_n -= 1;
    else 
	do_wait_for_key(FALSE);
}

void wait_for_key_if_verbose(void)
{
  if (Global_get_verbosity() >= 5)
    wait_for_key();
}

void really_wait_for_key(void)
{ 
    do_wait_for_key(TRUE);
}

/*decide how to handle the wait*/
void do_wait_for_key(bool really_wait)
{
    if ( really_wait || Verbosity >= 0.0 ) 
    {
	switch (Global_get_ui_mode())
	{
	  case kUI_Shell:
	      wait_for_key_shell(really_wait);
	      break;
	  case kUI_NoInput:
	      my_printf(1,"skipping wait_for_key in batch mode.\n");
	      break;
	  default:
	      break; /*should not get here!*/
	}
    }
}

/*kUI_Shell (text based) wait method */
void wait_for_key_shell(bool really_wait)
{
  int c = ' ';
  int n = 0;

  /*instructions to user*/
  if (really_wait)
      my_printf(0,"really_wait_for_key(): [Return to continue or q to quit]");
  else
      my_printf(0,"wait_for_key(): [Return to continue or h for other options]\n");

  /*get user input */
  while ( c != '\n' )
  {
      c = getchar();

      if (!really_wait)
      {
	  /*process menu choice (really_wait has no menu)*/

	  /*num waitpoints to skip*/
	  if ( c >= '0' && c <= '9' )
	      n = 10 * n + c - '0';
	  /*breakpoint*/
	  else if ( c == 'b' || c == 'B' )
	  {
	      my_breakpoint();
	  }
	  /*verbosity*/
	  else if ( c == 'v' || c == 'V' ) 
	  {
	      char buff[100];
	      int i = 0;
	      c = getchar();
	      while ( i < 99 && c != '\n')
	      {
		  buff[i] = c;
		  buff[i+1] = '\0';
		  i += 1;
		  c = getchar();
	      }

	      my_printf(0,"Old Verbosity: %g\n",Verbosity);
	      Verbosity = atof(buff);
	      my_printf(0,"New Verbosity: %g\n",Verbosity);
	  }
	  /*help*/
	  else if ( c == 'h' || c == 'H' )
	  {
	      my_printf(0,"Enter:\n");
	      my_printf(0,"   <Return>          To continue past this wait point.\n");
	      my_printf(0,"   <number> <Return> To continue past this and the next\n");
	      my_printf(0,"                     <number> wait points.\n");
	      my_printf(0,"   q (or Q)          To exit the program entirely.\n");
	      my_printf(0,"   b (or B)          Call my_breakpoint\n");
	      my_printf(0,"   h (or H)          To read this help menu.\n");
	      my_printf(0,"   v <number>        To change the Verbosity level. The\n");
	      my_printf(0,"                     higher the verbosity level the more\n");
	      my_printf(0,"                     information will be printed on your\n");
	      my_printf(0,"                     terminal (and the more wait points).\n");

	      while ( c != '\n') c = getchar();
	      c = ' ';
	  }
      }
      if ( c == 'q' || c == 'Q' )
      {
	  my_printf(0,"You hit Q at a wait point. Exiting program\n");
	  exit(0);
      }
  }

  /* Andrew writes: I want to still fast forward through ordinary
     wait_for_key()'s even if I've asked to skip some but I'm at a
     really_wait_for_key. So I only want to overwrite n if at a
     regular wait. I have a feeling someone wanted things the other
     way, so feel free to debate this crucial point with me!! */
  if ( !really_wait )
    Ignore_next_n = n;

}


/* ---------------End wait for key code-------------------- */

/* -------------- Get user input code --------------------- */
char *mk_get_user_input(const char *message, const char *def_reply)
{
    char *reply = NULL;

    if (!message) my_errorf("mk_get_user_input called with message = NULL.\n");
    if (!def_reply) my_errorf("mk_get_user_input called with def_reply = NULL.\n");

    switch (Global_get_ui_mode())
    {
      case kUI_Shell:
	  reply = get_user_input_shell(message, def_reply);
	  break;
      case kUI_NoInput:
	  my_printf(1,"skipping mk_get_user_input in batch mode.\n");
	  reply = mk_copy_string(def_reply);
	  break;
      default:
	  break; /*should not get here!*/
    }
    return reply;
}

char *get_user_input_shell(const char *message, const char *def_reply)
{
    printf("%s> ", message);
    return mk_string_from_line(stdin);
} 


/* -------------- End get user input code ----------------- */

/* -------------- buftab stuff used by both fprintf_dyv and fprintf_dym  ----------------- */
/* (was static in amdym.c) */

char *bufstr(buftab *bt,int i,int j)
{
  char *result;

  if ( i < 0 || i >= bt->rows || j < 0 || j >= bt->cols )
  {
    result = NULL;
    my_error("bufstr()");
  }
  else
    result = bt->strings[i][j];

  if ( result == NULL )
    result = "-";

  return(result);
}

void fprint_buftab( FILE *s, buftab *bt)
{
  int *widths = AM_MALLOC_ARRAY(int,bt->cols);
  int i,j;
  set_ints_constant(widths,bt->cols,0);

  for ( i = 0 ; i < bt->rows ; i++ )
    for ( j = 0 ; j < bt->cols ; j++ )
      widths[j] = int_max(widths[j],strlen(bufstr(bt,i,j)));

  for ( i = 0 ; i < bt->rows ; i++ )
    for ( j = 0 ; j < bt->cols ; j++ )
    {
      char ford[20];
      sprintf(ford,"%%%ds%s",widths[j],(j==bt->cols-1) ? "\n" : " ");
      fprintf(s,ford,bufstr(bt,i,j));
    }

  AM_FREE_ARRAY(widths,int,bt->cols);
}

void init_buftab( buftab *bt, int rows, int cols)
{
  if ( rows < 0 || cols < 0 )
    my_error("init_buftab()");
  else
  {
    int i,j;
    bt -> rows = rows;
    bt -> cols = cols;
    bt -> strings = AM_MALLOC_ARRAY(char_ptr_ptr,rows);
    for ( i = 0 ; i < rows ; i++ )
      bt->strings[i] = AM_MALLOC_ARRAY(char_ptr,cols);
    for ( i = 0 ; i < rows ; i++ )
      for ( j = 0 ; j < cols ; j++ )
        bt->strings[i][j] = NULL;
  }
}

void free_buftab_contents(buftab *bt)
{
  int i,j;
  for ( i = 0 ; i < bt->rows ; i++ )
    for ( j = 0 ; j < bt->cols ; j++ )
      if ( bt->strings[i][j] != NULL )
        AM_FREE_ARRAY(bt->strings[i][j],char, strlen(bt->strings[i][j]) + 1);

  for ( i = 0 ; i < bt->rows ; i++ )
    AM_FREE_ARRAY(bt->strings[i],char_ptr,bt->cols);
    
  AM_FREE_ARRAY(bt->strings,char_ptr_ptr,bt->rows);
}

void set_buftab( buftab *bt, int i, int j, const char *str)
{
  if ( i < 0 || i >= bt->rows || j < 0 || j >= bt->cols )
    my_error("set_buftab()");
  else if ( bt->strings[i][j] != NULL )
    my_error("set_buftab: non null string");
  else
    bt->strings[i][j] = make_copy_string(str);
}

/*-----------------------------------------------------------------*/

#ifdef PC_PLATFORM
#define vsnprintf _vsnprintf
#endif


int safe_print_varargs_to_buf(char *buf, size_t size, 
			      const char *format, va_list ap)
{
  int vsnprintf_ret=0;
  buf[0]= '\0';
  if (format){
    int vsnprintf_ret = vsnprintf(buf, size, format, ap);
    if (vsnprintf_ret >= 0 && vsnprintf_ret < size)
      buf[vsnprintf_ret] = '\0';			
    else 
      buf[size-1] = '\0'; 
    
    if (vsnprintf_ret < 0 || vsnprintf_ret >= size)			
      fprintf(stderr, "safe_print_varargs_to_buf - error or unable to write " 
	      "all data.\nOur array size is %u and vsnprintf returned %d.\n" 
	      "What we have so far is: \n'%s'\n", (unsigned)size,	
	      vsnprintf_ret, buf);			
  }
  return vsnprintf_ret;
}

/*-----------------------------------------------------------------*/




