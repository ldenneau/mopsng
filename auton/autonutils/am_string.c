
/* am_string.c
   File:         amstr.c
   Author:       Andrew Moore
   Created:      June 25 1997 from amma.c by Frank Dellaert
   Description:  String related functions.

   Copyright (C) Andrew W. Moore, 1992

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

#include "am_string.h"
#include "am_string_array.h"
#include "amma.h"

/* Used to remember messages about warnings. */
bool Control_m_message_delivered = FALSE;
bool Lone_control_m_message_delivered = FALSE;

/* Proto for private functions */
char *mk_amstr_substring(const char *s,int start,int end);
bool old_string_pattern_matches(const char *pattern, const char *string);


char *mk_copy_string(const char *s)
{
  char *newv;

  if ( s == NULL )
  {
    newv = NULL;
  }
  else
  {
    newv = AM_MALLOC_ARRAY(char,1+strlen(s));
    sprintf(newv,"%s",s);
  }

  return(newv);
}

/*copy first n chars*/
char *mk_copy_string_n(const char *s, int n)
{
    char *newv;

    if (s == NULL)
    {
	newv = NULL;
    }
    else
    {
	int len = strlen(s);
	int num = n;
	if (num > len) num = len;

	newv = AM_MALLOC_ARRAY(char, 1+num);
	strncpy(newv, s, num);
	newv[num] = '\0';
    }

    return(newv);
}


char *mk_printf(const char *format, ...)
{
  VA_MAGIC_INTO_BIG_ARRAY
  return mk_copy_string(big_array);
}

int get_printf_buffer_size()
{
  return BIG_ARRAY_SIZE;
}

char *mk_string_extension(const char *s, const char *ext)
{
  const char *sub = s;
  char *dot_ext = AM_MALLOC_ARRAY(char, strlen(ext)+2);
  char *res;
  bool has_ext = FALSE;

  dot_ext[0] = '.';
  strcpy(dot_ext+1,ext);
  while(sub)
  {
    sub = strstr(sub,dot_ext);
    if (sub && (strlen(sub) == strlen(dot_ext))) 
    {
      has_ext = TRUE;
      break;
    }
    if (sub) sub++;  /* make sure we keep progressing through the string */
  }
  if (has_ext) res = mk_copy_string(s);
  else
  {
    res = AM_MALLOC_ARRAY(char, strlen(s) + strlen(dot_ext) + 1);
    strcpy(res,s);
    strcpy(res+strlen(s),dot_ext);
  }
  AM_FREE_ARRAY(dot_ext, char, strlen(dot_ext));
  return res;
}

char *make_copy_string(const char *s)
{
  return(mk_copy_string(s));
}

char *mk_copy_quoted_string(const char *s)
{
  int i,idx;
  char *newv;
  char *resultv;

  if ((!s) || (!strcmp(s,"NULL"))) return NULL;

  newv = AM_MALLOC_ARRAY(char,1+strlen(s));

  for (i=0,idx=0;i< (int) strlen(s);i++)
  {
    if (s[i] == '\"') continue;
    else if (s[i] == '\\')
    {
      if ((s[i+1] == '\"')||(s[i+1] == '\\')) 
      {
	newv[idx++] = s[i+1]; 
	i++;
      }
      /* technically there shouldn't be an else, but we'll be graceful */
      else newv[idx++] = s[i];
    }
    else newv[idx++] = s[i];
  }
  newv[idx] = '\0';
  resultv = mk_copy_string(newv);
  AM_FREE_ARRAY(newv,char,1+strlen(s));
  return resultv;
}

/* inverts the mk_copy_quoted_string operation */
char *mk_copy_to_quoted_string(const char *s)
{
  int i,size,idx;
  char *newv;
  bool whitespace = FALSE;

  if (!s) return mk_copy_string("NULL");

  for (i=0,size=2;i< (int) strlen(s);i++)
  {
    if (am_isspace(s[i])) whitespace = TRUE;
    if ((s[i]=='\\')||(s[i]=='\"')) size+=2;
    else size++;
  }

  if (!whitespace) return mk_copy_string(s);

  newv = AM_MALLOC_ARRAY(char,1+size);
  idx = 0;
  newv[idx++] = '\"';
  for (i=0;i< (int) strlen(s);i++)
  {
    if ((s[i]=='\\')||(s[i]=='\"')) newv[idx++] = '\\';
    newv[idx++] = s[i];
  }
  newv[idx++] = '\"';
  newv[idx] = '\0';
  return newv;
}

char *mk_input_string(const char *message)
{
  char buff[200];
  printf("%s > ",message);
  input_string("",buff,200);
  return(mk_copy_string(buff));
}

char *mk_downcase_string(const char *s)
{
  char *result = mk_copy_string(s);
  int i;
  for ( i = 0 ; result[i] != '\0' ; i++ )
    if ( result[i] >= 'A' && result[i] <= 'Z' )
      result[i] = (char)(result[i] - 'A' + 'a');
  return(result);
}

char *mk_downcase_string_with_length(const char *s,int n)
{
  char *result = mk_copy_string(s);
  int i;
  for ( i = 0 ; i < n ; i++ )
    if ( result[i] >= 'A' && result[i] <= 'Z' )
      result[i] = (char)(result[i] - 'A' + 'a');
  return(result);
}

char *mk_upcase_string(const char *s)
{
  char *result = mk_copy_string(s);
  int i;
  for ( i = 0 ; result[i] != '\0' ; i++ )
    if ( result[i] >= 'a' && result[i] <= 'z' )
      result[i] = (char)(result[i] - 'a' + 'A');
  return(result);
}

char *mk_upcase_string_with_length(const char *s,int n)
{
  char *result = mk_copy_string(s);
  int i;
  for ( i = 0 ; i < n ; i++ )
    if ( result[i] >= 'a' && result[i] <= 'z' )
      result[i] = (char)(result[i] - 'a' + 'A');
  return(result);
}

bool caseless_eq_string(const char *s1,const char *s2)
{
  int i;
  bool result = TRUE;
  for ( i = 0 ; s1[i] != '\0' && s2[i] != '\0' && result ; i++ )
  {
    char c1 = s1[i];
    char c2 = s2[i];
    if ( c1 >= 'a' && c1 <= 'z' ) c1 = c1 + 'A' - 'a';
    if ( c2 >= 'a' && c2 <= 'z' ) c2 = c2 + 'A' - 'a';
    result = c1 == c2;
  }
  if ( result && s1[i] != s2[i] )
    result = FALSE;
  return result;
}

bool caseless_eq_string_with_length(const char *s1,const char *s2,int n)
{
  char *d1 = mk_downcase_string(s1);
  char *d2 = mk_downcase_string(s2);
  bool result = eq_string_with_length(d1,d2,n);
  free_string(d1);
  free_string(d2);
  return(result);
}

int count_occurences_of_char_in_string(const char *s,char c){
  char *p;
  int count;
  for(p=strchr(s,c),count=0;p;p=strchr(p+1,c),count++);
  return count;
}

int find_char_in_string(const char *s,char c){
  char *p = strchr(s,c);
  return (p)? (p-s):-1;
}

bool string_to_double(const char *s, double *pValue)
{    int i, nLen;
     const char *p = s;

     int sign = 1;
     double vInt = 0, vDec = 0, vFraction = 1, vPower = 1;
     double *pV  = &vInt;

     for( ; *p==' ' && *p!='\0' ; ++p )
          ;

     if( *p == '\0' )
          return FALSE;

     nLen = strlen(p);
     
     for( i=0; i<nLen; ++i,++p )
     {
          switch( *p )
          {
               case '0': case '1': case '2': case '3': case '4': 
               case '5': case '6': case '7': case '8': case '9': 
                    *pV = *pV * 10 + ( *p - '0' );
                    break;

               case '+':
                    if( i > 0 )
                         return FALSE;
                    break;

               case '-':
                    if( i > 0 )
                         return FALSE;
                    sign = -1;
                    break;
                    
               case '.':
                    if( pV != &vInt )
                         return FALSE;
                    pV = &vDec;
                    break;

               case '/':
                    if( pV != &vInt )
                         return FALSE;
                    vFraction = 0;
                    pV = &vFraction;
                    break;

               case '%':
                    if( i != nLen-1 )
                         return FALSE;
                    vPower = 0.01;
                    break;

               case 'E': case 'e':
                    if( i == nLen-1 )
                         return FALSE;
                    if( string_to_double(p+1,&vPower) == FALSE )
                         return FALSE;
                    vPower = pow(10.0,vPower);
                    i = nLen; /* to stop */
                    break;

               case '$':
                    if( i==0 || ( i==1 && sign==-1 ) )
                         break;
                    return FALSE;

               case ',':
                    if( (nLen-i)%4 == 0 || ( (nLen-i)%4 == 1 && s[nLen-1] == '%' ) )
                         break;
                    return FALSE;
                    
               case '(':
                    if( i != 0 || s[nLen-1] != ')' )
                         return FALSE;
                    sign = -1;
                    nLen--;
                    break;


               default:
                    return FALSE;

          }
     }
     
     while( vDec > 1 )
          vDec /= 10;

     *pValue = ( vInt + vDec ) * sign / vFraction * vPower;
     
     return TRUE;
}

void free_string(const char *s)
{
  if ( s == NULL )
    my_error("am_free_string NULL s");
  AM_FREE_ARRAY(s,char,(1 + strlen(s)));
}

void am_free_string(const char *s)
{
  free_string(s);
}

char *mk_concat_strings(const char *s1, const char *s2)
{
  char *s = AM_MALLOC_ARRAY(char,1+strlen(s1)+strlen(s2));
  sprintf(s,"%s%s",s1,s2);
  return s;
}

char *mk_amstr_substring(const char *s,int start,int end)
{
  char *result;
  int i;
  if ( start < 0 || start >= end || end > (int)strlen(s) )
    my_error("mk_substring bad start or end");

  result = AM_MALLOC_ARRAY(char,end-start+1); /* +1 for '\0' */

  for ( i = 0 ; i < end-start ; i++ )
    result[i] = s[start+i];
  result[end-start] = '\0';
  return result;
}

/* Returns a copy of string in which any character appearing in
   chars has been deleted (that's deleted, not just replaced by
   a space.

   Example: 

   mk_copy_string_without_chars("andrew moore","erx") -> "andw moo"
*/
char *mk_copy_string_without_chars(const char *string,const char *chars)
{
  int len = (int) strlen(string);
  int temp_size = len+1;
  char *temp = AM_MALLOC_ARRAY(char,temp_size);
  int i;
  int j = 0;
  char *result;

  for ( i = 0 ; i < len ; i++ )
  {
    char c = string[i];
    if ( !char_is_in(chars,c) )
    {
      temp[j] = c;
      j += 1;
    }
  }

  temp[j] = '\0';

  /* Can't just return temp because it's probably the wrong length amount of
     memory */

  result = mk_copy_string(temp);

  AM_FREE_ARRAY(temp,char,temp_size);

  return result;
}


/*returns a copy of str in which all instances of the substring 'bad' 
  are replaced by the substring 'good'.*/
char *mk_replace_string_in_string(const char *str, 
				  const char *bad, const char *good)
{
    if (bad && good && strlen(bad) > 0)
    {
	string_array *sa = mk_split_string(str, bad);
	char *ret = mk_join_string_array(sa, good);
	free_string_array(sa);
	return ret;
    }
    else
    {
	return mk_copy_string("");
    }
}

char *mk_replace_string(const char *string,char oldc,char newc)
{
  int i;
  char *copy_string = mk_copy_string(string);

  for ( i = 0 ; copy_string[i] != '\0' ; i++ )
  {
    if ( copy_string[i] == oldc )
      copy_string[i] = newc;
  }
  return copy_string;
}

bool string_pattern_matches(const char *pattern, const char *string)
{
  if(pattern[0] == '*')
    {
      int spos, passed = 0, ppos = 1;

      if(pattern[1] == '\0')
	return 1;

      /*concatenate multiple **** into one * 
       */
      while(pattern[ppos] == '*')
	ppos++;

      for(spos = 0; spos < (int) strlen(string); spos++)
	{
	  /*printf("after * testing pattern=\"%s\" string=\"%s\"\n",
	    pattern+ppos, string+spos);*/
	  passed = string_pattern_matches(pattern+ppos, string+spos);
	  if(passed)
	    break;
	}
      /*if(!passed) printf("Didn't match last *, quitting\n");*/
      return passed;
    }
  else
    {
      if(pattern[0] == string[0])
	{
	  int ret;

	  if(pattern[0] == '\0')
	    return 1;

	  /*printf("testing pattern=\"%s\"  string=\"%s\"\n",
	    pattern+1, string+1);*/

	  ret = string_pattern_matches(pattern+1, string+1);

	  return ret;
	}
      else
	{
	  /*printf("not match pattern=\"%s\"  string=\"%s\"\n",
	    pattern, string);*/
	  return 0;
	}
    }

}

bool old_string_pattern_matches(const char *pattern, const char *string)
{
  /* p_pos - index to pattern and s_pos is index to string */
  int p_pos = 0, s_pos = 0; 
  bool prev_ast = 0; /* true if the previous char in pattern is * */

  for(p_pos = 0; p_pos < (int) strlen(pattern); p_pos++)
  {
    if(pattern[p_pos] == '*') 
    {
      prev_ast = 1;
    }
    else
    {
      if(prev_ast)
      {
        /*if the last char was a *, try to match the whole string*/
        while(string[s_pos] != '\0' && pattern[p_pos] != string[s_pos])
        {
          s_pos++;
        }
      }
      if(pattern[p_pos] == string[s_pos])
        s_pos++;
      else
        return 0;
      prev_ast = 0;
    }
  }
  /*last char in pattern wasnot * and string not reached to the end,
  backtrack the string and pattern to make sure they match*/
  if(!prev_ast && s_pos != (int)strlen(string))
  {
    s_pos = (int) strlen(string) - 1;
    p_pos = (int) strlen(pattern) - 1;
    while(pattern[p_pos] != '*' && pattern[p_pos] == string[s_pos])
    {
      /* printf("%1s matched with %1s\n", &pattern[p_pos], &string[s_pos]); */
      s_pos--;
      p_pos--;
    }
    if(pattern[p_pos] != '*')
      return 0;
  }

  return 1;
}

/* string_has_suffix("plop.ps",".ps") == TRUE
   string_has_suffix("plop.ps","ps") == TRUE
   string_has_suffix("plops",".ps") == FALSE
   string_has_suffix("plops","ps") == TRUE
   string_has_suffix("plop.ps.hello",".ps") == FALSE */
bool string_has_suffix(const char *string,const char *suffix)
{
  int suffix_length = (int) strlen(suffix);
  int string_length = (int) strlen(string);
  int matches = string_length >= suffix_length;
  int i;
  for ( i = 0 ; matches && i < suffix_length ; i++ )
    matches = string[string_length - suffix_length + i] == suffix[i];
  return matches;
}

/* Returns -1 if c does not appear in s
   Else returns smallest i such that s[i] == c */
int find_index_of_char_in_string(const char *s,char c)
{
  int result = -1;
  int i;
  for ( i = 0 ; result < 0 && s[i] != '\0' ; i++ )
  {
    if ( c == s[i] )
      result = i;
  }
  return result;
}

int find_last_index_of_char_in_string(const char *s,char c)
{
  int i = strlen(s) - 1;
  while(i > -1)
    {
    if ( c == s[i] )
      return i;
    i--;
    }
  return i;
}

/* Returns -1 if no member of s appears in stops.
   Else returns smallest i such that s[i] appears in stops */
int find_index_in_string(const char *s,const char *stops)
{
  int result = -1;
  int i;
  for ( i = 0 ; result < 0 && s[i] != '\0' ; i++ )
  {
    if ( 0 <= find_index_of_char_in_string(stops,s[i]) )
      result = i;
  }
  return result;
}

/* Makes a new null-terminated string using character
   s[start] s[start+1] ... s[end-1]
   strlen(result) = end - start

   PRE: 0 <= start < end <= strlen(s) 
*/
char *mk_substring(const char *s,int start,int end)
{
  char *result;
  int i;
  if ( start < 0 || start >= end || end > (int)strlen(s) )
    my_error("mk_substring bad start or end");

  result = AM_MALLOC_ARRAY(char,end-start+1); /* +1 for '\0' */

  for ( i = 0 ; i < end-start ; i++ )
    result[i] = s[start+i];
  result[end-start] = '\0';
  return result;
}

/* Just like mk_substring except returns an empty string ("")
   if start >= end, and makes start at least 0 and end at most
   strlen(s) before going into action. Always succeeds. */
char *mk_friendly_substring(const char *s,int start,int end)
{
  char *result;

  start = int_max(0,start);
  end = int_min((int)strlen(s),end);

  if ( end <= start )
    result = mk_copy_string("");
  else
    result = mk_substring(s,start,end);

  return result;
}

char *mk_remove_chars(const char *string,const char *seppers)
{
  int temp_size = (int)strlen(string) + 1;
  char *temp = AM_MALLOC_ARRAY(char,temp_size);
  char *result = NULL;
  int string_index;
  int temp_index = 0;
  for ( string_index = 0 ; string[string_index] != '\0' ; string_index++ )
  {
    char c = string[string_index];
    if ( find_index_of_char_in_string(seppers,c) < 0 )
    {
      temp[temp_index] = c;
      temp_index += 1;
    }
  }
  temp[temp_index] = '\0';
  result = mk_copy_string(temp);
  AM_FREE_ARRAY(temp,char,temp_size);
  return result;
}

/* Make string with leading and trailing whitespace removed
 */
char *mk_string_stripped(const char *str)
{
  int start,stop;

  for (start = 0; start < strlen(str); start++)
    if (str[start] != ' ')
      break;
  for (stop = strlen(str)-1; stop >= 0; stop--)
    if (str[stop] != ' ')
      break;

  if (start <= stop) {
    int string_num_bytes = stop-start + 2;
    char *temp = AM_MALLOC_ARRAY(char,string_num_bytes);
    int i;
    for (i = 0; i < stop-start+1; i++)
      temp[i] = str[start+i];
    temp[string_num_bytes-1] = '\0';
    return temp;
  }
  else
    return mk_copy_string("");
}

/* If s contains a "." character, returns a
   string result that is the same as s up to before the last ".".
   result ends at that last dot. 

   "plop.csv" -> "plop"
   "hello.html" -> "html"
   "plop.hello.html" -> "plop.hello"
   "boing/plop.hello.html" -> "/boing/plop.hello"
   "simple" -> "simple"
*/
char *mk_filename_without_suffix(const char *s)
{
  int i = (int) strlen(s) - 1;
  char *result;

  while ( i >= 0 && s[i] != '.' )
    i -= 1;
  if ( i < 0 )
    result = mk_copy_string(s);
  else
    result = mk_substring(s,0,i);

  return result;
}

char upcase_char(char c)
{ /* Why not just use toupper() ? */
  return (c >= 'a' && c <= 'z') ? (c - 'a' + 'A') : c;
}

/* If s contains a "." character, returns a
   string result that is the same as s up to before the last ".",
   followed by .<suffix>.

   If s has no ".", simple returns "string"."suffix"
   result ends at that last dot. 

   Examples, assume suffix == "txt"

   "plop.csv" -> "plop.txt"
   "hello.html" -> "html.txt"
   "plop.hello.html" -> "plop.hello.txt"
   "boing/plop.hello.html" -> "/boing/plop.hello.txt"
   "simple" -> "simple.txt"
*/
char *mk_filename_with_replaced_suffix(const char *string,const char *new_suffix)
{
  char *s1 = mk_filename_without_suffix(string);
  char *result = mk_printf("%s.%s",s1,new_suffix);
  free_string(s1);
  return result;
}

char* mk_filename_last_part(char* string)
{ /* In Perl, s/.*[\/\\]//;  */
  int index_to_keep_at = strlen(string)-1;
  char* returner;
  while(index_to_keep_at >= 0) /* Return empty string if ends in / or is empty */
    {
    if( (string[index_to_keep_at] == '\\') || (string[index_to_keep_at] == '/') )
      break;
    index_to_keep_at--;
    }
  returner = mk_copy_string(&string[index_to_keep_at+1]);
  return returner;
}

char* mk_filename_last_part_with_default(char* string, char* default_string)
{
  char* result = mk_filename_last_part(string);
  if(strlen(result) == 0)
    {
    free_string(result);
    result = mk_copy_string(default_string);
    }
  return result;
}

char *mk_simple_executable_name(int argc,char** argv)
{
  char *result;

  if ( argc == 0 ) /* This should probably never happen */
    result = mk_copy_string("executable_name");
  else
    result = mk_filename_last_part_with_default(argv[0], "executable_name");

  return result;
}

char *mk_quoteless_string(const char *s)
{
  return mk_string_without_character(s,'\"');
}

char *mk_string_without_character(const char *string,char c)
{
  int string_num_chars = (int)strlen(string);
  int string_num_bytes = string_num_chars + 1;
  char *temp = AM_MALLOC_ARRAY(char,string_num_bytes);
  int si = 0;
  int ti = 0;
  char *result;

  for ( si = 0 ; si < string_num_chars ; si++ )
  {
    if ( string[si] != c )
    {
      temp[ti] = string[si];
      ti += 1;
    }
  }

  temp[ti] = '\0';
  result = mk_copy_string(temp);
  AM_FREE_ARRAY(temp,char,string_num_bytes);
  return result;
}

void pstring(const char *s)
{
  printf("string = %s\n",s);
}

void string_update_sizeof(const char *str, auton_sf *x)
{
  if ( str != NULL )
    add_to_auton_sf(x,(unsigned long)strlen(str));
}

bool is_sepper(char c, const char *seppers)
{
  bool result = c == ' ' || c == '\n' || c == '\r' || c == '\t';
  if ( !result && seppers != NULL )
  {
    int i;
    for ( i = 0 ; !result && seppers[i] != '\0' ; i++ )
      if ( seppers[i] == c ) result = TRUE;
  }
  return(result);
}

void add_char_to_dstring_s_p(char **ds_ref, char c,
			     int *pos_ref, int *power_ref)
{
  char *new_ds;
  int new_pow = *power_ref;

  if (*pos_ref >= *power_ref)
  {
    while (*pos_ref >= new_pow)
      if (*power_ref == 0)
	new_pow = 32;
      else
	new_pow += new_pow;
    new_ds = AM_MALLOC_ARRAY(char, new_pow);
    if (*ds_ref != NULL)
    {
      memcpy(new_ds, *ds_ref, *power_ref);
      AM_FREE_ARRAY(*ds_ref, char, *power_ref);
    }
    *ds_ref = new_ds;
    *power_ref = new_pow;
  }
  (*ds_ref)[(*pos_ref)++] = c;
}

char *mk_string_from_line_allowing_quoted_newlines(FILE *s)
{
  int size = 0;
  int pos = 0;
  int c = '\0';
  char *line = NULL;
  char *retval;
  bool finished = FALSE;
  bool instring = FALSE;
  bool possible_quoted = FALSE;
#ifdef OLD_SPR
  int i = 0;
  char *str = NULL;
#ifdef PC_MVIS_PLATFORM
  bool use_str = s == stdin;
#else
  bool use_str = FALSE;
#endif

  if ( use_str )
  {
    gui_prompt("Enter>",CLI_COMMAND_INPUT,FALSE);
    str = gui_getstring(s);
  }

#endif
  while (!finished)
  {
    c = 
#ifdef OLD_SPR
      (use_str)? str[i++] : 
#endif
      fgetc(s);

    switch (c)
    {
    case EOF:
      finished = TRUE;
      break;
    case '\n':
      if (instring)
      {
	      if (possible_quoted)
	      {
	        add_char_to_dstring_s_p(&line, '\\', &pos, &size);
	        possible_quoted = FALSE;
	      }
	      add_char_to_dstring_s_p(&line, (char) c, &pos, &size);
      }
      else
	      finished = TRUE;
      break;
    case '\\':
      if (possible_quoted)
      {
	      add_char_to_dstring_s_p(&line, (char) c, &pos, &size);
	      possible_quoted = FALSE;
      }
      else
	      possible_quoted = TRUE;
      break;
    case '\"':
      if (!possible_quoted)
	      instring = !instring;
      add_char_to_dstring_s_p(&line, (char) c, &pos, &size);
      break;
    case 0x0D:
    {
      if ( !Control_m_message_delivered )
      {
	printf("\n*** I notice a control-M in the file. This and all future\n"
	       "*** Control-M's will be ignored.\n\n");
	Control_m_message_delivered = TRUE;
      }
      break;
    }
    default:
      if (possible_quoted)
      {
	      possible_quoted = FALSE;
	      add_char_to_dstring_s_p(&line, '\\', &pos, &size);
      }
      add_char_to_dstring_s_p(&line, (char) c, &pos, &size);
    }
  }
  if (line != NULL || c == '\n')
    add_char_to_dstring_s_p(&line, '\0', &pos, &size);

  /* Have to do this because the dstring in line may contain
     more characters than the string length. */
  retval = mk_copy_string(line);
  if (line != NULL)
    AM_FREE_ARRAY(line,char,size);

#ifdef OLD_SPR  
  if ( use_str ){
    gui_unprompt();
    free_string(str);
  }
#endif

  return (retval);
}

int fgetc_ignore_control_w(FILE *s)
{
  int result = fgetc(s);

  if ( result == 0x17 )
  {
    static bool reported_control_w = FALSE;

    if ( !reported_control_w )
    {
      printf("*** I was surprised to find a Control-W (0x17) in the file.\n"
	     "*** I proceeded to ignore it. I will ignore all future\n"
	     "*** control w's silently\n");
      reported_control_w = TRUE;
    }

    while ( result == 0x17 )
      result = fgetc(s);
  }

  return result;
}

void ungetc_ignore_control_w(int prev_char,FILE *s)
{
  ungetc(prev_char,s);
}


char *mk_string_from_line_without_quoted_newlines(FILE *s,
						  bool careful_with_newlines)
{
  int array_size = 20;
  char *array = AM_MALLOC_ARRAY(char,array_size);
  char *result;
  bool finished = FALSE;
  bool file_ended = FALSE;
  int i = 0;

  while ( !finished && !file_ended )
  {
    int c = (careful_with_newlines) ? clever_fgetc(s) : fgetc(s);
    if ( c == EOF ) file_ended = TRUE;
    else if ( c == '\n' ) finished = TRUE;
    else
    {
      if ( i > array_size-2 )
      {
	int new_array_size = 2 * array_size;
	char *new_array = AM_MALLOC_ARRAY(char,new_array_size);
	int j;
	for ( j = 0 ; j < i ; j++ )
	  new_array[j] = array[j];
	AM_FREE_ARRAY(array,char,array_size);
	array = new_array;
	array_size = new_array_size;
      }
      array[i] = (char) c;
      i += 1;
    }
  }

  array[i] = '\0';

  if ( file_ended && i == 0 )
    result = NULL;
  else
    result = mk_copy_string(array);

  AM_FREE_ARRAY(array,char,array_size);

  return result;
}

char *mk_string_from_line(FILE *s)
{
  return mk_string_from_line_without_quoted_newlines(s,s != stdin);
}

/* Show a prompt and get a reply string.
   Use mk_get_user_input (ambs.c) instead of this function if your code is
   applicified. 
*/
char *mk_string_from_user(const char *prompt)
{
  printf("%s> ",prompt);
  return mk_string_from_line(stdin);
}

int clever_fgetc(FILE *s)
{
  int result = fgetc_ignore_control_w(s);

  if ( s == stdin )
    my_error("clever_fgetc should not be used with stdin");

  if ( result == 0x0D || result == 0x0A )
  {
    int next = fgetc_ignore_control_w(s);
    if ( (result == 0x0D && next == 0x0A) ||
	 (result == 0x0A && next == 0x0D) )
    {
      if ( !Control_m_message_delivered )
      {
	printf("\n"
	       "*** I notice a control-M (0x0D) in the file next to a\n"
	       "*** control-J (Newline, 0x0A). In this and all other\n"
	       "*** future similar cases I'll ignore the control-M\n");
	Control_m_message_delivered = TRUE;
      }
      result = 0x0A;
      my_assert(0x0A == '\n');
    }
    else if ( result == 0x0D )
    {
      if ( !Lone_control_m_message_delivered )
      {
	printf("\n"
	       "*** I notice a control-M (0x0D) in the file but not next\n"
	       "*** to a\n"
	       "*** control-J (Newline, 0x0A). I'll treat it like a\n"
	       "*** newline now and in the future.\n");
	Lone_control_m_message_delivered = TRUE;
      }
      result = 0x0A;
      my_assert(0x0A == '\n');

      ungetc_ignore_control_w(next,s);
    }
    else
      ungetc_ignore_control_w(next,s);
  }
  return result;
}

void add_to_error_message(char **errmess, const char *new_mess)
{
  if (!new_mess) return;
  else
  {
    if (*errmess)
    {
      int len = strlen(*errmess) + strlen(new_mess) + 2;
      char *buf = AM_MALLOC_ARRAY(char,len);
      sprintf(buf,"%s%s",*errmess,new_mess);
      am_free_string(*errmess);
      *errmess = mk_copy_string(buf);
      AM_FREE_ARRAY(buf,char,len);
    }
    else *errmess = mk_copy_string(new_mess);
  }
}

void prepend_error_message(char **errmess, const char *new_mess)
{
  if (!new_mess) return;
  else
  {
    if (*errmess)
    {
      int len = strlen(*errmess) + strlen(new_mess) + 2;
      char *buf = AM_MALLOC_ARRAY(char,len);
      sprintf(buf,"%s%s",new_mess,*errmess);
      am_free_string(*errmess);
      *errmess = mk_copy_string(buf);
      AM_FREE_ARRAY(buf,char,len);
    }
    else *errmess = mk_copy_string(new_mess);
  }
}

void fprint_dashes(FILE *s,int num_dashes)
{
  int i;
  for ( i = 0 ; i < num_dashes ; i++ )
    fprintf(s,"-");
  fprintf(s,"\n");
}

int am_isspace(int c)
{ /* See http://bugzilla.redhat.com/bugzilla/show_bug.cgi?id=86465
     This is not an issue if we don't reference that symbol, which means
     avoiding locale stuff like the libc isspace(). */
  if(	(c == ' ')
||	(c == '\f')
||	(c == '\n')
||	(c == '\r')
||	(c == '\t')
||	(c == '\v') )
	return 1;
return 0;
}

char *mk_copy_string_trans_chars(const char *string, const char *chars, char replacement)
{
  int len = (int) strlen(string);
  int temp_size = len+1;
  char *temp = AM_MALLOC_ARRAY(char,temp_size);
  int i;
  int j = 0;
  char *result;

  for ( i = 0 ; i < len ; i++ )
    {
    char c = string[i];
    if ( !char_is_in(chars,c) )
      temp[j] = c;
    else
      temp[j] = replacement;
    j++;
    }

  temp[j] = '\0';

  /* Can't just return temp because it's probably the wrong length amount of
     memory */

  result = mk_copy_string(temp);

  AM_FREE_ARRAY(temp,char,temp_size);

  return result;
}

/* moved string_{edit,pattern}_distance to imat.[ch], to break
   linkage between the am_string.[ch] module and imat.[ch].
   These functions use imats, but neither use nor create
   am_strings.  - J. Ostlund
   */

char* mk_first_line_of_string(char* src)
{
  char* returner;
  int rlength;

  rlength = find_char_in_string(src, '\n');
  if(rlength == -1)
    rlength = strlen(src);
  returner = mk_substring(src, 0, rlength);
  return returner;
}

/* return a string that is the concatenation of s1 and s2.  
   If free_inputs is TRUE, frees s1 and s2 before returning.*/
char *mk_strcat(const char *s1, const char *s2, bool free_inputs)
{
    char *ret = NULL;
    if (s1 && s2)
	ret = mk_printf("%s%s", s1, s2);
    else if (s1)
	ret = mk_copy_string(s1);
    else if (s2)
	ret = mk_copy_string(s2);
	    
    if (free_inputs && s1) free_string(s1);
    if (free_inputs && s2) free_string(s2);
    return ret;
}

int string_num_bytes(const char *s)
{
  int string_num_chars = (int)strlen(s);
  return string_num_chars + 1;
}
