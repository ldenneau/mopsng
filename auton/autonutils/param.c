/*
   File:        param.c
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

#include "param.h"
#include "amma.h"
#include "ambs.h"
#include "am_string.h"
#include <string.h>
#include <errno.h>
/*#include <limits.h>*/

/* ------------- private (static) functions -------------------------*/

static param *mk_param(param_type t, const char *name, 
		const char *def_value, const char *description) 
{
  param *p = AM_MALLOC(param);

  p->m_type = t;
  p->m_name = mk_copy_string(name);
  p->m_def_val = mk_copy_string(def_value);
  p->m_desc = mk_copy_string(description);

  p->m_shortname = NULL;
  p->m_value = NULL;
  p->m_hidden = FALSE;
  p->m_has_min = FALSE;
  p->m_min = 0;
  p->m_val_can_equal_min = FALSE;
  p->m_has_max = FALSE;
  p->m_max = 0;
  p->m_val_can_equal_max = FALSE;
  p->m_choices = NULL;
  p->m_must_exist = FALSE; /* ???? */

  return p;
}


static void free_param(param *p)
{
  free_string(p->m_name);
  if (p->m_shortname)
    free_string(p->m_shortname);
  if (p->m_desc)
    free_string(p->m_desc);
  if (p->m_def_val)
    free_string(p->m_def_val);
  if (p->m_value)
    free_string(p->m_value);
  if (p->m_choices)
    free_string_array(p->m_choices);

  AM_FREE(p,param);

}

static param *mk_copy_param(param *p) 
{
  param *newp = AM_MALLOC(param);

  newp->m_type = p->m_type;
  newp->m_name = mk_copy_string(p->m_name);
  newp->m_def_val = mk_copy_string(p->m_def_val);
  newp->m_desc = mk_copy_string(p->m_desc);

  newp->m_shortname = mk_copy_string(p->m_shortname);
  newp->m_value = mk_copy_string(p->m_value);
  newp->m_hidden = p->m_hidden;
  newp->m_has_min = p->m_has_min;
  newp->m_min = p->m_min;
  newp->m_val_can_equal_min = p->m_val_can_equal_min;
  newp->m_has_max = p->m_has_max;
  newp->m_max = p->m_max;
  newp->m_val_can_equal_max = p->m_val_can_equal_max;
  if (p->m_choices)
      newp->m_choices = mk_copy_string_array(p->m_choices);
  else
      newp->m_choices = NULL;
  newp->m_must_exist = p->m_must_exist;

  return newp;
}

static int find_index_of_param_by_name(param_array *pa,const char *name)
{
  int i;
  int size = generic_array_size(pa->m_params);
  bool found = FALSE;
  param *p = NULL;

  i=0;
  while (!found && i < size) 
  {
    p = (param *)generic_array_ref(pa->m_params,i);

    if (strcmp(name,p->m_name) == 0)
      found = TRUE;
    
    i++;
  }
  
  if (found)
    i--;
  else
    i=-1;

  return i;
}



static int find_index_of_param_by_shortname(param_array *pa,const char *sname)
{
  int i;
  int size = generic_array_size(pa->m_params);
  bool found = FALSE;
  param *p = NULL;

  i=0;
  while (!found && i < size) 
  {
    p = (param *)generic_array_ref(pa->m_params,i);

    if (p->m_shortname)
      if (strcmp(sname,p->m_shortname) == 0)
        found = TRUE;
    
    i++;
  }
  
  if (found)
    i--;
  else
    i=-1;

  return i;
}



static param *get_param_by_name(param_array *pa, const char *name)
{
  param *p = NULL;
  int indx;

  indx = find_index_of_param_by_name(pa,name);

  if (indx >= 0)
    p = (param *) generic_array_ref(pa->m_params,indx);

  return p;
}

static param *get_param_by_shortname(param_array *pa, const char *sname)
{
  param *p = NULL;
  int indx;
  
  indx = find_index_of_param_by_shortname(pa,sname);

  if (indx >= 0)
    p = (param *) generic_array_ref(pa->m_params,indx);

  return p;
}
  






/* -------------- Implementation ------------------------------------*/

/* -------------------- List Creation/Destruction ------------------ */


void void_free_param(void *data) 
{
  free_param((param *) data);
}

void *void_mk_copy_param(void *data) 
{
  return (void *) mk_copy_param((param *) data);
}


param_array *mk_empty_param_array() 
{
  param_array *pa = AM_MALLOC(param_array);
  
  pa -> m_params = mk_empty_generic_array(void_free_param,
                                          void_mk_copy_param,
                                          NULL);
  /*pa -> m_name_to_index = mk_string_array(0);*/
  
  pa -> m_ok_to_add_params = TRUE;
  
  return pa;
}

param_array *mk_copy_param_array(const param_array *pa) 
{
  param_array *pa_new = AM_MALLOC(param_array);
  pa_new -> m_params = mk_copy_generic_array(pa->m_params);
  pa_new -> m_ok_to_add_params = pa -> m_ok_to_add_params;

  return pa_new;
}

  

void free_param_array(param_array *pa) 
{
  free_generic_array(pa->m_params);
  /*free_string_array(pa->m_name_to_index);*/

  AM_FREE(pa,param_array);
}


static bool add_to_param_array(param_array *pa,param *p)
{
  bool ret = TRUE;
  
  if (find_index_of_param_by_name(pa,p->m_name) >= 0)
    ret = FALSE;
  else 
  {
    /*if (generic_array_size(pa->m_params) != string_array_size(pa->m_name_to_index))
      my_error("Size of generic array array with string array size: add_to_param_array");*/

    add_pointer_to_generic_array(pa->m_params,(void *) p);

    /*add_to_string_array(pa->m_name_to_index,p->m_name);*/
  }

  return ret;
}



/* Add a new parameter definition.  If a parameter with the specified
   name already exists, the code will my_error.  Return TRUE if the
   parameter was added, FALSE otherwise.
*/
bool add_param_definition(param_array *pa, param_type t, const char *name,
			  const char *def_value, const char *description)
{
  param *p = mk_param(t,name,def_value,description);
  if (!pa->m_ok_to_add_params) my_error("Cannot add more params: add_param_definition");
  
  return add_to_param_array(pa, p);
}


/* remove a parameter from the list.  Return TRUE if the element was 
   removed, FALSE if it was not part of the list. 
*/
bool remove_param_definition(param_array *pa, const char *name)
{ 
  int indx;
  bool ret=FALSE;

  indx = find_index_of_param_by_name(pa,name);

  if (indx >= 0) {
    generic_array_remove(pa->m_params,indx);
    /*string_array_remove(pa->m_name_to_index,indx);*/
    ret=TRUE;
  }

  return ret;
}




/* ------------- Param value set/get functions ----------------------*/
  
const char *get_param_value_string(param_array *pa, const char *name)
{
  const char *val;
  param *p = get_param_by_name(pa, name);
  
  if (!p) p = get_param_by_shortname(pa, name); 
  if (!p) my_errorf("Param not found in param_array: get_param_value_string param=%s",name);
  
  if (p->m_value)
  {
    /*not null => value is set */
    val = p->m_value;
  }
  else
  {
    /*null => value not set.  Use default.*/
    val = p->m_def_val;
  }
  
  
  return val;
}

void set_param_value_string(param_array *pa, const char *name,const char *val)
{
  param *p = get_param_by_name(pa, name);  
  
  if (!p) p = get_param_by_shortname(pa, name); /* handles case where called
						   from command line using
						   short param name.  This
						   is the only place we need
						   to worry about checking
						   for short name.*/
  if (!p) printf("WARNING: param not found in param_array: set_param_value_string  param=%s",name);
  else
  {
    if (p->m_value)
    {
      free_string(p->m_value);
      p->m_value = NULL;
    }
    if (val)
    {
      p->m_value = mk_copy_string(val);
    }
  }

}

bool convert_param_int(const char *str_val, int *val, char **r_errmess)
{
  long int lval = 0;
  char *endptr = NULL;
  *val = -1;
  
  lval = strtol(str_val, &endptr, 10);
  if( errno == ERANGE) {
    *r_errmess = mk_printf("value out of range: convert_param_int param value_string=%s\n",str_val);
    return FALSE;
  }
  
  if (!endptr || strcmp(endptr, "") != 0)
  {
    *r_errmess = mk_printf("error in converting string to integer - non integer characters in string: convert_param_int value_string=%s\n",str_val);
    return FALSE;
  } 
  else
  {
    if (lval < INT_MAX) 
      *val = (int) lval;
    else 
    {
      *r_errmess = mk_printf("number greater than INT_MAX: convert_param_int value_string=%s\n",str_val);
      return FALSE;
    }
  }

  return TRUE;
}


int get_param_value_int(param_array *pa, const char *name)
{
  int val;    
  const char *str_val = get_param_value_string(pa, name);
  char *err_msg = NULL;
  
  convert_param_int(str_val,&val,&err_msg);

  if (err_msg)
    free_string(err_msg);  
  
  return val;
}

void set_param_value_int(param_array *pa, const char *name, int val)
{
  /* convert to a string */
  char *str_val = mk_printf("%d", val);
  set_param_value_string(pa, name, str_val);
  free_string(str_val);
}

bool convert_param_double(const char *str_val, double *val, char **r_errmess)
{
  char *endptr = NULL;
  
  *val = strtod(str_val, &endptr);

  if( errno == ERANGE) 
  {
    *r_errmess = mk_printf("Overflow or underflow occurred: convert_param_double param value_string=%s\n",str_val);
    return FALSE;
  }

  if (endptr) 
  {
      if (strlen(endptr)!=0) 
      {
          *r_errmess = mk_printf("unable to convert value to double: convert_param_double value_string=%s found junk characters in the end=%s\n",str_val,endptr);
          return FALSE;
      }
  }

  return TRUE;



}

/* access double parameter type */
double get_param_value_double(param_array *pa, const char *name)
{
  char *err_msg=NULL;

  const char *str_val = get_param_value_string(pa, name);
  double val;
  
  convert_param_double(str_val,&val,&err_msg);

  if (err_msg)
    free_string(err_msg);
  
  return val;
}


void set_param_value_double(param_array *pa, const char *name, double val)
{
  /* convert to a string */
  char *str_val = mk_printf("%f", val);
  set_param_value_string(pa, name, str_val);
  free_string(str_val);
}


bool convert_param_bool(const char *str_val, bool *val, char **r_errmess)
{
  bool r_ok=FALSE;
  
  *val = FALSE;

  if ( str_val != NULL && str_val[0] != '\0' )
  {
    char c = str_val[0];
    if ( c >= 'a' && c <= 'z' ) c += 'A' - 'a';
    if ( c == 'Y' || c == 'T' || c == '1' )
    {
      r_ok = TRUE;
      *val = TRUE;
    }
    if ( c == 'N' || c == 'F' || c == '0' )
    {
      r_ok = TRUE;
      *val = FALSE;
    }
  }


  if (!r_ok)
  {
    *r_errmess = mk_printf("error in get_param_value_bool param value_string=%s\n",str_val);
    return FALSE;
  }
  
  return TRUE;


}


/* access bool parameter type */
bool get_param_value_bool(param_array *pa, const char *name)
{
  const char *str_val = get_param_value_string(pa, name);
  bool val;
  char *err_msg=NULL;

  convert_param_bool(str_val,&val,&err_msg);
  if (err_msg)
    free_string(err_msg);
  
  return val;
}



void set_param_value_bool(param_array *pa, const char *name, bool val)
{
  /* convert to a string */
  char *str_val;
  
  if (val)
    str_val = mk_printf("true");
  else 
    str_val = mk_printf("false");

  set_param_value_string(pa, name, str_val);
}


/* access string list parameter type */
const string_array *get_param_value_string_list(param_array *pa, const char *name)
{


  return NULL;
}



void set_param_value_string_list(param_array *pa, const char *name,const string_array *val)
{



}




/* access date parameter type */
void get_param_value_date(param_array *pa, const char *name,int *year, int *month, int *day, char **r_errmess) 
{




}



void set_param_value_date(param_array *pa, const char *name,int year, int month, int day)
{




}








/* -------------------- Member variable access ---------------------*/

/*-- access list of parameter names --*/
string_array *mk_get_parameter_names(param_array *pa) 
{
  int size = generic_array_size(pa->m_params);
  string_array *names = mk_string_array(size);
  int i;
  param *p = NULL;

  for (i=0; i < size; i++)
  {
    p = generic_array_ref(pa->m_params,i);
    string_array_set(names,i,p->m_name);
  }
  
  return names;
}

/*-- access parameter definition using name as key --*/
param_type get_param_type(param_array *pa, const char *name)
{
  param_type t = k_err;
  
  param *p = get_param_by_name(pa, name);

  if (p)
    t = p->m_type;
  else 
    my_errorf("param not present in param array: get_param_type param=%s",name);

  return t;
}

const char *get_param_shortname(param_array *pa, const char *name) 
{
  char *sname = NULL;
  
  param *p = get_param_by_name(pa, name);

  if (p)
    sname = p->m_shortname;
  else
    my_errorf("param not present in param array: get_param_shortname param=%s",name);

  return sname;
}


void set_param_shortname(param_array *pa, const char *name, 
			 const char *short_name)
{
  param *p = get_param_by_name(pa, name);
  
  if (p) {
    if (p->m_shortname)
      free_string(p->m_shortname);
    p->m_shortname = mk_copy_string(short_name);
  }
  else
    my_errorf("param not present in param array: set_param_shortname param=%s",name);
}



const char *get_param_desc(param_array *pa, const char *name)
{
  char *desc = NULL;
  
  param *p = get_param_by_name(pa, name);

  if (p)
    desc = p->m_desc;
  else
    my_error("param not present in param array: get_param_desc");

  return desc;
}


void set_param_desc(param_array *pa, const char *name, const char *desc)
{
  param *p = get_param_by_name(pa, name);
  
  if (p) {
    if (p->m_desc)
      free_string(p->m_desc);
    p->m_desc = mk_copy_string(desc);
  }
  else
    my_errorf("param not present in param array: set_param_desc param=%s",name);
}

const char *get_param_def_val(param_array *pa, const char *name)
{
  char *def_val = NULL;
  
  param *p = get_param_by_name(pa, name);

  if (p)
    def_val = p->m_def_val;
  else
    my_errorf("param not present in param array: get_param_def_val param=%s",name);

  return def_val;
}



void set_param_def_val(param_array *pa, const char *name, const char *val)
{
  param *p = get_param_by_name(pa, name);
  
  if (p) {
    if (p->m_def_val)
      free_string(p->m_def_val);
    p->m_def_val = mk_copy_string(val);
  }
  else
    my_errorf("param not present in param array: set_param_def_val param=%s",name);
}

/* *** TODO: add rest of accessors *** */







/* ------------------ Assign Parameter Values -----------------------*/


/* Assign parameter values from the command line.  The param_array
   should be initialized with the expected parameters the user can
   specify.  Return an error if there is a mismatch between the command
   line and the expected parameter list.
*/
bool assign_params_from_args(param_array *pa, int argc, char **argv, 
			     char **r_errmess)
{
  int i;
  bool ret = TRUE;

  
  my_assert(*r_errmess == NULL);
  
  for (i = 1; i < argc; i+=2)
  {
    char *key;
    char *val;
    if (i+1 >= argc) 
    {
      *r_errmess = mk_printf("parameter %s has no key associated with it.",argv[i]);
      ret =FALSE;
    } 
    else 
    {
      key = argv[i];
      val = argv[i+1];
      set_param_value_string(pa, key, val);
    }
  }

  /* once this is called, its no longer ok to add new parameters */
  /* ???? */
  pa->m_ok_to_add_params = FALSE;

  return ret;
}

/* Assign parameter values defined in an aslAlg structure */
/*bool assign_params_from_ASL(param_array *pa, aslAlg alg, char **r_errmess)
{





}*/


static bool validate_param(param *p, char **r_errmess)
{
  bool ret=TRUE;
  
  my_assert(*r_errmess == NULL);

  switch (p->m_type)
  {
    case k_int:
      {
        int val;
        ret = convert_param_int(p->m_value,&val,r_errmess);
        if (ret)
        {
          if (p->m_has_min && 
              ( ((double)val < p->m_min) || ((double)val == p->m_min && !p->m_val_can_equal_min) ) )
          {
            *r_errmess = mk_printf("param %s does not satisfy minimum value condition\n",p->m_name);
            ret = FALSE;
          } 
          else if (p->m_has_max &&
              ( ((double)val > p->m_max) || ((double)val == p->m_max && !p->m_val_can_equal_max) ) )
          {
            *r_errmess = mk_printf("param %s does not satisfy maximum value condition\n",p->m_name);
            ret = FALSE;
          }
        }
      }
      break;    
    case k_double:
      {
        double val;
        ret = convert_param_double(p->m_value,&val,r_errmess);
        if (ret)
        {
          if (p->m_has_min && 
              ( (val < p->m_min) || (val == p->m_min && !p->m_val_can_equal_min) ) )
          {
            *r_errmess = mk_printf("param %s does not satisfy minimum value condition\n",p->m_name);
            ret = FALSE;
          } 
          else if (p->m_has_max &&
              ( (val > p->m_max) || (val == p->m_max && !p->m_val_can_equal_max) ) )
          {
            *r_errmess = mk_printf("param %s does not satisfy maximum value condition\n",p->m_name);
            ret = FALSE;
          }
        }
      }
      break;
    case k_bool:
      {
        bool val;
        ret = convert_param_bool(p->m_value,&val,r_errmess);

      }
      break;
    case k_string:
      {        

      } 
      break;
    case k_string_list:
      break;
    case k_date:
      break;
    case k_file:
      break;
    default:
      {
        *r_errmess = mk_printf("undefined type of param %s\n",p->m_name);
        ret = FALSE;
      }
  }

  return ret;
}

/* Make sure parameter values are valid */
bool validate_param_values(param_array *pa, char **r_errmess)
{
  int i=0;
  param *p=NULL;
  int size = generic_array_size(pa->m_params);
  bool ret = TRUE;
    
  while (i < size && ret) 
  {
    p = (param *) generic_array_ref(pa->m_params,i);
    if (p->m_value) 
    {
      ret = validate_param(p,r_errmess);
    }
    i++;
  }

  return ret;
}


bool validate_param_by_name(param_array *pa, const char *name, char **r_errmess)
{
  param *p = get_param_by_name(pa,name);

  if (p->m_value)
    return validate_param(p,r_errmess);
  else
    return FALSE;
}





/* ----------------------Printing / Documentation --------------------------*/


void fprintf_param_array(FILE *s, 
		 const char *m1, const param_array *pa, const char *m2)
{





}


void fprintf_param(FILE *s, const char *m1, const param *p, const char *m2)
{










}



void fprint_param_array_documentation(FILE *s, param_array *pa)
{
 




}



