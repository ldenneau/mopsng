/* ----------------------------------------------------------------------------
 *                           * * * UNCLASSIFIED * * *
 * ----------------------------------------------------------------------------
 *
 * $Logfile: /Software/libraries/mops-interface-c/modc.h $
 *
 * ----------------------------------------------------------------------------
 *
 * $Workfile: modc.h $
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
 * $History: modc.h $
 * 
 * *****************  Version 6  *****************
 * User: Lisa A. Shannon Date: 9/21/04    Time: 3:39p
 * Updated in $/Software/libraries/mops-interface-c
 * added #include Utilities.h so that modcu_free() can be used
 * 
 * *****************  Version 5  *****************
 * User: Lisa A. Shannon Date: 9/10/04    Time: 4:08p
 * Updated in $/Software/libraries/mops-interface-c
 * modified #ifdef cplusplus to #Ifdef __cplusplus
 * __cplusplus is recognized by gcc and g++ where cplusplus (no
 * underscores) is only recognized by g++
 * 
 * *****************  Version 4  *****************
 * User: Wayne L. Smith Date: 8/12/04    Time: 8:57a
 * Updated in $/Software/libraries/mops-interface-c
 * Removed C++ style comments for C code.  Note that this does seem to be
 * supported in the target standard (C99), but it seems safer to avoid
 * them.
 * 
 * *****************  Version 3  *****************
 * User: Wayne L. Smith Date: 8/11/04    Time: 8:43a
 * Updated in $/Software/libraries/mops-interface-c
 * Updated name of file in "define".
 * 
 * *****************  Version 2  *****************
 * User: Wayne L. Smith Date: 8/10/04    Time: 7:15p
 * Updated in $/Software/libraries/mops-interface-c
 * Cleaned up documentation using groups.  Refactored all functions to use
 * a prefix specific to MOPS DC libraries.
 * 
 * *****************  Version 1  *****************
 * User: Wayne L. Smith Date: 8/10/04    Time: 5:55p
 * Created in $/Software/libraries/mops-interface-c
 * Single-point-of-entry include file.
 *
 */

#ifndef __MODC_H
#define __MODC_H

#ifdef __cplusplus 
extern "C"  {
#endif

#include "DetectionRef.h"
#include "OrbitRef.h"
#include "PairRef.h"
#include "ObservationRef.h"
#include "Utilities.h"
#include "VersionTools.h"

/** @file
  * This is the main include file for the MOPS-DC library.  This will include all
  * the necessary files.
  *
  * @version Revision<!--$$Revision: 14 $-->on<!--$$Date: 2004-12-09 09:55:35 -1000 (Thu, 09 Dec 2004) $-->
  * @author Wayne L. Smith,
  *         Last modified by: <!--$$Author: denneau $-->
  **/


#ifdef __cplusplus 
}
#endif

#endif /* __MODC_H */
