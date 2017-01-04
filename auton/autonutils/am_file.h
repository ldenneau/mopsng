/* 	File: am_file.h
 * 	Author(s): Andrew Moore, Pat Gunn
 * 	Date: 31 July 2003
 * 	Purpose: Store things related to file manipulation, e.g.
 *               wrapped fopen/open functions, fcntl stuff, etc.
 *               Also contains code for manipulating file names.
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

#ifndef AM_FILE_H
#define AM_FILE_H

#include "standard.h"
#include "ambs.h"
#include "am_string_array.h"

/* 	Unix pathstyles are native for auton applications. Here are some things
	that convert windows style filenames to unix-style, and vice versa.
	We do not guarantee that platform-specific things, like drive letters,
	are preserved. Sorry. If this is important, maybe we'll structify
	this at some point. */
char* mk_dospath_from_path(const char* path);
char* mk_path_from_dospath(const char* path);

/* A wrapper for your system's mkdir() function. 
	   TRUE for success, FALSE for error. */
int am_mkdir(const char* filename);

/*wrapper for getcwd that handles string allocation*/
char *mk_getcwd(void);

/* Makes a directory, making all its parents as needed.
		FALSE if it fails. It does consider it a
		success if the directory already exists.  */
bool am_mkdir_deeply(const char* pathname);
bool am_mkpath_for(char* filename);

/* Returns true if the provided path is a file or directory, as appropriate. */
bool am_isdir(const char* path);
bool am_isfile(const char* path);

/* Returns true if the path is writable */
bool am_iswritable(const char *path);

/* Rename a file or directory */
bool am_rename_file(const char* old_file, const char *new_file);

/* Remove a file. */
void remove_file(const char* fname, char** r_errmess);
void remove_dir(const char *dname, bool remove_contents, char** err);

/* Copy a file. */
bool copy_file(const char *f1,const char *f2);

/* how many rows in a file? */
size_t get_file_num_rows(const char *fname);

/* Get a listing of all the files and/or directories in the specified dir*/
string_array *am_get_dir_listing(const char *dir, 
				 bool include_files, bool include_dirs,
				 char **err);

/* Legacy functions that need to be rewritten to use the stat() interface.
   Same function as
   am_isfile. Note that on some Unices, you can open a directory as a file,
   and on others you cannot. Caveat Emptor! */
bool filenames_exist(string_array* filenames);
bool file_exists(const char* fname);

/* Some characters are not valid in filenames - differs by OS */
bool is_valid_filename(const char *fname, bool validate_path_exists, 
		       char **err);

/********** filename manipulation routines ************/

/*return the path separator for this platform*/
char am_get_path_sep(void);

/*separator for the paths in the PATH environment variable string*/
char am_get_PATH_envstr_sep(void);

/*
  return the characters up to the last path separator.  If keep_trail_sep
  is true, the trailing path separator is also returned.
  ex. /usr/bin/emacs => /usr/bin/

  If the path does not contain a separator, an empty string is returned.
*/
char *mk_path_dirname(const char *path, bool keep_trail_sep);

/* Return the characters after the last path separator.  If
   include_suffix is FALSE, then exclude the last '.' and everything
   after it.

   If the path ends in a separator, an empty string is returned.*/
char *mk_path_basename(const char *path, bool include_suffix);

/*
  return the characeters after the last '.' 
  ex. /tmp/info.txt.back => back
*/
char *mk_path_suffix(const char *path);

/*return a copy of path with the trailing slash (if any) removed*/  
char *mk_path_rm_trail_sep(const char *path);

/*joins path segments using the path separator*/
char *mk_join_path2(const char *path1, const char *path2);
char *mk_join_path(const string_array *path_strings);

/*splits path (full path to a file) into segments*/
string_array *mk_split_path(const char *path);

/*return the absolute path to 'path'.  If no such directory exists,
  return the empty string.
*/
char *mk_abspath(const char *path);

/*figure out the full path to an exe...
  - if the input string contains path separators, assume it is the full path
  - else look in the current dir (windows only - on unix even current dir
     should be in the user's path)
  - else look in the user's PATH for an exe with this name.
  - if the exe does not exist, return NULL.
*/
char *mk_get_exe_path(const char *exe);

/* return the full path to the user's home directory.  Implemented
   for Linux and Windows. */
char *mk_get_home_dir(char **err);

/* return the full path to the user's tmp directory if specified.
   Implemented for Linux and Windows*/
char *mk_get_tmp_dir(void);

/* Decide if 'path' is inside an auton 'h' dir by searching up the 
   directory tree until gmake-magic is found.
*/
char *mk_get_h_dir(const char *path);

/*
   - 'exe_dir' should be the directory the software is running from (as 
     returned by mk_get_exe_path(argv[0]))
   - 'to_find' should be a relative name or dir to find
   - 'install_rel_path' - relative path to search for 'to_find' in 
      the shipped version of the software
   - 'h_rel_path' - relative path to search for 'to_find' in 
     an auton h dir (for auton internal testing)
     (h/ dir is the value returned by mk_get_h_dir(exe_dir); )
   
     Return 'exe_dir/install_rel_path/to_find' if it exists
     Else Return '<h_dir>/h_rel_path/to_find' if it exists
     Else Return ''
   
   - The relative paths specified can be in either dos or unix format
   - The path returned will always be the correct format for the platform
     the software is running on

   Example:  exe_dir = 
             to_find = ASL_algrun
             install_rel_path = ./
             h_rel_path = ./ASL_algrun/
   
   If we are running from $HOME/h/ASL_gui/ will return 
     $HOME/h/ASL_algrun/ASL_algrun
   If we are running from /opt/ASL/ will return
     /opt/ASL/ASL_algrun

  - If the file was not found, create an error message stating the 
    location where we thought the file should be.
*/
char *mk_find_file_robust(const char *exe_dir,
			  const char *to_find,
			  const char *install_rel_path,
			  const char *h_rel_path,
			  char **err);


/*****************************************************/

#endif /* AM_FILE_H */ 

