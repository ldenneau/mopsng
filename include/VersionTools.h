/* ----------------------------------------------------------------------------
 *                           * * * UNCLASSIFIED * * *
 * ----------------------------------------------------------------------------
 *
 * $Logfile: /Software/libraries/mops-interface-c/VersionTools.h $
 *
 * ----------------------------------------------------------------------------
 *
 * $Workfile: VersionTools.h $
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
 * $History: VersionTools.h $
 * 
 * *****************  Version 8  *****************
 * User: Wayne L. Smith Date: 9/18/04    Time: 6:27p
 * Updated in $/Software/libraries/mops-interface-c
 * Updated documentation.
 * 
 * *****************  Version 7  *****************
 * User: Lisa A. Shannon Date: 9/10/04    Time: 4:08p
 * Updated in $/Software/libraries/mops-interface-c
 * modified #ifdef cplusplus to #Ifdef __cplusplus
 * __cplusplus is recognized by gcc and g++ where cplusplus (no
 * underscores) is only recognized by g++
 * 
 * *****************  Version 6  *****************
 * User: Wayne L. Smith Date: 8/12/04    Time: 9:58a
 * Updated in $/Software/libraries/mops-interface-c
 * Fixed extern "C" issue for C code.
 * 
 * *****************  Version 5  *****************
 * User: Wayne L. Smith Date: 8/12/04    Time: 8:57a
 * Updated in $/Software/libraries/mops-interface-c
 * Removed C++ style comments for C code.  Note that this does seem to be
 * supported in the target standard (C99), but it seems safer to avoid
 * them.
 * 
 * *****************  Version 4  *****************
 * User: Wayne L. Smith Date: 8/11/04    Time: 6:41p
 * Updated in $/Software/libraries/mops-interface-c
 * Cleaned up the "see also" behavior.  Doxygen always starts a new
 * paragraph when it encounters the tag (although it doesn't insert the
 * subsection header).
 * 
 * *****************  Version 3  *****************
 * User: Adam N. Gordon Date: 8/11/04    Time: 11:54a
 * Updated in $/Software/libraries/mops-interface-c
 * Missing closing } for extern command.  Missing #endif command for
 * #ifndef.
 * 
 * *****************  Version 2  *****************
 * User: Wayne L. Smith Date: 8/10/04    Time: 7:15p
 * Updated in $/Software/libraries/mops-interface-c
 * Cleaned up documentation using groups.  Refactored all functions to use
 * a prefix specific to MOPS DC libraries.
 * 
 * *****************  Version 1  *****************
 * User: Wayne L. Smith Date: 8/10/04    Time: 5:53p
 * Created in $/Software/libraries/mops-interface-c
 * Version tools for client library.
 * 
 */

#ifndef __VERSIONTOOLS_H_
#define __VERSIONTOOLS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "MOPSStructs.h"
/** @file VersionTools.h
  * @brief Version control information functions.
  *
  * All functions will have the \c modcv prefix.
  *
  * Provides version and build information for the MOPS Data Collection client
  * libraries and the server-side installation.  The modcv_full() function will
  * return a detailed string describing the current release of the libraries.
  * This will include the description (modcv_description()), the version number
  * (modcv_versionNumber()), the build number (modcv_build()), and the build
  * date (modcv_date()).  A shorter representation of the same information is
  * provided by the modcv_id() function.
  *
  * The server-side build number is obtained with the modcv_server() function.
  *
  * @note The return value of all functions that return a ::StringTP should
  * be destroyed with the \c free() function.
  *
  * @version Revision<!--$$Revision: 14 $-->on<!--$$Date: 2004-12-09 09:55:35 -1000 (Thu, 09 Dec 2004) $-->
  * @author Wayne L. Smith,
  *         Last modified by: <!--$$Author: denneau $-->
  **/

/** @addtogroup version 
  * @{
  * @copydoc VersionTools.h
  **/

	
/** Returns a string that identifies the version of the library.  This will
  * generally be the simplest variant of the (generally) numeric version
  * number (e.g., "1.0" or "5.2a").
  *
  * @note The return value should be destroyed with \c free().
  *
  * @return The version identifier.
  **/
StringTP modcv_versionNumber();


/** Returns the build number of the current release of this library.
  *  
  * @return The current build number.
  *
  * @see modcv_server()
  **/
int modcv_build();


/** Returns the build number of the current server-side installation.
  *  
  * @return The server's current build number.
  *
  * @see modcv_build()
  **/
int modcv_server();


/** Returns the string representation of the release date of the current
  * release of the library.
  *
  * @note The return value should be destroyed with \c free().
  *
  * @return The current release date.
  **/
StringTP modcv_date();


/** This will return a string that identifies the specific release of the
  * library.  For example: "Version 1.0, Build 101".
  *
  * @note The return value should be destroyed with \c free().
  *
  * @return The release identifier.
  *
  * @see modcv_versionNumber()
  * @see modcv_build()
  **/
StringTP modcv_release();


/** This will return a detailed string that serves to identify the current 
  * build of the library.  This might be (for example):
  * <pre>
  * Pan-STARRS Moving Object Data Client
  * %Version 1.0, Build 101
  * Build Date: August 5, 2004 6:15pm
  * </pre>
  *
  * @note The return value should be destroyed with \c free().
  *
  * @return The library identification string.
  *
  * @see modcv_id(), modcv_versionNumber(), modcv_release(), modcv_build(),
  * modcv_date()
  **/
StringTP modcv_full();


/** This will return a one-line string that serves to identify the current
  * build of the library.  This will be an abbreviated variant of the
  * information returned from <code>getIdentification()</code>.
  *
  * @note The return value should be destroyed with \c free().
  *
  * @return The library identification string.
  *
  * @see modcv_full()
  **/
StringTP modcv_id();


/** Returns the full library description to be used in verbose identification.
  * For example, "Pan-STARRS Moving Object Data Client".
  *
  * @note The return value should be destroyed with \c free().
  *
  * @return The full library description.
  **/
StringTP modcv_description();

/** @} */

#ifdef __cplusplus
} // end extern "C"
#endif

#endif /* __VERSIONTOOLS_H_ */

