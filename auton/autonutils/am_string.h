/** @file am_string.h
    @brief String and character related functions.
@verbatim
   File:         am_string.h
   Author:       Andrew Moore, Pat Gunn
   Created:      20 Feb 2003 from amstr.h by Andrew Moore/Frank Dellaert
   Description:  String and character related functions.
   Copyright (C) Andrew W. Moore, 1992
@endverbatim
   This used to be amstr.h, but in a very large cleanup, it absorbed a
   lot of misplaced other functions, in the move for better code
   organization standards. Note that there are still a few string
   functions in amdmex.h. Some of them are esoteric enough that they'll stay
   there, others really belong here.

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

#ifndef AMSTR_H
#define AMSTR_H

#include "standard.h"
#include "ambs.h"
#include "auton_sf.h"

/** Returns first line of a multiline string. Assumes native-format strings
   with regards to newlines. */
char* mk_first_line_of_string(char* src);

/** Makes a copy of the provided string, or if you pass a NULL, will pass it
   back. */
char *mk_copy_string(const char *s);
char *mk_copy_string_n(const char *s, int n); /**copy first n chars*/
/** Depreciated alternate name. Will be removed someday. */
char *make_copy_string(const char *s);

char* mk_copy_string_nullok(char* src);

/** s is given in a possibly quoted format.  all delimiting quotes are removed.
   all backslash specified characters are replaced with their appropriate
   single characters.  There is one additional special case.  The string
   NULL is converted to the NULL pointer.  If the actual string NULL is wanted
   it should be specified as 'NULL' or "NULL"

   Modified 1-31-96 JS: single quotes are no longer valid delimiters */
char *mk_copy_quoted_string(const char *s);
char *mk_copy_to_quoted_string(const char *s);


/** makes and returns a string that is the concatenation of s1 and s2.  
   If free_inputs is TRUE, frees s1 and s2 before returning.*/
char *mk_strcat(const char *s1, const char *s2, bool free_inputs);

/** This function returns a string which is forced to have
   the given extension.  If the string already has the
   extension, the string is copied and returned.  If it
   does not have the extension, then that extension is
   added without any other modification to the string
   or other extensions it may already have.  The extension
   should be given without the ".", and the dot will be
   added if the extension is added.  */
char *mk_string_extension(const char *s, const char *ext);

char *mk_input_string(const char *message);

char *mk_downcase_string(const char *s);
char *mk_upcase_string(const char *s);
bool caseless_eq_string(const char *s1, const char *s2);

char *mk_downcase_string_with_length(const char *s,int n);
char *mk_upcase_string_with_length(const char *s,int n);
bool caseless_eq_string_with_length(const char *s1,const char *s2,int n);

char *mk_printf(const char *format, ...);
int get_printf_buffer_size(void);

int count_occurences_of_char_in_string(const char *s,char c);
int find_char_in_string(const char *s,char c);

/** Convert a string to double value, if possible
    input:
        const char *: The target null-terminated string
    output:
        double *:  The target address holding the double value
                   The value will not be changed if function 
                       returns 'false'
    output:
        bool: 'true' if succeed, or else 'false'

    Example of converting:
          "213.21" => 213.21
          "$212,212" => 212212
          "($23,323,212)" => -2.33232e+007
          "(843" is not a number, return 'false'
          "  -$54,432" => -54432
          "34,234%" => 342.34
          "2.34e-1" => 0.234
          "2.34E1.2" => 37.0865
          "50/3" => 16.6667

    Some combinations are not allowed, like:
         "(34.21E2)" : Neither a scientific nor accounting notation
         "3.1415926/25" : Not a fraction but a formula
    Some combinations are allowed although it may not be so meaningful,
       like:
         "$3278%" => 32.78
*/
bool string_to_double(const char *s, double *pValue);

/** Returns a copy of string in which any character appearing in
   chars has been deleted (that's deleted, not just replaced by
   a space.

   Example: 

   mk_copy_string_without_chars("andrew moore","erx") -> "andw moo"
*/
char *mk_copy_string_without_chars(const char *string,const char *chars);

/**returns a copy of str in which all instances of the substring 'bad' 
  are replaced by the substring 'good'.*/
char *mk_replace_string_in_string(const char *str, const char *bad, 
				  const char *good);

char *mk_replace_string(const char *string,char oldc,char newc);

char *mk_copy_string_trans_chars(const char *string,const char *chars, char replacement);

/** Works out the length of the non null string s, and am_free's it.
  
   The following functions are synonymous (i.e. do the same thing).
   free_string is the preferred choice because it is consistent with naming
   conventions of all other free functions in the Auton libraries */
void free_string(const char *s);
void am_free_string(const char *s);

/** FIXME: Our auton conventions here kind of stink. I think we should try
	to look like the C library function names when possible, so apropos
	will be useful. Either that, or we make manpages. */
char *mk_concat_strings(const char *s1, const char *s2);

/** pattern is a string with *s * can represent 0 or more characters.
   returns true if pattern is in the string

  example:
@verbatim
  pattern   string  return
  *x        hex     TRUE
  *xy       hex     FALSE
  an*w      andrew  TRUE
  *n*w      andrew  TRUE
@endverbatim
*/

bool string_pattern_matches(const char *pattern, const char *string);

/** 
   - string_has_suffix("plop.ps",".ps") == TRUE
   - string_has_suffix("plop.ps","ps") == TRUE
   - string_has_suffix("plops",".ps") == FALSE
   - string_has_suffix("plops","ps") == TRUE
   - string_has_suffix("plop.ps.hello",".ps") == FALSE */
bool string_has_suffix(const char *string,const char *suffix);

/** Returns -1 if c does not appear in s
   Else returns smallest i such that s[i] == c */
int find_index_of_char_in_string(const char *s,char c);
/** Same as above, except returns the largest i. */
int find_last_index_of_char_in_string(const char *s,char c);

/** Returns -1 if no member of s appears in stops.
   Else returns smallest i such that s[i] appears in stops */
int find_index_in_string(const char *s,const char *stops);

/** Makes a new null-terminated string using character
@verbatim
   s[start] s[start+1] ... s[end-1]
   strlen(result) = end - start

   PRE: 0 <= start < end <= strlen(s) 
@endverbatim
*/
char *mk_substring(const char *s,int start,int end);

/** Just like mk_substring except returns an empty string ("")
   if start >= end, and makes start at least 0 and end at most
   strlen(s) before going into action. Always succeeds. */
char *mk_friendly_substring(const char *s,int start,int end);

char *mk_remove_chars(const char *string,const char *seppers);

/** Removes leading and trailing whitespace */
char *mk_string_stripped(const char *str);

/** If s contains a "." character, returns a
   string result that is the same as s up to before the last ".".
   result ends at that last dot. 

   - "plop.csv" -> "plop"
   - "hello.html" -> "html"
   - "plop.hello.html" -> "plop.hello"
   - "boing/plop.hello.html" -> "/boing/plop.hello"
   - "simple" -> "simple"
*/
char *mk_filename_without_suffix(const char *s);

/** Returns the uppercase version of the provided character */
char upcase_char(char c);

/** If s contains a "." character, returns a
   string result that is the same as s up to before the last ".",
   followed by .<suffix>.

   If s has no ".", simple returns "string"."suffix"
   result ends at that last dot. 

   Examples, assume suffix == "txt"

   - "plop.csv" -> "plop.txt"
   - "hello.html" -> "html.txt"
   - "plop.hello.html" -> "plop.hello.txt"
   - "boing/plop.hello.html" -> "/boing/plop.hello.txt"
   - "simple" -> "simple.txt"
*/
char *mk_filename_with_replaced_suffix(const char *string,const char *new_suffix);


/** Following function converts stuff like the following:
	- ./foo.txt -> foo.txt
	- /usr/local/bin/mozilla -> mozilla
	- c:\foo -> foo
	- c:\foo\bar -> bar

   It does NOT look at the filesystem or anything like that.
*/

/** Makes a copy of string but with the path (everything up to
   and including the final /) removed. Examples:
   
     - /bin/plop/boink ---> boink
     - "/bin/plop/boink zoop" ---> "boink zoop"
     - zoop ---> zoop
*/

char* mk_filename_last_part(char* string);

char* mk_filename_last_part_with_default(char* string, char* default_string);

/** If argv[0] exists, makes a copy of argv[0] but with 
   the path (everything up to
   and including the final /) removed. Examples:

     - /bin/plop/boink ---> boink
     - "/bin/plop/boink zoop" ---> "boink zoop"
     - zoop ---> zoop

   If argv[0] empty or doesn't exist does something sensible anyway. */

char *mk_simple_executable_name(int argc,char** argv);

/** Returns a new string, copy of s, but without the " character */
char *mk_quoteless_string(const char *s);

/** Removes all occurences of c from string. If input was a string of
   length m containing n 'c' characters, then after it's a string of
   length m-n. */
char *mk_string_without_character(const char *string,char c);

/** Auton conventional print for a string */
void pstring(const char *s);

void string_update_sizeof(const char *str, auton_sf *x);

/** Returns true if c is whitespace or containined in the 0-terminated
    sepper string. If seppers is NULL it's ignored. */
bool is_sepper(char c, const char *seppers);

/** On entry *ds_ref points to an array of characters of size *power_ref.
   Characters 0 .. (*pos_ref)-1 are filled with characters of a string
   that is being built up.

   IF (*pos_ref < *power_ref)
     then c is placed in the *pos_ref'th character and *pos_ref is incremented

   Else a new array of chars is made of twice the length of previous one,
   the previous array is copied in, the previous array is freed and *ds_ref
   is set to point to the new one.

   NULL is a valid string to pass to this function (and probably
   should be).  Use a size of zero for NULL. */
void add_char_to_dstring_s_p(char **ds_ref, char c, int *pos_ref, int *power_ref);

/** Reads to the end of line, or end of file
   and adding all characters into string. If nothing on
   the line, returns a string of size zero. If nothing
   on line and file ends immediately returns NULL.

   Modified 1-23-96 JS: The reading always ends at an EOL (or EOF), but a
   single or double quote can be used to delimit a string that includes spaces
   or even newlines that should remain part of this string.  Therefore, if
   single quotes, double quotes, and backslashes are actually wanted in the 
   final string they should be specified by \' and \" and \\ respectively.
   Not the whole line is copied as is including all backslashes and quotes.
   All of the checking serves only to find the end of this line as defined
   by these rules.
   Modified 1-31-96 JS: single quote is no longer a valid delimiter
   Rewritten 6-28-96 MD: now resizes string to fit

   Updated by AWM 10/06/2000: Now ignores Control-Ms if it finds them.
   The first Control-M found causes a short message to go to stdout. */
char *mk_string_from_line_allowing_quoted_newlines(FILE *s);

/** Do a fgetc, but if there's a ^W, give a warning ONLY ONCE, and return
   the next non-^W character. */
int fgetc_ignore_control_w(FILE *s);

/** No good reason for this function to exist. */
void ungetc_ignore_control_w(int prev_char,FILE *s);

char *mk_string_from_line_without_quoted_newlines(FILE *s, bool careful_with_newlines);

char *mk_string_from_line(FILE *s);

/** Show a prompt and get a reply string.
   Use mk_get_user_input (ambs.c) instead of this function if your code is
   applicified. 
*/
char *mk_string_from_user(const char *prompt);

/** Behaves just like regular fgetc(s) except...
    - if it meets \n\r behaves just as though it met \n by itself
    - if it meets \r\n behaves just as though it met \n by itself
    - if it meets \r   behaves just as though it met \n by itself
    -  ignores Control-W (0x17) 
*/
int clever_fgetc(FILE *s);

/** This function is most useful for functions that want
   to accumulate a set of messages, error or otherwise, as they
   go.  This function also works when *err_mess is NULL initially. */
void add_to_error_message(char **errmess, const char *new_mess);

/** same as above except new_mess is prepended */
void prepend_error_message(char **errmess, const char *new_mess);

void fprint_dashes(FILE *s,int num_dashes);

int am_isspace(int c); /** DO NOT USE isspace() */

int string_num_bytes(const char *str);

#endif /** AMSTR_H */

