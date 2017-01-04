/* ----------------------------------------------------------------------------
 *                           * * * UNCLASSIFIED * * *
 * ----------------------------------------------------------------------------
 *
 * $Logfile: /Software/libraries/mops-interface-c/DetectionRef.h $
 *
 * ----------------------------------------------------------------------------
 *
 * $Workfile: DetectionRef.h $
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
 * $History: DetectionRef.h $
 * 
 * *****************  Version 27  *****************
 * User: Wayne L. Smith Date: 9/21/04    Time: 10:36a
 * Updated in $/Software/libraries/mops-interface-c
 * Updated documentation.
 * 
 * *****************  Version 26  *****************
 * User: Wayne L. Smith Date: 9/18/04    Time: 6:27p
 * Updated in $/Software/libraries/mops-interface-c
 * Updated documentation.
 * 
 * *****************  Version 25  *****************
 * User: Wayne L. Smith Date: 9/15/04    Time: 2:30p
 * Updated in $/Software/libraries/mops-interface-c
 * Fixed doxygen errros for the "free" functions.
 * 
 * *****************  Version 24  *****************
 * User: Lisa A. Shannon Date: 9/10/04    Time: 4:08p
 * Updated in $/Software/libraries/mops-interface-c
 * modified #ifdef cplusplus to #Ifdef __cplusplus
 * __cplusplus is recognized by gcc and g++ where cplusplus (no
 * underscores) is only recognized by g++
 * 
 * *****************  Version 23  *****************
 * User: Lisa A. Shannon Date: 9/02/04    Time: 9:36a
 * Updated in $/Software/libraries/mops-interface-c
 * added freeArray() freeStruct() and freeStructArray()
 * 
 * *****************  Version 22  *****************
 * User: Wayne L. Smith Date: 8/13/04    Time: 2:07a
 * Updated in $/Software/libraries/mops-interface-c
 * Fully scrubbed Detection documentation.  Partially scrubbed Orbits,
 * Pairs, Observations.
 * 
 * *****************  Version 21  *****************
 * User: Wayne L. Smith Date: 8/12/04    Time: 8:56p
 * Updated in $/Software/libraries/mops-interface-c
 * Created lots of data types to better indicate expectations.  Changed a
 * few function signatures.  Added some descriptive documentation.
 * 
 * *****************  Version 20  *****************
 * User: Lisa A. Shannon Date: 8/12/04    Time: 5:17p
 * Updated in $/Software/libraries/mops-interface-c
 * Added ...._free method to allow the user to free any Mops...TP pointer
 * 
 * *****************  Version 19  *****************
 * User: Lisa A. Shannon Date: 8/12/04    Time: 3:51p
 * Updated in $/Software/libraries/mops-interface-c
 * changed all references to getNext to next 
 * changed all references to getPrev to prev
 * 
 * *****************  Version 18  *****************
 * User: Wayne L. Smith Date: 8/12/04    Time: 3:27p
 * Updated in $/Software/libraries/mops-interface-c
 * Cleaned up documentation groupings.  Refactored a few names.
 * 
 * *****************  Version 17  *****************
 * User: Wayne L. Smith Date: 8/12/04    Time: 9:55a
 * Updated in $/Software/libraries/mops-interface-c
 * Fixed extern "C" issue for C code.
 * 
 * *****************  Version 16  *****************
 * User: Wayne L. Smith Date: 8/11/04    Time: 8:18p
 * Updated in $/Software/libraries/mops-interface-c
 * Partial sanity check on documentation.  Simplified some function names.
 * 
 * *****************  Version 15  *****************
 * User: Lisa A. Shannon Date: 8/11/04    Time: 2:25p
 * Updated in $/Software/libraries/mops-interface-c
 * Removed update method since objects of this kind are immutable
 * 
 * *****************  Version 14  *****************
 * User: Lisa A. Shannon Date: 8/11/04    Time: 2:15p
 * Updated in $/Software/libraries/mops-interface-c
 * More Documentation Changes
 * 
 * *****************  Version 13  *****************
 * User: Lisa A. Shannon Date: 8/11/04    Time: 1:15p
 * Updated in $/Software/libraries/mops-interface-c
 * Changed <p>Notes:  to \note
 * 
 * *****************  Version 12  *****************
 * User: Lisa A. Shannon Date: 8/11/04    Time: 1:00p
 * Updated in $/Software/libraries/mops-interface-c
 * Added a vew </LI> to correctly end list line items.
 * 
 * *****************  Version 11  *****************
 * User: Lisa A. Shannon Date: 8/11/04    Time: 12:56p
 * Updated in $/Software/libraries/mops-interface-c
 * Corrected some of the \sa (s) so they would be references
 * 
 * *****************  Version 10  *****************
 * User: Lisa A. Shannon Date: 8/11/04    Time: 12:11p
 * Updated in $/Software/libraries/mops-interface-c
 * Made documentation changes:
 * 1.  Implemented ModcDetTP, ModcOrbitTP, ModcObsTP, FIlterTP
 * 2.  Enchanced documentation
 * 
 * *****************  Version 9  *****************
 * User: Wayne L. Smith Date: 8/11/04    Time: 8:44a
 * Updated in $/Software/libraries/mops-interface-c
 * Moved lower-level layer header files into the implementation (*.cpp)
 * rather than this layer's header files.
 * 
 * *****************  Version 8  *****************
 * User: Wayne L. Smith Date: 8/10/04    Time: 7:15p
 * Updated in $/Software/libraries/mops-interface-c
 * Cleaned up documentation using groups.  Refactored all functions to use
 * a prefix specific to MOPS DC libraries.
 * 
 * *****************  Version 7  *****************
 * User: Lisa A. Shannon Date: 8/10/04    Time: 3:08p
 * Updated in $/Software/libraries/mops-interface-c
 * Finished documenting functionality for C layer API
 * 
 * *****************  Version 6  *****************
 * User: Lisa A. Shannon Date: 8/09/04    Time: 6:14p
 * Updated in $/Software/libraries/mops-interface-c
 * removed temp method getDetObs()
 * 
 * *****************  Version 5  *****************
 * User: Lisa A. Shannon Date: 8/02/04    Time: 4:46p
 * Updated in $/Software/libraries/mops-interface-c
 * added the ability to get a single observation from a detection
 * 
 * *****************  Version 4  *****************
 * User: Lisa A. Shannon Date: 7/29/04    Time: 12:14p
 * Updated in $/Software/libraries/mops-interface-c
 * Modifications so Adam can work
 * 
 * *****************  Version 3  *****************
 * User: Lisa A. Shannon Date: 7/29/04    Time: 12:13p
 * Updated in $/Software/libraries/mops-interface-c
 * Modifications so Adam can work
 * 
 * *****************  Version 2  *****************
 * User: Lisa A. Shannon Date: 7/22/04    Time: 3:56p
 * Updated in $/Software/libraries/mops-interface-c
 * Pipeline changes
 * 
 * *****************  Version 1  *****************
 * User: Lisa A. Shannon Date: 7/19/04    Time: 9:32a
 * Created in $/Software/libraries/mops-interface-c
 * Initial Check in of mops-interface-c layer files
 * 
 */

/** @file
  * 
  * C-language functions to manipulate detections.
  *
  * All functions will have the \c modcd prefix.
  *
  * @see DetectionTP
  *
  * @version Revision<!--$$Revision: 14 $-->on<!--$$Date: 2004-12-09 09:55:35 -1000 (Thu, 09 Dec 2004) $-->
  * @author Lisa A. Shannon,
  *         Last modified by: <!--$$Author: denneau $-->
  **/

/** @addtogroup modcd
  * @{
  * @copydoc DetectionRef.h
  **/

#ifndef __DETECTIONREF_H
#define __DETECTIONREF_H

#include "MOPSStructs.h"

#ifdef __cplusplus
extern "C" {
#endif


/** @defgroup modcd_general General Utilities
  * Functions to manipulate and convert detection objects.
  *
  * New detections can be created with the modcd_create() function, 
  * which will create a new object in memory (i.e., not stored in the
  * database).  The resulting MOPS-DC object pointer (a ::ModcDetTP) 
  * can be used to manipulate the object, including storing it in the 
  * database (see @ref modcd_dbin) and accessing data fields (see 
  * @ref modcd_access).
  * 
  * The MOPS-DC object pointer may reference a single object or an array of 
  * objects when it is the result of a database retrieval (see 
  * @ref modcd_db).  The number of objects can be obtained with the
  * modcd_getCount() function, and the various objects may be iterated
  * using the modcd_next() and modcd_prev() functions.
  *
  * All MOPS-DC object pointers can be converted to C-language data 
  * structures.  The modcd_toStruct() function is used to create a single
  * data structure, and the modcd_toStructArray() function will create
  * an array of data structures.
  *
  * Finally, the storage for a MOPS-DC object pointer can be released 
  * by use of the modcd_free() function.  This function should be called
  * when the object pointer is no longer needed.  Additional data structures
  * can be freed with the modcd_freeArray(), modcd_freeStruct(), and 
  * modcd_freeArrayStruct() functions.
  *
  * @note These functions do not perform any database activities.  However,
  *       an active database connection may be required for their use.  See
  *       modc_initialize(StringTP).
  *
  * @ingroup modcd
  **/

/** @defgroup modcd_access Accessor Functions
  * Functions to access values from the fields of a detection object.
  * These functions operate on an existing detection object, and do not 
  * perform a database retrieval.
  * @sample_code DetTests.c modcd_access
  * @ingroup modcd
  **/

/** @defgroup modcd_db Retrieval Functions
  * Functions to retrieve detections from the persistant database.
  * 
  * All retrievals will return a MOPS-DC pointer to the resulting
  * objects.  For most retrievals, this pointer will reference an array
  * of objects.  The modcd_getCount() function can be used to determine
  * how many objects were retrieved.  If no objects are retrieved, the
  * MOPS-DC pointer will be \c NULL.  See \ref modcd_general and 
  * \ref modcd_access for more details on using the MOPS-DC pointer.
  *
  * @ingroup modcd
  **/

/** @defgroup modcd_dbin Storage Functions
  * Functions to store and update detections in the persistant database.
  * @ingroup modcd
  *
  * Note that you cannot insert a detection directly.  Each detection must
  * be associated with an existing observation through the 
  * ::modcm_addDetections() function.
  **/

	
/** Converts the specified object pointer to a detection data structure.
  * This function assumes that the ::ModcDetTP pointer was previously 
  * retrieved from the database. If \a ptr was not retrieved from the 
  * database (i.e., it was created using modcd_create()), this method will 
  * return \c NULL.
  *
  * @param ptr The MOPS-DC pointer to a detection object.
  *
  * @return The equivalent detection data structure (or \c NULL if \a ptr
  *         was not retrieved from the database).
  *
  * @note The caller of this function is responsible for freeing the space 
  *       allocated for the ::DetectionTP.  This may be done using \c free().
  * @note This function does not perform a retrieval from the database.
  *
  * @sample_code DetTests.c modcd_convertToDetection
  * @ingroup modcd_general
  **/
DetectionTP modcd_toStruct(ModcDetTP ptr);

/** Converts all of the detection objects associated with the specified
  * object pointer into an array of detection data structures.
  * This function assumes that the ::ModcDetTP pointer was previously 
  * retrieved from the database. If \a ptr was not retrieved from the 
  * database (i.e., it was created using modcd_create()), this method will 
  * return \c NULL.
  *
  * The \a ptr will generally refer to a set of detection objects.  
  * The returned array will be of size modcd_getCount(ModcDetTP). 
  *
  * @param ptr The MOPS-DC pointer to a set of detection objects.
  *
  * @return The equivalent array of detection data structures (or \c NULL 
  *         if \a ptr was not retrieved from the database).
  *
  * @note The caller of this function is responsible for freeing the space 
  *       allocated for the ::DetectionTP.  This may be done using \c free().
  * @note This function does not perform a retrieval from the database.
  *
  * @see modcd_getCount(ModcDetTP)
  *
  * @sample_code DetTests.c modcd_convertToDetectionArray
  * @ingroup modcd_general
  **/
DetectionArrayTP modcd_toStructArray(ModcDetTP ptr);

/** Returns the number of detection objects associated with the specified
  * object pointer.  This function assumes that the ::ModcDetTP pointer was 
  * previously retrieved from the database. If \a ptr was not retrieved from 
  * the database (i.e., it was created using modcd_create()), this function 
  * will return <code>(int)</code>::Fail.
  *
  * @param ptr  The MOPS-DC pointer that originated from a detection 
  *             retrieval.
  *
  * @return The number of detections associated with the ::ModcDetTP.
  *
  * @note This function does not perform a retrieval from the database.
  * @note If \a ptr was not retrieved from the database, this function will 
  *       return <code>(int)</code>::Fail.
  *
  * @see modcd_toStructArray()
  *
  * @sample_code DetTests.c modcd_convertToDetectionArray
  * @ingroup modcd_general
  **/
int	modcd_getCount(ModcDetTP ptr);


/** Create a new detection object.  This function will not store the object
  * in the database.   Detections may only be inserted into the database 
  * along with their corresponding observation.  To perform an insertion
  * use the modcm_addDetections(ModcObsTP, ModcDetArrayTP) function.
  *
  * Passing the ::DOUBLE_NIL value as any of the \c double parameters 
  * indicates that the parameter should remain uninitialized.  This is 
  * permitted for the following parameters:
  * @todo List the allowable ::DOUBLE_NIL parameters to create.
  * Passing \c NULL to the \a filter parameter is not legal (and will result 
  * in a \c NULL return value).
  *
  * @param ra 		 The Right Ascension (degrees).
  * @param dec 		 The Declination (degrees).
  * @param epoch 	 The observation time (MJD).
  * @param mag 	 	 The magnitude (magnitude).
  * @param filter    The filter identifier.
  * @param isFake    ::True if this is a simulated detection, ::False otherwise.
  * @param flags     The detection flags.
  * @param raSigma   The error in Right Ascension (arcsec).
  * @param decSigma  The error in Declination (arcsec).
  * @param magSigma  The error in magnitude (magnitude).
  * @param length  	 The length of the detection (arcsec).
  * @param angle  	 The angle of the detection (degrees, clockwise from North).
  *
  * @note This function does not perform any database operations.
  * @note If any \c double value is ::DOUBLE_NIL then the parameter will 
  *       remain uninitialized.  If a required parameter is uninitialized,
  *       this function will return \c NULL.
  *         
  * @see modcm_addDetections(ModcObsTP, ModcDetArrayTP)
  *
  * @sample_code DetTests.c modcd_create
  * @ingroup modcd_general
  **/
ModcDetTP modcd_create( double ra,
						double dec,
						MjdTP epoch,
						double mag,
						FilterTP filter,
						BoolTP isFake,
						FlagTP flags,
						double raSigma,
						double decSigma,
						double magSigma,
						float length,
						float angle);

/** Retrieves all unattributed detections associated with a specific
  * observation.  An unattributed detection is one that has not been associated
  * with any known orbit.  This will return \c NULL if no matching detections
  * can be found.
  *
  * @param obsId  The ID of the observation.
  *
  * @return The MOPS-DC pointer to the retrieved object set, or \c NULL.
  *
  * @note This function performs a retrieval from the database.
  *
  * @see modcd_toStruct(), modcd_toStructArray(), modcd_getCount()
  * 
  * @sample_code DetTests.c modcd_retrieveUnattributedByObsId
  * @ingroup modcd_db
  **/
ModcDetTP modcd_retrieveUnattributedByObsId(IdTP obsId);

/** Retrieves all unattributed detections based on specific values.  
  * An unattributed detection is one that has not been associated
  * with any known orbit.  This will return \c NULL if no matching detections
  * can be found.
  *
  * @param epoch      The epoch of the detection (MJD).
  * @param ra         The Right Ascension (degrees).
  * @param dec        The Declination (degrees).
  * @param deltaEpoch The value that will be +/- from \a epoch to create a 
  *                   lower and upper bound for the epoch of the detection. 
  *                   If \a epoch is ::DOUBLE_NIL then the \a deltaEpoch 
  *                   parameter will be ignored.
  * @param deltaRa    The value that will be +/- from \a ra to create a lower 
  *                   and upper bound for the Right Ascension.  If \a ra is 
  *                   ::DOUBLE_NIL then the \a deltaRA parameter will be 
  *                   ignored.
  * @param deltaDec   The value that will be +/- from \a dec to create a lower
  *                   and upper bound for the Declination.  If \a dec is 
  *                   ::DOUBLE_NIL then the \a deltaDec parameter will be 
  *                   ignored.
  * @param isFake     ::True if the detection is simulated, ::False otherwise.
  *
  * @return The MOPS-DC pointer to the retrieved object set, or \c NULL.
  *
  * @todo Add a function that doesn't care about \a isFake value in DB.  This 
  * can be handled for the \a ra, \a dec, and \a epoch by passing ::DOUBLE_NIL
  * but that won't work for a ::BoolTP.  Maybe a \c retrieveByPattern() 
  * function.
  *
  * @note This function performs a retrieval from the database.
  *
  * @see modcd_toStruct(), modcd_toStructArray(), modcd_getCount()
  * 
  * @sample_code DetTests.c modcd_retrieveUnattributedByValue
  * @ingroup modcd_db
  **/
ModcDetTP modcd_retrieveUnattributedByValue(MjdTP epoch, 
										    double ra, 
										    double dec,
										    double deltaEpoch, 
										    double deltaRa, 
										    double deltaDec, 
										    BoolTP isFake);

/** Retrieves a single detection based on a unique identifier.  This will
  * return \c NULL if the ID cannot be found.
  *
  * @param detId  The ID of the detection.
  *
  * @return The MOPS-DC pointer to the retrieved object set, or \c NULL.
  *
  * @note This function performs a retrieval from the database.
  *
  * @see modcd_toStruct(), modcd_toStructArray(), modcd_getCount()
  * 
  * @sample_code DetTests.c modcd_retrieve
  * @ingroup modcd_db
  **/
ModcDetTP modcd_retrieve(IdTP detId);

/** Retrieves all detections based on specific values. This will return \c NULL
  * if no matching detections can be found.
  *
  * @param epoch      The epoch of the detection (MJD).
  * @param ra         The Right Ascension (degrees).
  * @param dec        The Declination (degrees).
  * @param deltaEpoch The value that will be +/- from \a epoch to create a 
  *                   lower and upper bound for the epoch of the detection. 
  *                   If \a epoch is ::DOUBLE_NIL then the \a deltaEpoch 
  *                   parameter will be ignored.
  * @param deltaRa    The value that will be +/- from \a ra to create a lower 
  *                   and upper bound for the Right Ascension.  If \a ra is 
  *                   ::DOUBLE_NIL then the \a deltaRA parameter will be 
  *                   ignored.
  * @param deltaDec   The value that will be +/- from \a dec to create a lower
  *                   and upper bound for the Declination.  If \a dec is 
  *                   ::DOUBLE_NIL then the \a deltaDec parameter will be 
  *                   ignored.
  * @param isFake     ::True if the detection is simulated, ::False otherwise.
  *
  * @return The MOPS-DC pointer to the retrieved object set, or \c NULL.
  *
  * @todo Add a retrieval-by-value function that doesn't care about 
  * \a isFake value in DB.  This  can be handled for the \a ra, \a dec, 
  * and \a epoch by passing ::DOUBLE_NIL but that won't work for a ::BoolTP
  * \e Alternative: Something like \c modcd_retrieveByPattern(DetPatternTP)
  * where \c DetPatternTP is a structure containing ranges for the various
  * fields and a flag indicating whether they should be used.
  *
  * @note This function performs a retrieval from the database.
  *
  * @see modcd_toStruct(), modcd_toStructArray(), modcd_getCount()
  * 
  * @sample_code DetTests.c modcd_retrieveByValue
  * @ingroup modcd_db
  **/
ModcDetTP modcd_retrieveByValue(MjdTP epoch, 
							    double ra, 
							    double dec,
							    double deltaEpoch, 
							    double deltaRa, 
							    double deltaDec, 
							    BoolTP isFake);


/** Retrieves all detections associated with a specific observation.  This will
  * return \c NULL if no matching detections can be found.
  *
  * @param obsId  The ID of the observation.
  *
  * @return The MOPS-DC pointer to the retrieved object set, or \c NULL.
  *
  * @note This function performs a retrieval from the database.
  *
  * @see modcd_toStruct(), modcd_toStructArray(), modcd_getCount()
  * 
  * @sample_code DetTests.c modcd_retrieveByObsId
  * @ingroup modcd_db
  **/
ModcDetTP modcd_retrieveByObsId(IdTP obsId);
								
/** Retrieves all detections associated with a specific observation.  This will
  * return \c NULL if no matching detections can be found.
  *
  * @param obs  The MOPS-DC observation pointer.
  *
  * @return The MOPS-DC pointer to the retrieved object set, or \c NULL.
  *
  * @note This function performs a retrieval from the database.
  *
  * @see modcd_toStruct(), modcd_toStructArray(), modcd_getCount()
  * 
  * @sample_code DetTests.c modcd_retrieveByObs
  * @ingroup modcd_db
  **/
ModcDetTP modcd_retrieveByObs(ModcObsTP obs);

/** Retrieves all detections attributed to a specific orbit.  This will
  * return \c NULL if no matching detections can be found.
  *
  * @param orbitId  The unique ID of the orbit.
  *
  * @return The MOPS-DC pointer to the retrieved object set, or \c NULL.
  *
  * @note This function performs a retrieval from the database.
  *
  * @see modcd_toStruct(), modcd_toStructArray(), modcd_getCount()
  * 
  * @sample_code DetTests.c modcd_retrieveByOrbitId
  * @ingroup modcd_db
  **/
ModcDetTP modcd_retrieveByOrbitId(IdTP orbitId);
								
/** Retrieves all detections attributed to a specific orbit.  This will
  * return \c NULL if no matching detections can be found.
  *
  * @param orbit  The MOPS-DC orbit pointer.
  *
  * @return The MOPS-DC pointer to the retrieved object set, or \c NULL.
  *
  * @note This function performs a retrieval from the database.
  *
  * @see modcd_toStruct(), modcd_toStructArray(), modcd_getCount()
  * 
  * @sample_code DetTests.c modcd_retrieveByOrbit
  * @ingroup modcd_db
  **/
ModcDetTP modcd_retrieveByOrbit(ModcOrbitTP orbit);


/** Attribute a single detection to an orbit.  Use the orbit-based function 
  * ::modco_attributeDetections(ModcOrbitTP, ModcDetArrayTP) to attribute 
  * multiple detections at once.  The orbit specified by \a orbitId must 
  * already exist in the database, and the \a det object must be the result 
  * of a previous database retrieval.
  *
  * @param det      The MOPS-DC detection object to attribute.
  * @param orbitId  The unique ID of the orbit.
  *
  * @return ::Success if the detection was successfully attributed to the
  *         orbit, ::Fail otherwise.
  *
  * @note This function performs an insert into the database.
  * 
  * @see modcd_clearOrbit(), modcd_clearOrbitById(), 
  *      modco_attributeDetections(), modco_clearDetections()
  *
  * @sample_code DetTests.c modcd_attributeToOrbitById
  * @ingroup modcd_dbin
  **/
SucceedTP modcd_attributeToOrbitById(ModcDetTP det, IdTP orbitId);	

/** Attribute a single detection to an orbit.  Use the orbit-based function 
  * ::modco_attributeDetections(ModcOrbitTP, ModcDetArrayTP) to attribute 
  * multiple detections at once.  The \a orbit object and the \a det object
  * must both already exist in the database (i.e., the pointers are the result
  * of previous retrievals).
  *
  * @param det      The MOPS-DC detection object to attribute.
  * @param orbit    The MOPS-DC orbit object.
  *
  * @return ::Success if the detection was successfully attributed to the
  *         orbit, ::Fail otherwise.
  *
  * @note This function performs an insert into the database.
  * 
  * @see modcd_clearOrbit(), modcd_clearOrbitById(), 
  *      modco_attributeDetections(), modco_clearDetections()
  *
  * @sample_code DetTests.c modcd_attributeToOrbit
  * @ingroup modcd_dbin
  **/
SucceedTP modcd_attributeToOrbit(ModcDetTP det,
							     ModcOrbitTP orbit);


/** Remove an attributed detection from an orbit.  Use the orbit-based function 
  * ::modco_clearDetections(ModcOrbitTP, ModcDetArrayTP) to clear the 
  * attribution of multiple detections at once.  The orbit and detection must
  * both exist in the database, and the detection must already be attributed
  * to the orbit.
  *
  * @param det      The MOPS-DC detection object to remove from the orbit.
  * @param orbitId  The unique ID of the orbit.
  *
  * @return ::Success if the detection was successfully cleared from the
  *         orbit, ::Fail otherwise.
  *
  * @note This function performs a delete from the database.
  * 
  * @see modcd_clearOrbit(), modco_clearDetections()
  *
  * @sample_code DetTests.c modcd_clearOrbitById
  * @ingroup modcd_dbin
  **/
SucceedTP modcd_clearOrbitById(ModcDetTP det,
                               IdTP orbitId);

/** Remove an attributed detection from an orbit.  Use the orbit-based function 
  * ::modco_clearDetections(ModcOrbitTP, ModcDetArrayTP) to clear the 
  * attribution of multiple detections at once.  The orbit and detection must
  * both exist in the database, and the detection must already be attributed
  * to the orbit.
  *
  * @param det    The MOPS-DC detection object to remove from the orbit.
  * @param orbit  The MOPS-DC orbit object.
  *
  * @return ::Success if the detection was successfully cleared from the
  *         orbit, ::Fail otherwise.
  *
  * @note This function performs a delete from the database.
  * 
  * @see modcd_clearOrbitById(), modco_clearDetections()
  *
  * @sample_code DetTests.c modcd_clearOrbit
  * @ingroup modcd_dbin
  **/
SucceedTP modcd_clearOrbit(ModcDetTP det,
						   ModcOrbitTP orbit);

/* Note Detection Insertion is done in Observation, since a
   Detection must always be associated with an Observation. */

/** Delete a detection from the database.  The detection must already exist
  * in the database.
  *
  * @param detId  The unique ID of the detection to delete.
  *
  * @return ::Success if the detection was successfully deleted, ::Fail 
  *         otherwise.
  *
  * @note This function performs a delete from the database.
  *
  * @sample_code DetTests.c modcd_delete
  * @ingroup modcd_dbin
  **/
SucceedTP modcd_delete(IdTP detId);

/** Free all memory associated with the MOPS-DC object pointer.  This will
  * clear all memory associated with all ::ModcDetTP information obtained from
  * a retrieval or create operation.  All detection objects associated with a
  * retrieval will be freed by a single call to this function.
  *
  * @param det  The MOPS-DC object pointer to free.
  *
  * @return ::Success if the detection was successfully deleted, ::Fail 
  *         otherwise.
  *
  * @note This function does not perform any database operations.
  *
  * @sample_code DetTests.c modcd_free
  * @ingroup modcd_general
  **/
SucceedTP modcd_free(ModcDetTP det);

/** Free all memory associated with the MOPS-DC pointer array.  This will
  * clear memory associated with ::ModcDetArrayTP only.
  *
  * @param dets  The MOPS-DC object array pointer to free.
  *
  * @return ::Success if the array was successfully deleted, ::Fail 
  *         otherwise.
  *
  * @note This function does not perform any database operations.
  *
  * @see modco_getDetections(ModcOrbitTP), 
  *      modcp_retrieveDetections(ModcPairTP), 
  *      modcp_getDets(ModcPairTP), and
  *      modcp_retrieveDetectionsByPairId(IdTP).
  *
  * @ingroup modcd_general
  **/
SucceedTP modcd_freeArray(ModcDetArrayTP dets);

/** Free all memory associated with the detection structure pointer.  This will
  * clear memory associated with the ::DetectionTP only.  This should be called on
  * the result of the ::modcd_toStruct() function.
  *
  * @param det  The structure pointer to free.
  *
  * @return ::Success if the detection structure was successfully deleted, 
  *         ::Fail otherwise.
  *
  * @note This function does not perform any database operations.
  *
  * @ingroup modcd_general
  **/
SucceedTP modcd_freeStruct( DetectionTP det);

/** Free all memory associated with the array of detection structures.  This will
  * clear memory associated with the ::DetectionArrayTP only.  This should be called on
  * the result of the ::modcd_toStructArray() function.
  *
  * @param dets  The structure array to free.
  *
  * @return ::Success if the structure array was successfully deleted, ::Fail 
  *         otherwise.
  *
  * @note This function does not perform any database operations.
  *
  * @ingroup modcd_general
  **/
SucceedTP modcd_freeArrayStruct( DetectionArrayTP dets);

/* ////////////////////////////////////Getters////////////////////////////////////// */


/** Return the unique ID for the detection object.  This will be \c NULL if the
  * detection was not retrieved from the database.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The unique ID of the detection object.
  *
  * @note This method does not perform a retrieval from the database.
  *
  * @ingroup modcd_access
  **/
IdTP modcd_getId(ModcDetTP ptr);

/** Return the Right Ascension of the detection object.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The Right Ascension of the detection object (degrees).
  *
  * @note This method does not perform a retrieval from the database.
  *
  * @ingroup modcd_access
  **/
double modcd_getRa(ModcDetTP ptr);

/** Return the Declination of the detection object.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The Declination of the detection object (degrees).
  *
  * @note This method does not perform a retrieval from the database.
  *
  * @ingroup modcd_access
  **/
double modcd_getDec(ModcDetTP ptr);

/** Return the epoch of the detection object.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The epoch of the detection object (MJD).
  *
  * @note This method does not perform a retrieval from the database.
  *
  * @ingroup modcd_access
  **/
MjdTP modcd_getEpoch(ModcDetTP ptr);	

/** Return the magnitude of the detection object.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The magnitude of the detection object.
  *
  * @note This method does not perform a retrieval from the database.
  *
  * @ingroup modcd_access
  **/
double modcd_getMagnitude( ModcDetTP ptr);	

/** Return the filter for the detection object.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The filter ID of the detection object.
  *
  * @note This method does not perform a retrieval from the database.
  *
  * @ingroup modcd_access
  **/
FilterTP modcd_getFilter(ModcDetTP ptr);
	
/** Return a flag indicating that the detection object is simulated.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return ::True if the detection is simulated, ::False otherwise.
  *
  * @note This method does not perform a retrieval from the database.
  *
  * @ingroup modcd_access
  **/
BoolTP modcd_getIsFake(ModcDetTP ptr);

/** Return the bit flags for the detection object.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The flags of the detection object.
  *
  * @note This method does not perform a retrieval from the database.
  *
  * @ingroup modcd_access
  **/
FlagTP modcd_getFlags(ModcDetTP ptr);	

/** Return the error in Right Ascension for the detection object.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The error in R.A. of the detection object (arcsec).
  *
  * @note This method does not perform a retrieval from the database.
  *
  * @ingroup modcd_access
  **/
double modcd_getRaSigma(ModcDetTP ptr);

/** Return the error in Declination for the detection object.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The error in Declination of the detection object (arcsec).
  *
  * @note This method does not perform a retrieval from the database.
  *
  * @ingroup modcd_access
  **/
double modcd_getDecSigma(ModcDetTP ptr);	

/** Return the error in magnitude for the detection object.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The error in magnitude of the detection object.
  *
  * @note This method does not perform a retrieval from the database.
  *
  * @ingroup modcd_access
  **/
double modcd_getMagSigma(ModcDetTP ptr);

/** Return the detection length for the detection object.  This will be 
  * zero if the detection was a point source.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The length of the detection object (arcsec).
  *
  * @note This method does not perform a retrieval from the database.
  *
  * @see modcd_getAngle()
  *
  * @todo How to represent the database NULL in this case?  Maybe have a
  *       flag for the detection (not stored) indicating that the length is 
  *       meaningful.
  *
  * @ingroup modcd_access
  **/
float modcd_getLength(ModcDetTP ptr);

/** Return the orientation angle for the detection object.  This should be
  * ignore if modcd_getLength() is zero (i.e., a point source).
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The orientation angle of the detection object (degrees, 
  *         counter-clockwise from North).
  *
  * @note This method does not perform a retrieval from the database.
  *
  * @see modcd_getLength()
  *
  * @ingroup modcd_access
  **/
float modcd_getAngle(ModcDetTP ptr);	

/** Return the observation (metadata) for this detection object.  This will be 
  * \c NULL if the detection was not retrieved from the database.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The MOPS-DC object pointer for the observation.
  *
  * @note This method does not perform a retrieval from the database.
  *
  * @ingroup modcd_access
  **/
ModcObsTP modcd_getObservation(ModcDetTP ptr);		

/** Returns the number of orbits to which the specified detection has been
  * attributed.  This function assumes that the ::ModcDetTP pointer was 
  * previously retrieved from the database. If \a ptr was not retrieved from 
  * the database (i.e., it was created using modcd_create()), this function 
  * will return <code>(int)</code>::Fail.
  *
  * @param ptr  The MOPS-DC pointer that originated from a detection 
  *             retrieval.
  *
  * @return The number of orbits associated with the ::ModcDetTP.
  *
  * @note This function does not perform a retrieval from the database.
  * @note If \a ptr was not retrieved from the database, this function will 
  *       return <code>(int)</code>::Fail.
  *
  * @see modcd_getOrbits(ModcDetTP)
  *
  * @ingroup modcd_access
  **/
int	modcd_getOrbitCount(ModcDetTP ptr);


/** Return all orbits to which the specified detection has been attributed.
  * This will return an array of size ::modcd_getOrbitCount(), or \c NULL
  * if \a ptr was not retrieved from the database.
  *
  * @param ptr the MOPS-DC object to access.
  *
  * @return All orbits to which this detection is attributed.
  *
  * @note This function does not perform a retrieval from the database.
  *
  * @see modcd_getOrbitCount()
  *
  * @ingroup modcd_access
  **/
ModcOrbitArrayTP modcd_getOrbits(ModcDetTP ptr);		


/** Return the next detection object in the pool of available detections
  * retrieved from the database.  If \a ptr is referencing the last 
  * detection in the pool, this function will return \c NULL.  If \a ptr
  * was not created from a database retrieval, this function will always
  * return \c NULL.
  *
  * @param ptr  The MOPS-DC object pointer retrieved from the database.
  *
  * @return The next detection in the retrieved set (or \c NULL).
  *
  * @note This function does not perform a retrieval from the database.
  *
  * @sample_code DetTests.c modcd_next
  * @ingroup modcd_general
  **/
ModcDetTP modcd_next(ModcDetTP ptr);

/** Return the previous detection object in the pool of available detections
  * retrieved from the database.  If \a ptr is referencing the first 
  * detection in the pool, this function will return \c NULL.  If \a ptr
  * was not created from a database retrieval, this function will always
  * return \c NULL.
  *
  * @param ptr  The MOPS-DC object pointer retrieved from the database.
  *
  * @return The previous detection in the retrieved set (or \c NULL).
  *
  * @note This function does not perform a retrieval from the database.
  *
  * @ingroup modcd_general
  **/
ModcDetTP modcd_prev(ModcDetTP ptr);

/** Return the next orbit to which the specified detection has been attributed.
  * If \a ptr is referencing the last orbit in the pool, this function will 
  * return \c NULL.  If \a ptr was not created from a database retrieval, 
  * this function will always return \c NULL.
  *
  * @param ptr  The MOPS-DC detection pointer retrieved from the database.
  *
  * @return The next orbit to which the detection is attributed (or \c NULL).
  *
  * @note This function does not perform a retrieval from the database.
  *
  * @see modcd_getOrbits()
  *
  * @sample_code DetTests.c modcd_nextOrbit
  * @ingroup modcd_access
  **/
ModcOrbitTP modcd_nextOrbit(ModcDetTP ptr);

/** Return the previous orbit to which the specified detection has been 
  * attributed.  If \a ptr is referencing the first orbit in the pool, this 
  * function will return \c NULL.  If \a ptr was not created from a database 
  * retrieval, this function will always return \c NULL.
  *
  * @param ptr  The MOPS-DC detection pointer retrieved from the database.
  *
  * @return The previous orbit to which the detection is attributed (or \c NULL).
  *
  * @note This function does not perform a retrieval from the database.
  *
  * @see modcd_getOrbits()
  *
  * @ingroup modcd_access
  **/
ModcOrbitTP modcd_prevOrbit(ModcDetTP ptr);

/** @} */

#ifdef __cplusplus
} // end extern "C"
#endif 


#endif 

