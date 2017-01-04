
/*
   File:        ammarep.h
   Author:      Andrew W. Moore
   Created:     Sun Jun 18 10:51:22 EDT 1995
   Description: Header for Explains current malloc situation

   Copyright (C) 1995, Andrew W. Moore

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


#ifndef AMMAREP_H
#define AMMAREP_H

#include "standard.h"
#include "ambs.h"
#include "amma.h"

void am_malloc_report(void);
bool am_malloc_report_polite( void);


#endif /* #ifndef AMMAREP_H */
