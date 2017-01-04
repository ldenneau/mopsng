/*
   File:        param.h
   Author:      Ashwin Tengli, Jeanie Komarek
   Created:     Wed Oct  5 15:23:00 EDT 2005
   Description: header for generic parameter handling type

   Copyright (C) 2005 Carnegie Mellon University

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

#ifndef UTILS_PARAM_H
#define UTILS_PARAM_H


#include "am_string_array.h"
#include "genarray.h"
/*#include "aslAlg.h"*/


/* ---------------- Generic Parameter Handling Code --------------- 

This code provides a generic parameter handling type that can be used
by all Auton code to specify the parameters needed by the program, 
get user's values from the command line, validate the parameters,
and pass the parameter values down to the program's implementation.

-- Supported Parameter Types --------

  - integer        with optional min and max values
  - double         with optional min and max values
  - bool
  - string         with optional list of valid values
  - string list    with optional list of valid values
  - date
  - filename       with option to validate that the filename exists.

-- Usage Model ----------------------

  .. Implement a function like below and call it in main.c ....
  
  param_array *mk_param_array_for_this_algorithm() {
    ..... create param_array struct .......
    param_array *pa = mk_empty_param_array(); 

    ...define all the parameters your code requires...
       add_param_definition(pa, ...);
       add_param_definition(pa, ...);
       add_param_definition(pa, ...);
  }

  
    ...In main.c assign parameter values from command line or from other source like ASL...
    assign_params_from_args(pa, argc, argv);

    ... Validate the parameters assigned using...
    validate_param_values(pa, &err_msg);

    ...use the param_array struct to access the user's chosen values in
    your code...
    int val = get_param_val_int(pa, "num_its");

    :
    :
  

--------------------------------------------------------------------*/

typedef enum param_type {k_int = 0, k_double, k_bool, k_string, k_string_list,
		 k_date, k_file, k_err} param_type;
/* k_err enum is added to indicate a error: for example error in access param type in 
 * the function get_param_type
 * */




typedef struct param
{
  param_type m_type;   /* parameter type */
  char *m_name;        /* name of the parameter */
  char *m_shortname;   /* alternate short name for use in command line parse*/
  char *m_desc;        /* description of the parameter */
  char *m_def_val;     /* default value represented as a string */
  char *m_value;       /* assigned value represented as a string */

  bool  m_hidden;      /* hidden from user => not part of documentation.  Ok
			  to add or remove parameter from list any time. */
  
  /* for int and double types */
  bool m_has_min;          /* paramter has a minimum value */
  double m_min;            /* minimum valid value */
  bool m_val_can_equal_min;/* allow val >= min  (else val > min) */
  bool m_has_max;          /* parameter has a max value */
  double m_max;            /* maximum valid value */
  bool m_val_can_equal_max;/* allow val <= max  (else val < max) */

  /* for string and string list */
  string_array *m_choices; /* valid choices for the string or string list */

  /* for filename */
  bool m_must_exist;       /* the specified filename must exist to be valid */

}param;  

struct param_array
{
  generic_array *m_params;        /* array of parameters */
  /*string_array  *m_name_to_index;*/ /* for reverse lookup of parameters */

  bool m_ok_to_add_params; /* want to enforce: all params should be created 
			      before getting/setting values. */
};

/*needed to allow forward declaration of type datset without full 
  #include ds.h for the ASL C api*/
#ifndef PARAM_ARRAY_TYPEDEF_DEFINED
#define PARAM_ARRAY_TYPEDEF_DEFINED
typedef struct param_array param_array;
#endif

/* -------------------- List Creation/Destruction ------------------ */

param_array *mk_empty_param_array();

param_array *mk_copy_param_array(const param_array *pa);
    
void free_param_array(param_array *pa);

/* Add a new parameter definition.  If a parameter with the specified
   name already exists, the code will my_error.  Return TRUE if the
   parameter was added, FALSE otherwise.
*/
bool add_param_definition(param_array *pa, param_type t, const char *name,
			  const char *def_value, const char *description);

/* remove a parameter from the list.  Return TRUE if the element was 
   removed, FALSE if it was not part of the list. 
*/
bool remove_param_definition(param_array *pa, const char *name);


/* ------------------ Assign Parameter Values -----------------------*/

/* Assign parameter values from the command line.  The param_array
   should be initialized with the expected parameters the user can
   specify.  Return an error if there is a mismatch between the command
   line and the expected parameter list.
*/
bool assign_params_from_args(param_array *pa, int argc, char **argv, 
			     char **r_errmess);

/* Assign parameter values defined in an aslAlg structure */
/*bool assign_params_from_ASL(param_array *pa, aslAlg alg, char **r_errmess);*/

/* Make sure parameter values are valid */
bool validate_param_values(param_array *pa, char **r_errmess);
bool validate_param_by_name(param_array *pa, const char *name, char **r_errmess);

/* -------------------- Member variable access ---------------------*/

/*-- access list of parameter names --*/
string_array *mk_get_parameter_names(param_array *pa);

/*-- access parameter definition using name as key --*/
param_type get_param_type(param_array *pa, const char *name);

const char *get_param_shortname(param_array *pa, const char *name);
void set_param_shortname(param_array *pa, const char *name, 
			 const char *short_name);
const char *get_param_desc(param_array *pa, const char *name);
void set_param_desc(param_array *pa, const char *name, const char *desc);

const char *get_param_def_val(param_array *pa, const char *name);
void set_param_def_val(param_array *pa, const char *name, const char *val);

/* *** TODO: add rest of accessors *** */

/*-- access parameter value set by user using name as key --*/

/* access the string representation of any parameter type. */
const char *get_param_value_string(param_array *pa, const char *name); /* ???? any error message reqd */
void set_param_value_string(param_array *pa, const char *name,const char *val);

/* access integer or string (with choices) parameter types. 
   For a string with choices, return the index of the choice selected.*/
int get_param_value_int(param_array *pa, const char *name);
void set_param_value_int(param_array *pa, const char *name, int val);

/* access double parameter type */
double get_param_value_double(param_array *pa, const char *name);
void set_param_value_double(param_array *pa, const char *name, double val);

/* access bool parameter type */
bool get_param_value_bool(param_array *pa, const char *name);
void set_param_value_bool(param_array *pa, const char *name, bool val);

/* access string list parameter type */
const string_array *get_param_value_string_list(param_array *pa, 
						const char *name);
void set_param_value_string_list(param_array *pa, const char *name,
				 const string_array *val);
				 
/* access date parameter type */
void get_param_value_date(param_array *pa, const char *name,
                          int *year, int *month, int *day, char **r_errmess);
void set_param_value_date(param_array *pa, const char *name,
			  int year, int month, int day);

/* ----------------------Printing / Documentation --------------------------*/

void fprintf_param_array(FILE *s, 
		 const char *m1, const param_array *pa, const char *m2);
void fprintf_param(FILE *s, const char *m1, const param *p, const char *m2);

void fprint_param_array_documentation(FILE *s, param_array *pa);


/* ------------------------------------------------------------------------- */



#endif /* UTILS_PARAM_H */

