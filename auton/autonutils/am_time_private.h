/* am_time_private.h 

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
/* No #ifdef protection, because it should only be included from one place anyhow */

  /* itimer stuff checks once an hour */
#define AUTON_IT_CONSTANT (60*60)





#ifdef UNIX_PLATFORM
struct itimer_state
{
  int signal_used; /* Non-active data, knows what signal is used for this itimer*/
  int iter_counter; /* Active data, holds number of iterations we've gone thr*/
  struct itimerval itim;
  char name[12];
  int active; /* 1 = inactive, 2 = active. 0 = try to catch uninitialized.*/
};

static void itimer_sig_hdlr(int sig_id);
static int sigid_to_index(int sig_id);
static int itimet_to_index(int itimer_type);

#endif /* UNIX_PLATFORM */


static void void_free_date(void* data);
static void *void_mk_copy_date(void* data);
static void void_fprintf_date(FILE* s, char* m1, void* data, char* m2);

