/* 	File: am_file.c
 * 	Author(s): Andrew Moore, Pat Gunn
 * 	Date: 31 July 2003
 * 	Purpose: Store things related to file manipulation, e.g.
 *               wrapped fopen/open functions, fcntl stuff, etc.
 *               Also contains code for manipulating file names.
 *
 * Copyright The Auton Lab, CMU
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
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "am_file.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "am_string.h"
#include "am_string_array.h"
#include "amma.h"

#ifdef PC_PLATFORM
#include <wchar.h>
#include <direct.h>
#endif

#ifdef UNIX_PLATFORM
#include <dirent.h>
#include <unistd.h>  /*access*/
#endif

/* Unix uses / as a directory seperator, and
   / as the root.  */
#define UNIX_PATHSTYLE 0
/* 	Windows uses \ as a directory seperator, and
   \ (with optional DRIVELETTER: ) as the root.
   They also support \MACHINENAME\SHARENAME as a root, IIRC, but
   we're not going to support that.
   NT/XP/2k support Unicode, but we'll
   just hope, for now, that that won't come up. */
#define DOS_PATHSTYLE 1
/* 	If we ever need it.. classic versions of MacOS used : as a
   directory seperator, and :: as the root. I don't know how
   well this translation works though. */
#define MACOS_PATHSTYLE 2
/* 	Whatever's local */
#ifdef PC_PLATFORM
#define HERE_PATHSTYLE 1
#else
#define HERE_PATHSTYLE 0
#endif

#ifdef PC_PLATFORM
#define getcwd _getcwd
#endif

static char* mk_path_translate(const char* path, int modefrom, int modeto);
static char get_sep_for_pathstyle(int mode);
static bool is_root_dir(const char *path);
static char *mk_expand_tilde(const char *path);

char* mk_dospath_from_path(const char* path)
{
  return mk_path_translate(path, UNIX_PATHSTYLE, DOS_PATHSTYLE);
}

char* mk_path_from_dospath(const char* path)
{
  return mk_path_translate(path, DOS_PATHSTYLE, UNIX_PATHSTYLE);
}

static char* mk_path_translate(const char* path, int modefrom, int modeto)
{ 
  char* iter;
  char* returner = mk_copy_string(path);
  iter = returner;

  if(modefrom == modeto) {return returner;}

  if(modefrom == DOS_PATHSTYLE)
    { /* If we're coming from windows, check for and remove driveletter prefix. */
      if( (*iter == '\0') /* Pardon my Perl style */
	  ||(*(iter+1) != ':')) /* If we don't have a driveletter, move on*/
	{
	  /*  printf("DEBUG: No removal needed\n");*/
	}
      else
	{
	  printf("Note: Removing drive letter from path [%s]\n", path);
	  free_string(returner);
	  returner = mk_copy_string(iter+2); 
	  iter = returner;
	}
    }
  while(*iter != '\0')
    {
      if(*iter == get_sep_for_pathstyle(modefrom) )
	*iter = get_sep_for_pathstyle(modeto);
      iter++;
    }
  return returner;
}

static char get_sep_for_pathstyle(int mode)
{
  switch(mode)
    {
    case DOS_PATHSTYLE:
      return '\\';

    case UNIX_PATHSTYLE:
      return '/';

    case MACOS_PATHSTYLE:
      return ':';

    default:
      my_errorf("Unknown pathstyle %d encountered!", mode);
      return '/'; /* Unused. if only we used function attributes ..*/
    }
}

bool am_mkpath_for(char* filename)
{
  char* pathpart;
  bool result;
  int part = find_last_index_of_char_in_string(filename, get_sep_for_pathstyle(HERE_PATHSTYLE));

  if(part == -1) return TRUE;
  filename[part] = '\0'; /* Tokenize */
  pathpart = mk_copy_string(filename);
  filename[part] = get_sep_for_pathstyle(HERE_PATHSTYLE); /* And restore. */
  result = am_mkdir_deeply(pathpart);
  free_string(pathpart);
  return result;
}

bool am_mkdir_deeply(const char* dirname)
{
  int diri=0;
  int keep_going=1;

  printf("am_mkdir_deeply(%s)\n", dirname);
  while(keep_going)
    { /* Whenever we see our seperator, do a mkdir on all characters up to but
	 not including that character */
      char* thispart;

      printf("diri=%d\n", diri);
      if(dirname[diri] == '\0')
	keep_going=0;
      else if(dirname[diri] != get_sep_for_pathstyle(HERE_PATHSTYLE))
	{diri++ ; continue;}

      thispart = mk_copy_string(dirname);
      thispart[diri] = '\0'; /* Chop it off at the current sep */
      am_mkdir(thispart);
      if(keep_going)
	thispart[diri] = get_sep_for_pathstyle(HERE_PATHSTYLE); /* Workaround a bug */

      free_string(thispart);
      diri++;
    }
  return am_isdir(dirname); 
}

int am_mkdir(const char* fn)
{
  int result;
#ifdef PC_PLATFORM
  result = mkdir(fn);
#else
  result = mkdir(fn, 0755);
#endif
  return (!result); /* Stupid disagreement btw kernel and C notions of truth. */
}

bool am_isdir(const char* path)
{
#ifdef PC_PLATFORM /*use GetFileAttributes because stat can't
		     handle network (UNC) paths*/

/*remove trailing backslash else windows won't find the dir*/
 char *new_path = mk_path_rm_trail_sep(path); 

 DWORD attr = GetFileAttributes(new_path);
 if (attr == 0xffffffff)
 {
   DWORD dwError = GetLastError();
 }
 else if (attr & FILE_ATTRIBUTE_DIRECTORY)
 {
   free_string(new_path);
   return TRUE;
 }
 free_string(new_path);
 return FALSE;
#else
 struct stat buf;
 char *expanded_path = mk_expand_tilde(path); /*stat won't do this for you*/
 bool ret = FALSE;

 if(stat(expanded_path, &buf) != -1)
 {
   if(S_IFDIR & buf.st_mode)
     ret = TRUE;
 }
 free_string(expanded_path);
 return ret;

#endif
}

/*static*/
char *mk_expand_tilde(const char *path)
{
  char *ret = NULL;
  char *home_env = getenv("HOME");
  if (path && strlen(path) > 1 && path[0] == '~' && home_env)
  {
    ret = mk_printf("%s%s", home_env, &path[1]);
  }
  else
  {
    ret = mk_copy_string(path);
  }
  return ret;
}

bool am_isfile(const char* path)
{
#ifdef PC_PLATFORM /*use GetFileAttributes because stat can't
		 		     handle network (UNC) paths*/
    DWORD attr = GetFileAttributes(path);
    if (attr == 0xffffffff)
    {
      DWORD dwError = GetLastError();
    }
    else if ( ! (attr & FILE_ATTRIBUTE_DIRECTORY) )
    {
      return TRUE;	
    }

    return FALSE;
#else
    struct stat buf;
    bool ret = FALSE;

    char *expanded_path = mk_expand_tilde(path); /*stat won't do this for you*/
    if(stat(expanded_path, &buf) != -1)
    {
      if(S_IFREG & buf.st_mode)
	ret = TRUE;
    }
    free_string(expanded_path);
    return ret;
#endif
}

/* Returns true if the path is writable */
bool am_iswritable(const char *path)
{
#ifdef PC_PLATFORM
  DWORD ret = GetFileAttributes(path);
  if ((ret != (DWORD)-1) && 
      !(ret & FILE_ATTRIBUTE_READONLY))
    return TRUE;
  return FALSE;
#else
  int ret = access(path, W_OK);
  return ret==0 ? TRUE : FALSE;
#endif
}

/* Rename a file or directory */
bool am_rename_file(const char* old_file, const char *new_file)
{
    if ( rename (old_file, new_file) == 0 )
	return TRUE;
    return FALSE;
}

bool filenames_exist(string_array *filenames)
{
  bool result = TRUE;
  int i;
  for ( i = 0 ; result && i < string_array_size(filenames) ; i++ )
  {
    char *filename = string_array_ref(filenames,i);
    result = result && file_exists(filename);
  }
  return result;
}

bool file_exists(const char *fname)
{
  FILE *s = fopen(fname,"r");
  bool result = s != NULL;
  if ( s != NULL ) fclose(s);
  return result;
}

/* Some characters are not valid in filenames - differs by OS */
bool is_valid_filename(const char *fname, bool validate_path_exists, 
		       char **err)
{
  bool ret = TRUE;
  char *badchars = "";
  char *basename = mk_path_basename(fname, TRUE);

  #ifdef PC_PLATFORM
  badchars = "\\/:*?\"<>|";
  #endif

  /* make sure not empty string */
  if (!fname || strcmp(fname, "") == 0)
  {
    *err = mk_printf("The empty string is not a valid filename.");
    ret = FALSE;
  }

  /* validate filename chars */
  if (ret && find_index_in_string(badchars, basename) != -1)
  {
    *err = mk_printf("'%s' contains one or more invalid filename characters: %s",
		     fname, badchars);
    ret = FALSE;
  }
  free_string(basename);

  /* validate path */
  if (ret && validate_path_exists)
  {
    char *dirname = mk_path_dirname(fname, TRUE);
    if (strcmp(dirname, "") != 0 && !am_isdir(dirname))
    {
      *err = mk_printf("Invalid filename.  Directory '%s' does not exist.", dirname);
      ret = FALSE;
    }
    free_string(dirname);
  }

  return ret;
}

void remove_file(const char *fname,char **r_errmess)
{
  *r_errmess = NULL;
  if (remove(fname) == -1)
  {
      *r_errmess = mk_printf("Unable to remove '%s':%s", fname, 
			     strerror(errno));
  }
}

void remove_dir(const char *dname, bool remove_contents, char** err)
{
    int i;
    *err = NULL;

    if (rmdir(dname) == 0)
    {
	return;
    }
    else if (errno == ENOTEMPTY && remove_contents == TRUE)
    {
	/*remove subdirs*/
	string_array *files = am_get_dir_listing(dname, FALSE, TRUE, err);
	for (i = 0; i < string_array_size(files) && *err == NULL; i++)
	{
	    char *name = string_array_ref(files, i);
	    char *path = mk_join_path2(dname, name);

	    if (strcmp(name,".") == 0 || strcmp(name, "..") == 0)
	    {
		free_string(path);
		continue;
	    }
	    remove_dir(path, remove_contents, err);
	    free_string(path);
	}
	free_string_array(files);

	/*remove files*/
	files = am_get_dir_listing(dname, TRUE, FALSE, err);
	for (i = 0; i < string_array_size(files) && *err == NULL; i++)
	{
	    char *path = mk_join_path2(dname, string_array_ref(files, i));
	    remove_file(path, err);
	    free_string(path);
	}
	free_string_array(files);

	/*now try to remove the dir again*/
	if (*err == NULL && rmdir(dname) == 0)
	{
	    return;
	}
    }

    /*if we got to this point, there was an error*/
    if (*err == NULL)
    {
	*err = mk_printf("Unable to remove '%s':%s", dname, strerror(errno));
    }
}

/* Copy file named by f1 into file named by f2 */

bool copy_file(const char *f1,const char *f2)
{
  bool ret = TRUE;
  FILE *sin = NULL;
  FILE *sout = NULL;
  int c = 0;

  sin = safe_fopen(f1,"r");
  if (!sin) return FALSE;
  sout = safe_fopen(f2,"w");
  if (!sout) { 
    fclose(sin);
    return FALSE;
  }

  c = getc(sin);
  while ( c != EOF)
  {
    putc(c,sout);
    c = getc(sin);
  }

  fclose(sin);
  if (fclose(sout) != 0) {
    ret = FALSE;
  }
  return ret;
}

size_t get_file_num_rows(const char *fname)
{
  size_t count = 0;
  FILE *infile = fopen(fname, "r");
  if (infile){
    char line[BIG_ARRAY_SIZE];
    while (fgets(line, BIG_ARRAY_SIZE, infile)){
      if (strrchr(line, '\n')) count++;
    }
    fclose(infile);
  }
  return count;
}


/*
  return the characters up to the last path separator.  If keep_trail_sep
  is true, the trailing path separator is also returned.
  ex. /usr/bin/emacs => /usr/bin/

  If the path does not contain a separator, an empty string is returned.
*/
char *mk_path_dirname(const char *path, bool keep_trail_sep)
{
    char *str;
    char *dirname = NULL;
    char sep = am_get_path_sep();

    str = strrchr(path, sep);
    if (str)
    {
	dirname = AM_MALLOC_ARRAY(char, str - path + 2);
	strncpy(dirname, path, str - path + 1);
	dirname[str - path + 1] = '\0';
    }
    else
    { 
	dirname = mk_copy_string("");
    }
    if (keep_trail_sep == FALSE)
    {
	char *tmp = mk_path_rm_trail_sep(dirname);
	free_string(dirname);
	dirname = tmp;
    }

    return dirname;
}

/*
  return the characeters after the last '.' 
  ex. /tmp/info.txt.back => back
*/
char *mk_path_suffix(const char *path)
{
    char *str1, *str2;
    char *ext = NULL;
    str1 = strrchr(path, '.');
    str2 = strrchr(path, am_get_path_sep());
    if (str1 && str1 > str2) /*doesn't count if have path sep after last '.'*/
    {	
	str1++;
	if (*str1 != '\0')
	{
	    ext = mk_copy_string(str1);
	}
    }
    if (!ext)
    {
	ext = AM_MALLOC_ARRAY(char, 1);
	ext[0] = '\0';
    }
    return ext;
}

/* Return the characters after the last path separator.  If
   include_suffix is FALSE, then exclude the last '.' and everything
   after it.

   If the path ends in a separator, an empty string is returned.*/
char *mk_path_basename(const char *path, bool include_suffix)
{
    char *str;
    char *basename = NULL;
    char sep = am_get_path_sep();
    
    str = strrchr(path, sep);
    if (str)
    {
      str++;
      if (*str != '\0')
      {
	basename = mk_copy_string(str);
      }
      else
      {
	/*last char was sep.  Return empty string*/
	basename = mk_copy_string("");
      }
    }
    if (basename == NULL)
    {
	basename = mk_copy_string(path);
    }

    /* remove extension? */
    if (! include_suffix)
    {
      char *ext = strrchr(basename, '.');
      if (ext && ((ext - basename) > 0))
      {
	char *tmp = AM_MALLOC_ARRAY(char, ext - basename + 1);
	strncpy(tmp, basename, ext-basename);
        tmp[ext-basename] = '\0';
	free_string(basename);
	basename = tmp;
      }
    }
    return basename;
}


/*static*/ 
bool is_root_dir(const char *path)
{
    int len = strlen(path);
    char sep = am_get_path_sep();
    bool ret = FALSE;
 
    /* unix style */
    if (len == 1 && path[0] == sep) ret = TRUE;
    /* windows style */
    if (len == 3 && path[1] == ':' && path[2] == sep) ret = TRUE;

    return ret;
}

/*return a copy of path with the trailing slash (if any) removed.
  If the directory is '/' (or '\') or is of the form D:\, don't 
  remove the trailing slash*/
char *mk_path_rm_trail_sep(const char *path)
{
    int len = strlen(path);
    char sep = am_get_path_sep();
    char *ret = NULL;

    if (!is_root_dir(path) && len > 1 && path[len-1] == sep)
    {
      ret = AM_MALLOC_ARRAY(char, len);
      strncpy(ret, path, len-1);
      ret[len-1] = '\0';
    }
    else
    {
      ret = mk_copy_string(path);
    }
    return ret;
}

char am_get_path_sep(void)
{
    return get_sep_for_pathstyle(HERE_PATHSTYLE);
}

/*separator for the paths in the PATH environment variable string*/
char am_get_PATH_envstr_sep(void)
{
#if defined PC_PLATFORM
    return ';';
#elif defined UNIX_PLATFORM
    return ':';
#else 
    return ':';
#endif
}

/*joins path segments using the path separator*/
char *mk_join_path2(const char *path1, const char *path2)
{
    char *path1_nosep = mk_path_rm_trail_sep(path1);
    char sep = am_get_path_sep();
    char *full_path = NULL;
   
    if (strcmp(path1_nosep, "") == 0)
    {
	if (strcmp(path2, "") == 0)
	    full_path = mk_printf("");
	else
	    full_path = mk_copy_string(path2);
    }
    else
    {
	if (strcmp(path2, "") == 0)
	    full_path = mk_copy_string(path1_nosep);
	else if (path1_nosep[strlen(path1_nosep)-1] == sep)
	    /*root dir, don't insert extra sep*/
	    full_path = mk_printf("%s%s", path1_nosep, path2);
	else
	    full_path = mk_printf("%s%c%s", path1_nosep, sep, path2);
    }
    free_string(path1_nosep);

    return full_path;
}

char *mk_join_path(const string_array *path_strings)
{
    char *tmp;
    char *full_path = mk_copy_string("");
    int i;
    int size;

    if (!path_strings) return NULL;

    size = string_array_size(path_strings);

    for (i = 0; i < size; i++)
    {
	tmp = mk_join_path2(full_path, string_array_ref(path_strings, i));
	free_string(full_path);
	full_path = tmp;
    }

    return full_path;
}

/*splits path into segments*/
string_array *mk_split_path(const char *path)
{
    char *dirname = mk_path_rm_trail_sep(path);
    char *basename;
    char *tmp;
    string_array *path_list = mk_string_array(0);
    string_array *tmp_array;

    basename = mk_path_basename(dirname, TRUE);
    while(strlen(dirname) > 0)
    {
	if (strlen(basename) > 0)
	{
	    /*add the path segment*/
	    string_array_add(path_list, basename);
	    free_string(basename);
	}
	else if (strlen(dirname) > 0)
	{
	    /*no more segments*/
	    string_array_add(path_list, dirname);
	    break;
	}	    

	/*get the next dirname and basename*/
	tmp = dirname;
	dirname = mk_path_dirname(dirname, FALSE);
	basename = mk_path_basename(dirname, TRUE);
	free_string(tmp);
    }
    if (dirname) free_string(dirname);

    /*now have the path split, but in reverse order*/
    tmp_array = mk_reverse_string_array(path_list);
    free_string_array(path_list);
    path_list = tmp_array;

    return path_list;
}

char *mk_getcwd()
{
    char *cwd = NULL;
    char *tmp;
    int size = 256;
    
    /* get the current working directory*/
    tmp = AM_MALLOC_ARRAY(char, size);
    while (!getcwd(tmp, size) && errno == ERANGE)
    {
	AM_FREE_ARRAY(tmp, char, size);
	size *= 2;
	tmp = AM_MALLOC_ARRAY(char, size);
    }
    if (tmp)
    {
	cwd = mk_copy_string(tmp);
	AM_FREE_ARRAY(tmp, char, size);
    }
    return cwd;
}

/*return the absolute path to 'path'*/
char *mk_abspath(const char *path)
{
    char *orig_dir = NULL;
    char *abs_path = NULL;
    char *filename = NULL;
    char *path_only = mk_copy_string(path);

    /*if its a file, remove the filename*/
    if (am_isfile(path_only))
    {
	filename = mk_path_basename(path, TRUE);
	free_string(path_only);
	path_only = mk_path_dirname(path, FALSE);
    }

    /*save the current directory*/
    orig_dir = mk_getcwd();

    /*get the absolute path*/
    if (orig_dir && path_only)
    {
        char *tmp = path_only;
        path_only = mk_expand_tilde(tmp);
        free_string(tmp);

	if (chdir(path_only) == 0)
	{
	    abs_path = mk_getcwd();
	}
	else
	{
	    abs_path = mk_copy_string("");
	}
	chdir(orig_dir);
    }
    
    /*append the filename*/
    if (filename)
    {
	char *tmp = mk_join_path2(abs_path, filename);
	free_string(abs_path);
	abs_path = tmp;
    }
    
    if (filename)  free_string(filename);
    if (path_only) free_string(path_only);
    if (orig_dir)  free_string(orig_dir);

    return abs_path;
}

/*figure out the full path to an exe...
  - if the input string contains path separators, assume it is the full path
  - else look in the current dir (windows only - on unix even current dir
     should be in the user's path)
  - else look in the user's PATH for an exe with this name.
  - On Unix, if the found path is a symbolic link, resolve the link and find the actual path
  - if the exe does not exist, return NULL.
*/
char *mk_get_exe_path(const char *exe)
{
   
    char *path = NULL;
    char *pathbuf = NULL;
    int err = 0 , found_link = 0 ;
    int pathbufsize = 512;

    if (strchr(exe, am_get_path_sep()) && am_isfile((char *)exe))
    {
	path = mk_copy_string(exe);
    }
    else
    {
#if defined PC_PLATFORM
	/*look in cwd on windows*/
	path = mk_join_path2(".", exe);
	if (!am_isfile(path))
	{
	    free_string(path);
	    path = NULL;
	}
#endif
	if (!path)
	{
	    /*get the user's PATH */
	    char *path_str = getenv("PATH");
	    if (path_str)
	    {
		char *tmp = mk_printf("%c", am_get_PATH_envstr_sep());
		string_array *paths = mk_split_string(path_str, tmp);
		int i;

		free_string(tmp);
		for (i = 0; 
		     i < string_array_size(paths) && path == NULL ; 
		     i++)
		{

		  if( string_array_ref(paths,i) && (strlen(string_array_ref(paths,i)) > 0)  ){
		    path = mk_join_path2(string_array_ref(paths, i),
					 exe);
		    if (!am_isfile(path))
		    {
			free_string(path);
			path = NULL;
		    }		    
		  }
		}
		free_string_array(paths);
	    }
	    else
	    {
		fprintf(stderr, "PATH variable not set.\n");
	    }
	}
    }

#ifdef UNIX_PLATFORM
    if( path ){
      struct stat buf;
      if(lstat(path, &buf) == 0){
	if( (buf.st_mode & S_IFMT ) == S_IFLNK){/* check if its a symbolic link*/
	  int rc ;
	  int done = 0;
	  pathbuf = AM_MALLOC_ARRAY(char,pathbufsize);

	  while( !done ){ /* get the real path */
	    rc = readlink(path,pathbuf,pathbufsize);
	    if( rc == -1) { err = 1;done =1;}
	    if( rc == pathbufsize){
	      AM_FREE_ARRAY(pathbuf,char,pathbufsize);
	      pathbufsize = pathbufsize*2;
	      pathbuf = AM_MALLOC_ARRAY(char,pathbufsize);
	    }else{
	      pathbuf[rc] ='\0';
	      done =1;found_link = 1 ;
	    }
	  }/* while*/
	}/* is link */
      }else{
	err = 1;
      }
      
      if( ! err && found_link ) {
	if( pathbuf[0] != '/'){
	  char* dir_name = mk_path_dirname(path,TRUE);
	  free_string(path);
	  path = mk_strcat(dir_name,pathbuf,FALSE);
	  free_string(dir_name);
	}else{
	  free_string(path);
	  path = mk_copy_string(pathbuf);
	}

      } else if( err ) { 
	free_string(path);
	path = NULL;
      }

      if( pathbuf) AM_FREE_ARRAY(pathbuf,char,pathbufsize);
    }

#endif

    if (path != NULL)
    {
      char *abspath = mk_abspath(path);
      free_string(path);
      path = abspath;
    }
    
    return path;
}


string_array *
am_get_dir_listing(const char *dir, bool include_files, bool include_dirs,
		   char **err)
{
#ifdef PC_PLATFORM

    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
    string_array *names = NULL;
    char *full_dir = mk_join_path2(dir, "*");

    hFind = FindFirstFile(full_dir, &FindFileData);
    free_string(full_dir);

    if (hFind == INVALID_HANDLE_VALUE) 
    {
	char *str = strerror(errno);
	if (!str) str = "unknown error.";
	*err = mk_copy_string(str);
    } 
    else 
    {
	bool done = FALSE;
	names = mk_string_array(0);	

	while(!done && FindFileData.cFileName)
	{
	    char *filename = FindFileData.cFileName;
	    char *full_path = mk_join_path2(dir, filename);
	    
	    if (include_files && am_isfile(full_path))
		string_array_add(names, filename);
	    if (include_dirs && am_isdir(full_path))
		string_array_add(names, filename);
	    
	    free_string(full_path);
	    if (!FindNextFile(hFind, &FindFileData))
	    {
		done = TRUE;
	    }
	}
	FindClose(hFind);
    }

    return names;
    
#else /*UNIX*/

    DIR *p_dir;
    struct dirent *de = NULL;
    string_array *names = NULL;

    if (am_isdir((char *)dir) == FALSE)
    {
	*err = mk_printf("'%s' is not a valid directory.", dir);
	return NULL;
    }
    
    p_dir = opendir(dir);
    if (!p_dir)
    {
	char *str = strerror(errno);
	if (!str) str = "unknown error.";
	*err = mk_copy_string(str);
    }
    else
    {
	names = mk_string_array(0);
	de = readdir(p_dir);
	while(de)
	{
	    char *full_path = mk_join_path2(dir, de->d_name);
	    
	    if (include_files && am_isfile(full_path))
		string_array_add(names, de->d_name);
	    if (include_dirs && am_isdir(full_path))
		string_array_add(names, de->d_name);

	    free_string(full_path);
	    de = readdir(p_dir);
	}
	closedir(p_dir);
    }
    return names;
#endif
}


#if 0
static void am_file_test_code()
{
    char sep = am_get_path_sep();
    char *paths[6] = { "~/h/applic/",
		       "/home/sillyputty/",
		       "/home//junk/",
		       "..",
		       "/",
		       "./junk.cfg"};
    int i;
    char *tmp;
    bool flag = FALSE;

    printf("separator char is '%c'\n", sep); /*use 'sep' so no warning*/
    for (i = 0; i < 6; i++)
    {
	tmp = mk_path_dirname(paths[i], flag);
	tmp = mk_path_basename(paths[i], flag);
	tmp = mk_path_suffix(paths[i]);
	tmp = mk_path_rm_trail_sep(paths[i]);
	tmp = mk_abspath(paths[i]);
	
	/*test join, split*/
	{
	    string_array *sa = mk_split_path(paths[i]);
	    tmp = mk_join_path(sa);
	}
    }
}
#endif

/* return the full path to the user's tmp directory if specified.
   Implemented for Linux and Windows*/
char *mk_get_tmp_dir()
{
  char *ret = NULL;
  #ifdef PC_PLATFORM
    ret = getenv("TMP");
    if (!ret) ret = getenv("TEMP");
  #else
    ret = "/tmp";
  #endif

  if (ret) ret = mk_copy_string(ret);
  return ret;
}

/* return the full path to the user's home directory.  Implemented
   for Linux and Windows. */
char *mk_get_home_dir(char **err)
{
  char *home = NULL;
  char *var_HOME = getenv("HOME");
  char *set_vars_str = NULL;
  *err = NULL;

#ifdef PC_PLATFORM
  {
    char *var_HOMEDRIVE = getenv("HOMEDRIVE");
    char *var_HOMEPATH = getenv("HOMEPATH");

    if (var_HOME)
    {
      home = mk_copy_string(var_HOME);
      set_vars_str = mk_printf("HOME='%s'", var_HOME);
    }
    else if (var_HOMEDRIVE && var_HOMEPATH)
    {
      home = mk_printf("%s%c%s", var_HOMEDRIVE, am_get_path_sep(),
		       var_HOMEPATH);
      set_vars_str = mk_printf("HOMEDRIVE='%s', HOMEPATH='%s'",
			       var_HOMEDRIVE, var_HOMEPATH);
    }
    else
    {
      *err = mk_printf("Unable to locate home directory.  Environment variable "
		       "HOME is not defined and neither are both "
		       "HOMEDRIVE and HOMEPATH.");
    }
  }
#else
  if (var_HOME)
  {
    home = mk_copy_string(var_HOME);
    set_vars_str = mk_printf("HOME='%s'", var_HOME);
  }
  else
  {
    *err = mk_printf("Unable to locate home directory.  Environment variable "
		     "HOME is not defined.");
  }
#endif

  if (home)
  {
    char *tmp = home;
    home = mk_abspath(tmp);
    free_string(tmp);
    if (strcmp(home,"") == 0)
    {
      *err = mk_printf("Home directory is specified as %s but that directory "
		       "does not exist.", set_vars_str);
      free_string(home);
      home = NULL;
    }
  }
  if (set_vars_str) free_string(set_vars_str);

  return home;
}

char *mk_get_h_dir(const char *path)
{
  char *ret = NULL;
  char *path_abs = NULL;
  char *path_parent = NULL;
  char *gmake_dir = NULL;
  
  if (!am_isfile(path) && !am_isdir(path))
    return ret;

  path_abs = mk_abspath(path);
  path_parent = mk_path_dirname(path_abs, FALSE);
  free_string(path_abs);
  
  while(path_parent != "" && !is_root_dir(path_parent))
  {
    gmake_dir = mk_join_path2(path_parent, "gmake-magic");
    if (am_isdir(gmake_dir))
    {
      ret = mk_copy_string(path_parent);
      break;
    }
    else
    {
      char *tmp = path_parent;
      path_parent = mk_path_dirname(tmp, FALSE);
      free_string(tmp);
    }
    if (gmake_dir) free_string(gmake_dir);
    gmake_dir = NULL;
  }
  if (path_parent) free_string(path_parent);
  if (gmake_dir) free_string(gmake_dir);

  return ret;
}

char *mk_find_file_robust(const char *exe_dir,
			  const char *to_find,
			  const char *install_rel_path,
			  const char *h_rel_path,
			  char **err)
{
  char *ret = NULL;
  char *ret_install_path = NULL;
  char *ret_h_path = NULL;
  string_array *path_parts = NULL;
  char *exe_dir_clean = NULL;
  char *to_find_clean = NULL;
  char *install_path_clean = NULL;
  char *h_path_clean = NULL;
  *err = NULL;

  /*convert all to correct path sep  */
#ifdef PC_PLATFORM
  exe_dir_clean = mk_dospath_from_path(exe_dir);
  to_find_clean = mk_dospath_from_path(to_find);
  install_path_clean = mk_dospath_from_path(install_rel_path);
  h_path_clean = mk_dospath_from_path(h_rel_path);
#else
  exe_dir_clean = mk_path_from_dospath(exe_dir);
  to_find_clean = mk_path_from_dospath(to_find);
  install_path_clean = mk_path_from_dospath(install_rel_path);
  h_path_clean = mk_path_from_dospath(h_rel_path);
#endif

  /*see if 'exe_dir/install_rel_path/to_find' exists */
  path_parts = mk_string_array_3(exe_dir_clean, install_path_clean,
				 to_find_clean);
  ret_install_path = mk_join_path(path_parts);
  free_string_array(path_parts); path_parts = NULL;
  if (am_isfile(ret_install_path) || am_isdir(ret_install_path))
  {
    ret = mk_copy_string(ret_install_path);
  }

  /* else see if exe_dir is inside an auton h_dir */
  if (!ret)
  {
    char *tmp = mk_get_h_dir(exe_dir_clean);
    if (tmp)
    {
      path_parts = mk_string_array_3(tmp, h_path_clean, to_find_clean);
      free_string(tmp);
      ret_h_path = mk_join_path(path_parts);
      free_string_array(path_parts); path_parts = NULL;
      if (am_isfile(ret_h_path) || am_isdir(ret_h_path))
      {
        ret = mk_copy_string(ret_h_path);
      }
    }
  }

  if (!ret)
  {
    /*If we had an h/ dir, report ret_h_path as the missing file*/
    char *bad_path = ret_h_path ? ret_h_path : ret_install_path;
    if (bad_path)
    {
      *err = mk_printf("Unable to find '%s'", bad_path);
    }
  }

  free_string(exe_dir_clean);
  free_string(to_find_clean);
  free_string(install_path_clean);
  free_string(h_path_clean);
  if (ret_h_path) free_string(ret_h_path);
  if (ret_install_path) free_string(ret_install_path);
  if (path_parts) free_string_array(path_parts);

  return ret;
}



