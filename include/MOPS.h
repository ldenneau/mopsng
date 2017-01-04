/* ----------------------------------------------------------------------------
 *                           * * * UNCLASSIFIED * * *
 * ----------------------------------------------------------------------------
 *
 * $Logfile: /Software/libraries/mops-interface-c/MOPS.h $
 *
 * ----------------------------------------------------------------------------
 *
 * $Workfile: MOPS.h $
 * $Revision: 14 $
 * $Date: 2004-12-09 09:55:35 -1000 (Thu, 09 Dec 2004) $
 * $Author: denneau $
 *
 * ----------------------------------------------------------------------------
 *
 * Copyright (c) 2004 Science Applications International Corporation
 *                    8301 Greensboro Drive
 *                    McLean, Virginia  22102-3600
 *
 * Unpublished: All rights reserved under copyright laws of the United States.
 *
 * ----------------------------------------------------------------------------
 *
 * Modification History:
 *
 * $History: MOPS.h $
 * 
 * *****************  Version 21  *****************
 * User: Wayne L. Smith Date: 9/15/04    Time: 1:59p
 * Updated in $/Software/libraries/mops-interface-c
 * Shuffled documentation around to hide the middle-tier indirection for
 * data structures.
 * 
 * *****************  Version 20  *****************
 * User: Wayne L. Smith Date: 9/14/04    Time: 1:48p
 * Updated in $/Software/libraries/mops-interface-c
 * Modifed int, float, double arrays to use straight types, rather than
 * abstracted pointer types (i.e., removed IntTP, FloatTP, DoubleTP).
 * 
 * *****************  Version 19  *****************
 * User: Lisa A. Shannon Date: 9/10/04    Time: 4:08p
 * Updated in $/Software/libraries/mops-interface-c
 * modified #ifdef cplusplus to #Ifdef __cplusplus
 * __cplusplus is recognized by gcc and g++ where cplusplus (no
 * underscores) is only recognized by g++
 * 
 * *****************  Version 18  *****************
 * User: Lisa A. Shannon Date: 9/02/04    Time: 9:37a
 * Updated in $/Software/libraries/mops-interface-c
 * changed CovMatrix to a double*
 * 
 * *****************  Version 17  *****************
 * User: Lisa A. Shannon Date: 8/19/04    Time: 9:10a
 * Updated in $/Software/libraries/mops-interface-c
 * Integrated versions
 * Removed ArrayTPs - Moved to MOPSStructs.h.
 * ArrayTPs are now structs that contain a count
 * 
 * *****************  Version 16  *****************
 * User: Wayne L. Smith Date: 8/12/04    Time: 8:55p
 * Updated in $/Software/libraries/mops-interface-c
 * Created lots of data types to better indicate expectations.
 * 
 * *****************  Version 15  *****************
 * User: Wayne L. Smith Date: 8/12/04    Time: 10:27a
 * Updated in $/Software/libraries/mops-interface-c
 * Switched to using HUGE_VAL for commonality between C++ and C.
 * 
 * *****************  Version 14  *****************
 * User: Wayne L. Smith Date: 8/12/04    Time: 9:11a
 * Updated in $/Software/libraries/mops-interface-c
 * Fixed BoolTP and SucceedTP typedefs for C.  The enum keyword is
 * required.
 * 
 * *****************  Version 13  *****************
 * User: Wayne L. Smith Date: 8/12/04    Time: 8:57a
 * Updated in $/Software/libraries/mops-interface-c
 * Removed C++ style comments for C code.  Note that this does seem to be
 * supported in the target standard (C99), but it seems safer to avoid
 * them.
 * 
 * *****************  Version 12  *****************
 * User: Wayne L. Smith Date: 8/12/04    Time: 8:39a
 * Updated in $/Software/libraries/mops-interface-c
 * Typo in type name.
 * 
 * *****************  Version 11  *****************
 * User: Wayne L. Smith Date: 8/11/04    Time: 6:36p
 * Updated in $/Software/libraries/mops-interface-c
 * Sanity check on documentation.  Fixed auto-linking for
 * cross-references.
 * 
 * *****************  Version 10  *****************
 * User: Wayne L. Smith Date: 8/11/04    Time: 5:54p
 * Updated in $/Software/libraries/mops-interface-c
 * Added data types to the appropriate group (otherwise, these only appear
 * in the documentation for the file itself).
 * 
 * *****************  Version 9  *****************
 * User: Lisa A. Shannon Date: 8/11/04    Time: 4:50p
 * Updated in $/Software/libraries/mops-interface-c
 * Must include stdlib.h for malloc
 * 
 * *****************  Version 8  *****************
 * User: Lisa A. Shannon Date: 8/11/04    Time: 3:00p
 * Updated in $/Software/libraries/mops-interface-c
 * added math.h for use with DOUBLE_NIL
 * 
 * *****************  Version 7  *****************
 * User: Lisa A. Shannon Date: 8/11/04    Time: 2:16p
 * Updated in $/Software/libraries/mops-interface-c
 * Added FIlterTP, TaxonomicTP, ModcPairTP, ModcDetTP, ModcObsTP,
 * ModcOrbitTP, PairArrayTP, DetectionArrayTP, ObservationArrayTP,
 * OrbitArrayTP
 * 
 * *****************  Version 6  *****************
 * User: Wayne L. Smith Date: 8/11/04    Time: 8:43a
 * Updated in $/Software/libraries/mops-interface-c
 * Added comment regarding the load order for this file.  The Bool enum
 * clobbers a global enum loaded by OCCI.
 * 
 * *****************  Version 5  *****************
 * User: Wayne L. Smith Date: 8/10/04    Time: 5:48p
 * Updated in $/Software/libraries/mops-interface-c
 * Cleaned up doxygen documentation.
 * 
 */


#ifndef __MOPS_H
#define __MOPS_H

#ifdef __cplusplus 
extern "C"  {
#endif

#include <math.h>   /* required for DOUBLE_NIL */
#include <stdlib.h> /* required for DOUBLE_NIL */
#include <stdio.h>

/** @file
  *
  * Common type definitions.
  *
  * @version Revision<!--$$Revision: 14 $-->on<!--$$Date: 2004-12-09 09:55:35 -1000 (Thu, 09 Dec 2004) $-->
  * @author Lisa A. Shannon,
  *         Last modified by: <!--$$Author: denneau $-->
  **/

/** @addtogroup types
  * @{
  * @copydoc MOPS.h
  **/

/* TODO - LAS - make this a TYPDEDF for MOPSDCBusinessLogic::NIL_DOUBLE */

/** Used to indicate a missing value when passed as an argument.
  **/
#define DOUBLE_NIL ((double)((HUGE_VAL-1)*(-1)))

/** Used to indicate the maximum number of covariance matrix values.
  * @see ::CovMatrixTP
  **/
#define COV_MATRIX_SIZE 15

/** @name MOPS-DC Object Pointers
  * MOPS-DC object pointers that are used to manipulate the objects.  This
  * includes storage, retrieval, and data field access.
  * @{
  **/

/** A pointer to a MOPS-DC detection object.  A MOPS-DC pointer is used to 
  *   -# allow the C++ Detection object to be passed to C code, and
  *   -# protect the object from inappropriate modification.
  **/
typedef const long * 	ModcDetTP;

/** A pointer to a MOPS-DC orbit object.  A MOPS-DC pointer is used to 
  *   -# allow the C++ Orbit object to be passed to C code, and
  *   -# protect the object from inappropriate modification.
  **/
typedef const long * 	ModcOrbitTP;

/** A pointer to a MOPS-DC observation (metadata) object.  A MOPS-DC pointer
  * is used to 
  *   -# allow the C++ Observation object to be passed to C code, and
  *   -# protect the object from inappropriate modification.
  **/
typedef const long * 	ModcObsTP;

/** A pointer to a MOPS-DC detection-pair object.  A MOPS-DC pointer is used
  * to 
  *   -# allow the C++ Pair object to be passed to C code, and
  *   -# protect the object from inappropriate modification.
  **/
typedef const long * 	ModcPairTP;

/** @} */

/** @name Convenience Types
  * Types used as arguments and return values within the MOPS-DC API.
  * @{
  **/

/** Used to indicate a string (i.e., pointer to \c char). 
  **/ 
typedef char* 		StringTP;


/** Used to indicate an observational filter.
  **/ 
typedef StringTP	FilterTP;

/** Used to indicate the Taxonomic Type of an orbital body.
  **/ 
typedef StringTP	TaxonomicTP;

/** Used to indicate an observational survey mode.
  **/ 
typedef StringTP	SurveyTP;

/** Used to indicate an IAU/MPC observatory code.
  **/ 
typedef StringTP ObservatoryTP;

/** Used to indicate the covariance matrix of an orbit.
  * @see ::COV_MATRIX_SIZE
  **/ 
typedef double*	CovMatrixTP;

/** Used to indicate a Modified Julian Date (a \c double).
  **/ 
typedef double MjdTP;

/** Used to indicate a unique identification number.
  **/
typedef long IdTP;

/** Used to indicate bit flags for orbits and detections.
  * @todo Possibly use distinct types for detection flags and orbit flags.
  **/
typedef int FlagTP;

/** @} */

/** An enumeration used to simplify the use of boolean values.
  **/
enum Bool
{
	True = 0, /**< True boolean value. */
	False = 1 /**< False boolean value. */
};

/** The Boolean type.
  **/
typedef enum Bool BoolTP;

/** Enumeration of the allowable return types for Success/Failure operations.
  **/
enum Succeed 
{
	Success = 0, /**< Success */
	Fail = -1	 /**< Failure */	
};

/** Allowable Success types. */
typedef enum Succeed SucceedTP;

/** Initializes the MOPS-DC API.  This file must contain (at least)
  * the minimum connectivity information.
  *
  * @param fileName  The file containing the initialization information.
  *
  * @return ::Success if initialization was successful, ::Fail otherwise.
  *
  * @ingroup utils
  **/
SucceedTP modc_initialize(StringTP fileName);

/** @} */

#ifdef __cplusplus 
}
#endif

#endif
