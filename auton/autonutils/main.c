#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#ifdef USE_PTHREADS
#include <pthread.h>
#endif

#include "utils.h"
#include "am_time.h"

void test_time(void);

void test_memleak( int argc, char *argv[])
{
  char *s;
  memory_leak_check_args( argc, argv);
  s = mk_copy_string( "Hello, world.\n");
  printf( "We have created a memleak.  Calling am_malloc_report_polite().\n");
  am_malloc_report_polite();
  return;
}

void test_rusage( void)
{
  struct timeval start_time, end_time;
  int i;

  start_time = rusage_time();
  for (i = 0; i < 100; i++)
  {
    int max = 20000;
    ivec *iv = mk_random_ivec(max, 0, max);
    ivec *new_iv = mk_ivec_sort(iv);
    free_ivec(iv);
    free_ivec(new_iv);
  }

  end_time = rusage_time();

  fprintf(stdout, "##TIME %f\n",
	  1.0*end_time.tv_sec - start_time.tv_sec
	  + 1e-6*(end_time.tv_usec - start_time.tv_usec));
}

void test_corrupt( void)
{
  int i, size, tmp, a1tagval, a2tagval;
  int bytes_per_tag;
  char *a1, *a2, *a1tag, *a2tag;

  bytes_per_tag = 8;

  /* Initial AM_MALLOCs just to bump tag counts up. */
  for (i=0; i<2048; ++i) {
    a1 = AM_MALLOC( char);
    AM_FREE( a1, char);
  }

  /* Allocate. */
  size = 4;
  a1 = AM_MALLOC_ARRAY( char, size);
  a1tag = a1 - bytes_per_tag;
  a1tagval =
    (a1tag[0] & 0xFF) << 24 |
    (a1tag[1] & 0xFF) << 16 |
    (a1tag[2] & 0xFF) <<  8 |
    (a1tag[3] & 0xFF);
  for (i=0; i<size; ++i) a1[i] = '\0';

  a2 = AM_MALLOC_ARRAY( char, size);
  a2tag = a2 - bytes_per_tag;
  a2tagval =
    (a2tag[0] & 0xFF) << 24 |
    (a2tag[1] & 0xFF) << 16 |
    (a2tag[2] & 0xFF) <<  8 |
    (a2tag[3] & 0xFF);
  for (i=0; i<size; ++i) a2[i] = '\0';

  /* Print summary info. */
  printf( "a1 tag: %d\n", a1tagval);
  printf( "a2 tag: %d\n", a2tagval);
  printf( "a1:%p, a2:%p, a2-a1:%ld\n",
          (void *) a1, (void *) a2, (long int) (a2-a1));
  fflush( stdout);
  if (a2-a1 < 2) my_error( "test_corrupt: a2 and a1 are too close.");

  /* Set a1 and a2 vals. */
  printf( "UNCORRUPTED MEMORY\n");
  for (i=0; i<size-1; ++i) a1[i] = 'A' + (char) i;
  for (i=0; i<size-1; ++i) a2[i] = 'a' + (char) i;
  printf( "a1: '%s'\n", a1);
  printf( "a2: '%s'\n\n", a2);
  fflush( stdout);
  if (am_check_free_space()) {
    printf( "(not terminating on am_check_free_space().\n\n");
  }
  if (am_check_heap()) printf( "(not terminating on am_check_heap().\n\n");


  /* Invisible corruption -- destroys \0 and maybe the next char.
     We are likely to get a segfault, but AM_FREE_ARRAY won't likely
     complain. */
  printf( "SLIGHTLY CORRUPTED MEMORY\n");
  fflush( stdout);
  tmp = long_min( size+1, a2tag-a1);
  for (i=size-1; i < tmp; ++i) a1[i] = 'X';
  printf( "a1: '%s'\n", a1);
  printf( "a2: '%s'\n\n", a2);
  fflush( stdout);
  if (am_check_heap()) printf( "(not terminating on am_check_heap().\n\n");

  /* If t=verycareful, this free will trigger a fatal heap check. */
  printf( "Calling AM_FREE -- if t=verycareful, then this program should "
          "terminate.\n\n");
  fflush( stdout);
  AM_FREE( AM_MALLOC( char), char);

  /* This corruption might destroy something, or it might not.
     It doesn't overwrite the tag, and AM_FREE_ARRAY() might succeed. */
  printf( "SLIGHTLY MORE CORRUPTED MEMORY -- CORRUPTED NEIGHBORS PRE-TAG\n");
  for (i=tmp; i < a2tag-a1; ++i) a1[i] = 'Y';
  printf( "a1: '%s'\n", a1);
  printf( "a2: '%s'\n\n", a2);
  fflush( stdout);
  if (am_check_heap()) printf( "(not terminating on am_check_heap().\n\n");

  /* Now we corrupt part of the user-visible a2.  AM_FREE_ARRAY will 
     almost certainly complain. */
  printf( "REALLY CORRUPTED MEMORY -- CLOBBERED NEIGHBOR'S DATA\n");
  for (i=a2tag-a1; i <= a2-a1; ++i) a1[i] = 'Z';
  printf( "a1: '%s'\n", a1);
  printf( "a2: '%s'\n\n", a2);
  fflush( stdout);
  if (am_check_heap()) printf( "(not terminating on am_check_heap().\n\n");


  AM_FREE_ARRAY( a2, char, size);
  AM_FREE_ARRAY( a1, char, size);

  /* Check for leaks. */
  am_malloc_report_polite();

  return;
}

ivec_array *mk_intersect_test_array(int size,int maxval,int n)
{
  ivec_array *iva = mk_empty_ivec_array();
  int i;

  printf("Begin making test_array...\n");

  for ( i = 0 ; i < n ; i++ )
  {
    ivec *a = mk_identity_ivec(maxval);
    ivec *s;
    shuffle_ivec(a);
    while ( ivec_size(a) > size )
      ivec_remove_last_element(a);
    s = mk_sivec_from_ivec(a);
    add_to_ivec_array(iva,s);
    free_ivec(a);
    free_ivec(s);
  }

  printf("...end making test_array\n\n");

  return iva;
}

int intersect_main_given_asize(int argc,char *argv[],int asize,int n)
{
  int amax = int_from_args("amax",argc,argv,10000);
  int bsize = int_from_args("bsize",argc,argv,5000);
  int bmax = int_from_args("bmax",argc,argv,10000);
  ivec_array *a = mk_intersect_test_array(asize,amax,n);
  ivec_array *b = mk_intersect_test_array(bsize,bmax,n);
  int i,j;
  int sum_int_lengths = 0;
  int start_time = global_time();

  for ( i = 0 ; i < n ; i++ )
    for ( j = 0 ; j < n ; j++ )
    {
      ivec *ai = ivec_array_ref(a,i);
      ivec *bj = ivec_array_ref(b,j);
      ivec *intersection = mk_sivec_intersection(ai,bj);
      /*
      fprintf_ivec(stdout,"a",ai,"\n");
      fprintf_ivec(stdout,"b",bj,"\n");
      fprintf_ivec(stdout,"intersection",intersection,"\n");
      wait_for_key();
      */

      sum_int_lengths += ivec_size(intersection);

      free_ivec(intersection);
    }

  printf("sum_int_lengths = %d\nThat took %d sec(s)\n",
	 sum_int_lengths,global_time() - start_time);

  free_ivec_array(a);
  free_ivec_array(b);

  return global_time() - start_time;
}

void intersect_main(int argc,char *argv[])
{
  ivec *a = mk_ivec_6(1,2,3,6,7,8);
  ivec *b = mk_ivec_6(-1,2,3,5,6,9);
  ivec *i = mk_sivec_intersection(a,b);

  {
    int s1 = intersect_main_given_asize(argc,argv,10,700);
    int s2 = intersect_main_given_asize(argc,argv,100,600);
    int s3 = intersect_main_given_asize(argc,argv,1000,500);
    int s4 = intersect_main_given_asize(argc,argv,10000,250);

    printf("(asize : secs) results:\n");

    printf("(10,%d) (100,%d) (1000,%d) (10000,%d)\n",s1,s2,s3,s4);
  }

  free_ivec(a);
  free_ivec(b);
  free_ivec(i);
}

void test_mkdir(void)
{
#ifdef PC_PLATFORM
am_mkdir("here\\there");
printf("Look in here\\there\n");
#else
am_mkdir_deeply("here/there");
printf("Look for here/there\n");
am_mkdir_deeply("/tmp/alpha/beta");
printf("Look for /tmp for alpha/beta\n");
am_mkdir_deeply("somewhere");
printf("Look for somewhere\n");
#endif
if(am_isdir("here"))
  printf("directory 'here' exists\n");
if(am_isdir("shere"))
  printf("directory 'shere' exists\n");
if(am_isfile("main.c"))
  printf("file main.c exists\n");
if(am_isfile("goooooo.c"))
  printf("file gooooooo.c exists\n");
}

void test_mktemp( void)
{
  char pattern[500], *foo;
  sprintf( pattern, "/tmp/abcXXXXXX");
  printf( "\npattern = %s\n", pattern);
  foo = am_mktemp( pattern);
  if (foo == NULL) printf( "Unable to create unused file name %s.\n",pattern);
  else printf( "Created name %s.\n", pattern);
  return;
}

void test_backtrace_sigusr2( void)
{
  backtrace_install_sigusr2_handler();
  while (1) {
    fprintf( stderr, "Waiting 5 seconds for SIGUSR2 (press ctrl-c to exit)\n");
    am_sleep(5);
  }
}

void test_backtrace_sigsegv( void)
{
  backtrace_install_sigsegv_handler();
  fprintf( stderr, "\n\nAbout to invoke fprintf() on (FILE *) NULL\n");
  fprintf( NULL, "foo\n");
}

void test_backtrace( void)
{
  /* Getting backtrace info directly and printing. */
  btinfo *bt;
  bt = mk_btinfo( 15);  /* Parameter is the max number of stack levels to
			   print. */
  fprintf_btinfo( stdout, "\n", bt, "\n");
  free_btinfo( bt);

  /* Triggering a backtrace through a call to my_breakpoint() via
     my_error(). */
  printf( "******************************************************\n");
  printf( "*                                                    *\n");
  printf( "* Now trying call to my_error; this should be fatal. *\n");
  printf( "*                                                    *\n");
  printf( "******************************************************\n\n");
  my_error( "Don't Panic.\n");
  return;
}

void mdp(void)
{
  double r1 = 4.0;
  double r2 = -1.0;
  double r4 = 2.0;
  double v1 = 0.0;
  double v2 = 0.0;
  double v3 = 0.0;
  double v4 = 0.0;
  while ( TRUE )
  {
    double q1u = r1 + v2;
    double q1d = r1 + 0.5 * v4;
    double q2u = r2 + 0.3333333333333 * (v1 + v2);
    double q2d = r2 + 0.5 * v4;
    double q4u = r4 + v2;
    double q4d = r4 + 0.5 * (v4 + v2);

    printf("%g %g %g %g\n",v1,v2,v3,v4);

    v1 = real_max(q1u,q1d);
    v2 = real_max(q2u,q2d);
    v4 = real_max(q4u,q4d);

    wait_for_key();
  }
}

dyv *mk_bootstrap_dyv(dyv *x)
{
  int size = dyv_size(x);
  dyv *b = mk_dyv(size);
  int i;
  for ( i = 0 ; i < size ; i++ )
     dyv_set(b,i,dyv_ref(x,int_random(size)));
  return b;
}

dyv *mk_random_dyv_in_unit_cube(int size)
{
  dyv *x = mk_dyv(size);
  int i;
  for ( i = 0 ; i < size ; i++ )
    dyv_set(x,i,range_random(0.0,1.0));
  return x;
}

int num_bootstraps_above_zero(int num_cats,int num_bootstraps)
{
  dyv *x1 = mk_random_dyv_in_unit_cube(num_cats);
  dyv *x2 = mk_random_dyv_in_unit_cube(num_cats);
  dyv *diff = mk_dyv_subtract(x1,x2);
  int i;
  int result = 0;

  for ( i = 0 ; i < num_bootstraps ; i++ )
  {
    dyv *boot = mk_bootstrap_dyv(diff);
    if ( dyv_mean(boot) > 0.1 )
      result += 1;
    free_dyv(boot);
  }

  free_dyv(diff);
  free_dyv(x1);
  free_dyv(x2);
  return result;
}

void boot_main(int argc,char *argv[])
{
  int num_trials = int_from_args("num_trials",argc,argv,300);
  int num_bootstraps = int_from_args("num_bootstraps",argc,argv,5000);
  int num_cats = int_from_args("num_cats",argc,argv,200);
  int i;
  for ( i = 0 ; i < num_trials ; i++ )
    printf("%9.6f\n",num_bootstraps_above_zero(num_cats,num_bootstraps) /
                     (double) num_bootstraps);
}


/* 
   this function no longer prints internal amma variables
   because they have been made static in amma.c
*/
void test_malloc(int argc,char *argv[])
{
  int size = int_from_args("size",argc,argv,100);
  int delta = int_from_args("delta",argc,argv,1);
  int i;
  int ivec_size = 0;
  /*
  int next_n = 1;
  int next_t = 1;
  extern int Num_mallocs;
  extern int Total_mallocked;
  */
  int loops = int_from_args("loops",argc,argv,1);
  int j;

  for ( j = 0 ; j < loops ; j++ )
  {
    ivec_array *iva = mk_empty_ivec_array();
    for ( i = 0 ; i < size ; i++ )
    {
      ivec *iv = mk_identity_ivec(ivec_size);
      /*
      bool pn = TRUE;
      bool pt = TRUE;
      */

      add_to_ivec_array(iva,iv);
      free_ivec(iv);
      ivec_size += delta;

      /*
      while ( Num_mallocs >= next_n && next_n < 1000000000 )
      {
	if ( pn ) printf("Num_mallocs = %d\n",Num_mallocs);
	next_n *= 2;
	pn = FALSE;
      }

      while ( Total_mallocked >= next_t  && next_t < 1000000000 )
      {
	if ( pt ) printf("Total_mallocked = %d\n",Total_mallocked);
	next_t *= 2;
	pt = FALSE;
      }
      */
    }
    free_ivec_array(iva);
  }

}

void test_time(void)
{
  printf("Date: %s\n", curr_date());
}

void testthis( void)
{
  int a;
  volatile int b;
  b = 0;
  for (a=0; a<10000; ++a) b += a;
  return;
}

void test_itimer_benchmark( void)
{
  double T;
  T = itimer_benchmark( ITIMER_PROF, 0.02, testthis);
  printf( "T: %g usecs with relative error less than 2%%\n", T);
  return;
}

int test_logfile(int argc, char *argv[])
{
  /* add log message w/o setting the logfilename */
  printf("No logfile name yet.  Try adding a message.\n");
  my_logmsg("You shouldn't see this unless you set AM_LOGFILE\n");

  /* test setting an invalid logfile name */
  if (0)
  {
    char *really_long_str = "/tmp/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.txt";
    
    printf("Setting the logfile name to a really really long string.\n");
    if (! set_logfile_name(really_long_str)){
      printf(" Nope.  That didn't work.\n");
    }
  }
  /* set the logfile name */
  printf("Setting the logfile name to '/tmp/out.txt'\n");
  if (set_logfile_name("/tmp/out.txt"))
  {
    printf(" the name was set successfully!\n");
  }

  /* call my_logmsg directly */
  printf("Testing my_logmsg...\n");
  my_logmsg("A message with no \\n: Hello %s", "World");
  my_logmsg("A message with a \\n: Hello %s\n", "World");
  
  /* an info message */
  printf("Testing my_info...\n");
  my_info("This info is for you!");
  
  /* a warning */
  printf("Testing my_warning...\n");
  my_warning("Hey you - watch out!");
  my_warning_no_wait("call warning no wait.");

  /* an error */
  printf("Testing my_error...\n");
  my_error("Now you've really done it!");

  return 0;
}

#ifdef USE_PTHREADS
static void *thread_routine (void *arg)
{
  printf("Thread %d running...\n", pthread_self());
  return arg;
}

static int test_threads(int argc, char* argv[])
{
  pthread_t thread_id;
  void *thread_result;
  int status;

  printf("Thread %d running...\n", pthread_self());

  printf("doing pthread_create\n");
  status = pthread_create(&thread_id, NULL, thread_routine, NULL);
  if (status != 0) {
    my_error("Error doing pthread_create");
  }

  printf("doing pthread_join\n");
  status = pthread_join(thread_id, &thread_result);
  if (status != 0) {
    my_error("Error doing pthread_join");
  }

  if (thread_result == NULL) {
    return 0;
  } else {
    return 1;
  }
}
#endif


int main(int argc, char* argv[])
{
  prepare_timers();
  start_itimer(ITIMER_VIRTUAL);
  if (0){
    printf("sqrt(2 * PI) = %20.18f\n",sqrt(2 * PI));
    printf("log(2 * PI) = %20.18f\n",log(2 * PI));
    printf("log(sqrt(2 * PI)) = %20.18f\n",0.5 * log(2 * PI));
  }
  if ((argc > 1) && (eq_string(argv[1],"boot")))
    boot_main(argc,argv);
  else if ((argc > 1) && (eq_string(argv[1],"intersect")))
    intersect_main(argc,argv);
  else if ((argc > 1) && (eq_string(argv[1],"rusage")))
    test_rusage();
  else if ((argc > 1) && (eq_string(argv[1],"mdp")))
    mdp();
  else if ((argc > 1) && (eq_string(argv[1],"malloc")))
  {
    test_malloc(argc,argv);
    am_malloc_report();
    wait_for_key();
  }
  else if ((argc > 1) && (eq_string(argv[1],"backtrace")))
    test_backtrace();
  else if ((argc > 1) && (eq_string(argv[1],"btsegv")))
    test_backtrace_sigsegv();
  else if ((argc > 1) && (eq_string(argv[1],"btusr2")))
    test_backtrace_sigusr2();
  else if ((argc > 1) && (eq_string(argv[1],"mktemp")))
    test_mktemp();
  else if ((argc > 1) && (eq_string(argv[1],"time")))
    test_time();
  else if ((argc > 1) && (eq_string(argv[1],"mkdir")))
    test_mkdir();
  else if ((argc > 1) && (eq_string(argv[1],"ibench")))
    test_itimer_benchmark();
  else if ((argc > 1) && (eq_string(argv[1],"logfile")))
    test_logfile(argc, argv);
#ifdef USE_PTHREADS
  else if ((argc > 1) && (eq_string(argv[1],"threads")))
    test_threads(argc, argv);
#endif

  else
  {
    printf("Nothing to see here, move along, move along [%f]...\n",
	   stop_itimer(ITIMER_VIRTUAL));
  }
  return 0;
}

