/* ----------------------------------------------------------------------------
 *                           * * * UNCLASSIFIED * * *
 * ----------------------------------------------------------------------------
 *
 * $Logfile: /Software/libraries/mops-interface-c/PairRef.h $
 *
 * ----------------------------------------------------------------------------
 *
 * $Workfile: PairRef.h $
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
 * $History: PairRef.h $
 * 
 * *****************  Version 25  *****************
 * User: Wayne L. Smith Date: 9/21/04    Time: 10:36a
 * Updated in $/Software/libraries/mops-interface-c
 * Updated documentation.
 * 
 * *****************  Version 24  *****************
 * User: Wayne L. Smith Date: 9/20/04    Time: 9:45a
 * Updated in $/Software/libraries/mops-interface-c
 * Fixed documentation typo.
 * 
 * *****************  Version 23  *****************
 * User: Wayne L. Smith Date: 9/18/04    Time: 6:27p
 * Updated in $/Software/libraries/mops-interface-c
 * Updated documentation.
 * 
 * *****************  Version 22  *****************
 * User: Lisa A. Shannon Date: 9/16/04    Time: 12:07p
 * Updated in $/Software/libraries/mops-interface-c
 * added some tabs
 * 
 * *****************  Version 21  *****************
 * User: Lisa A. Shannon Date: 9/15/04    Time: 10:24a
 * Updated in $/Software/libraries/mops-interface-c
 * 
 * *****************  Version 20  *****************
 * User: Lisa A. Shannon Date: 9/10/04    Time: 4:08p
 * Updated in $/Software/libraries/mops-interface-c
 * modified #ifdef cplusplus to #Ifdef __cplusplus
 * __cplusplus is recognized by gcc and g++ where cplusplus (no
 * underscores) is only recognized by g++
 * 
 * *****************  Version 19  *****************
 * User: Lisa A. Shannon Date: 9/03/04    Time: 12:22p
 * Updated in $/Software/libraries/mops-interface-c
 * 
 * *****************  Version 18  *****************
 * User: Lisa A. Shannon Date: 9/02/04    Time: 9:46a
 * Updated in $/Software/libraries/mops-interface-c
 * added freeArray(), freeStruct() and freeArrayStruct()
 * 
 * *****************  Version 17  *****************
 * User: Wayne L. Smith Date: 8/13/04    Time: 2:07a
 * Updated in $/Software/libraries/mops-interface-c
 * Fully scrubbed Detection documentation.  Partially scrubbed Orbits,
 * Pairs, Observations.
 * 
 * *****************  Version 16  *****************
 * User: Wayne L. Smith Date: 8/12/04    Time: 8:56p
 * Updated in $/Software/libraries/mops-interface-c
 * Created lots of data types to better indicate expectations.  Changed a
 * few function signatures.  Added some descriptive documentation.
 * 
 * *****************  Version 15  *****************
 * User: Lisa A. Shannon Date: 8/12/04    Time: 5:17p
 * Updated in $/Software/libraries/mops-interface-c
 * Added ...._free method to allow the user to free any Mops...TP pointer
 * 
 * *****************  Version 14  *****************
 * User: Lisa A. Shannon Date: 8/12/04    Time: 3:51p
 * Updated in $/Software/libraries/mops-interface-c
 * changed all references to getNext to next 
 * changed all references to getPrev to prev
 * 
 * *****************  Version 13  *****************
 * User: Lisa A. Shannon Date: 8/12/04    Time: 3:36p
 * Updated in $/Software/libraries/mops-interface-c
 * changed retrieveById methods to be retrieve (i.e removed the ById)
 * 
 * *****************  Version 12  *****************
 * User: Lisa A. Shannon Date: 8/12/04    Time: 3:03p
 * Updated in $/Software/libraries/mops-interface-c
 * changed all methods that retrieve from the database from get... to
 * retrieve...
 * 
 * *****************  Version 11  *****************
 * User: Lisa A. Shannon Date: 8/12/04    Time: 11:47a
 * Updated in $/Software/libraries/mops-interface-c
 * modcp_getId now returns an IdTP
 * 
 * *****************  Version 10  *****************
 * User: Lisa A. Shannon Date: 8/12/04    Time: 10:42a
 * Updated in $/Software/libraries/mops-interface-c
 * 
 * *****************  Version 9  *****************
 * User: Wayne L. Smith Date: 8/12/04    Time: 10:09a
 * Updated in $/Software/libraries/mops-interface-c
 * Fixed extern "C" issue for C code.
 * 
 * *****************  Version 8  *****************
 * User: Lisa A. Shannon Date: 8/12/04    Time: 10:06a
 * Updated in $/Software/libraries/mops-interface-c
 * Removed getSpatialCoord() since spatial coordinates are an
 * implementation detail.
 * Added getById() to retrieve a Pair by a Pair id
 * 
 * *****************  Version 7  *****************
 * User: Lisa A. Shannon Date: 8/11/04    Time: 8:41p
 * Updated in $/Software/libraries/mops-interface-c
 * Documentation changes
 * 
 * *****************  Version 6  *****************
 * User: Lisa A. Shannon Date: 8/11/04    Time: 2:18p
 * Updated in $/Software/libraries/mops-interface-c
 * Implemented Documentation Changes
 * 
 * *****************  Version 5  *****************
 * User: Wayne L. Smith Date: 8/11/04    Time: 8:44a
 * Updated in $/Software/libraries/mops-interface-c
 * Moved lower-level layer header files into the implementation (*.cpp)
 * rather than this layer's header files.
 * 
 * *****************  Version 4  *****************
 * User: Wayne L. Smith Date: 8/10/04    Time: 7:15p
 * Updated in $/Software/libraries/mops-interface-c
 * Cleaned up documentation using groups.  Refactored all functions to use
 * a prefix specific to MOPS DC libraries.
 * 
 */

#ifndef __PAIRREF_H
#define __PAIRREF_H

#include "MOPSStructs.h"


/** @file
  * 
  * C-language functions to manipulate pairs of detections.
  *
  * All functions will have the \c modcp prefix.
  *
  * @see PairTP, DetectionTP
  *
  * @version Revision<!--$$Revision: 14 $-->on<!--$$Date: 2004-12-09 09:55:35 -1000 (Thu, 09 Dec 2004) $-->
  * @author Lisa A. Shannon,
  *         Last modified by: <!--$$Author: denneau $-->
  **/


/** @addtogroup modcp
  * @{
  * @copydoc PairRef.h
  **/

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup modcp_general General Utilities
  * Functions to manipulate and convert detection-pair objects.
  *
  * New detection-pairs can be created with the modcp_create() function, 
  * which will create a new object in memory (i.e., not stored in the
  * database).  The resulting MOPS-DC object pointer (a ::ModcPairTP) 
  * can be used to manipulate the object, including storing it in the 
  * database (see @ref modcp_dbin) and accessing data fields (see 
  * @ref modcp_access).
  * 
  * The MOPS-DC object pointer may reference a single object or an array of 
  * objects when it is the result of a database retrieval (see 
  * @ref modcp_db).  The number of objects can be obtained with the
  * modcp_getCount() function, and the various objects may be iterated
  * using the modcp_next() and modcp_prev() functions.
  *
  * All MOPS-DC object pointers can be converted to C-language data 
  * structures.  The modcp_toStruct() function is used to create a single
  * data structure, and the modcp_toStructArray() function will create
  * an array of data structures.
  *
  * Finally, the storage for a MOPS-DC object pointer can be released 
  * by use of the modcp_free() function.  This function should be called
  * when the object pointer is no longer needed.  Additional data structures
  * can be freed with the modcp_freeArray(), modcp_freeStruct(), and 
  * modcp_freeArrayStruct() functions.
  *
  * @note These functions do not perform any database activities.  However,
  *       an active database connection may be required for their use.  See
  *       modc_initialize(StringTP).
  *
  * @ingroup modcp
  **/

/** @defgroup modcp_access Accessor Functions
  * Functions to access values from the fields of a detection-pair object.
  * These functions operate on an existing detection-pair object, and do not 
  * perform a database retrieval.
  * @sample_code PairTests.c modcp_access
  * @ingroup modcp
  **/

/** @defgroup modcp_db Retrieval Functions
  * Functions to retrieve detection-pairs from the persistant database.
  * 
  * All retrievals will return a MOPS-DC pointer to the resulting
  * objects.  For most retrievals, this pointer will reference an array
  * of objects.  The modcp_getCount() function can be used to determine
  * how many objects were retrieved.  If no objects are retrieved, the
  * MOPS-DC pointer will be \c NULL.  See \ref modcp_general and 
  * \ref modcp_access for more details on using the MOPS-DC pointer.
  * @ingroup modcp
  **/

/** @defgroup modcp_dbin Storage Functions
  * Functions to store and update detection-pairs in the persistant database.
  * @ingroup modcp
  * Note that there is no \c update capability.  All pair values are
  * immutable once created (based on the original detections).
  **/

	
/** Converts the specified object pointer to a detection-pair data structure.
  * This function assumes that the ::ModcPairTP pointer was previously 
  * retrieved from the database. If \a ptr was not retrieved from the 
  * database (i.e., it was created using modcp_create()), this method will 
  * return \c NULL.
  *
  * @param ptr The MOPS-DC pointer to a detection-pair object.
  *
  * @return The equivalent detection-pair data structure (or \c NULL if \a ptr
  *         was not retrieved from the database).
  *
  * @note The caller of this function is responsible for freeing the space 
  *       allocated for the ::PairTP.  This may be done using \c free().
  * @note This function does not perform a retrieval from the database.
  *
  * @sample_code PairTests.c modcp_convertToPair
  * @ingroup modcp_general
  **/
PairTP modcp_toStruct(ModcPairTP ptr);

/** Converts all of the detection-pair objects associated with the specified
  * object pointer into an array of detection-pair data structures.
  * This function assumes that the ::ModcPairTP pointer was previously 
  * retrieved from the database. If \a ptr was not retrieved from the 
  * database (i.e., it was created using modcp_create()), this method will 
  * return \c NULL.
  *
  * The \a ptr will generally refer to a set of detection-pair objects.  
  * The returned array will be of size modcp_getCount(ModcPairTP). 
  *
  * @param ptr The MOPS-DC pointer to a set of detection-pair objects.
  *
  * @return The equivalent array of detection-pair data structures (or \c NULL 
  *         if \a ptr was not retrieved from the database).
  *
  * @note The caller of this function is responsible for freeing the space 
  *       allocated for the ::PairTP.  This may be done using \c free().
  * @note This function does not perform a retrieval from the database.
  *
  * @see modcp_getCount(ModcPairTP)
  *
  * @sample_code PairTests.c modcp_convertToPairArray
  * @ingroup modcp_general
  **/
PairArrayTP modcp_toStructArray(ModcPairTP ptr);

/** Returns the number of detection-pair objects associated with the specified
  * object pointer.  This function assumes that the ::ModcPairTP pointer was 
  * previously retrieved from the database. If \a ptr was not retrieved from 
  * the database (i.e., it was created using modcp_create()), this function 
  * will return <code>(int)</code>::Fail.
  *
  * @param ptr  The MOPS-DC pointer that originated from a detection-pair 
  *             retrieval.
  *
  * @return The number of detection-pairs associated with the ::ModcPairTP.
  *
  * @note This function does not perform a retrieval from the database.
  * @note If \a ptr was not retrieved from the database, this function will 
  *       return <code>(int)</code>::Fail.
  *
  * @see modcp_toStructArray()
  *
  * @sample_code PairTests.c modcp_convertToPairArray
  * @ingroup modcp_general
  **/
int modcp_getCount(ModcPairTP ptr);


/** Retrieves a single detection-pair based on a unique identifier.  This will
  * return \c NULL if the ID cannot be found.
  *
  * @param pairId   The ID of the detection-pair.
  *
  * @return The MOPS-DC pointer to the retrieved object set, or \c NULL.
  *
  * @note This function performs a retrieval from the database.
  *
  * @see modcp_toStruct(), modcp_toStructArray(), modcp_getCount()
  * 
  * @sample_code PairTests.c modcp_retrieve
  * @ingroup modcp_db
  **/
ModcPairTP modcp_retrieve(	IdTP pairId);

/** Retrieves all detection-pairs based on the unique identifier of a
  * detection.  This will return \c NULL if the detection ID is cannot be found
  * in any detection-pairs.
  *
  * @param det The ID of a detection.
  *
  * @return The MOPS-DC pointer to the retrieved object set, or \c NULL.
  *
  * @note This function performs a retrieval from the database.
  *
  * @see modcp_toStruct(), modcp_toStructArray(), modcp_getCount()
  *
  * @todo We might need to worry about this returning more than one...
  * It depends on whether a detection can be paired with more than one other
  * detection (e.g., det1->det2 and det2->det3 based on three consecutive
  * observations).
  *
  * @sample_code PairTests.c modcp_retrieveByDetection
  * @ingroup modcp_db
  **/
ModcPairTP modcp_retrieveByDetection(	IdTP det);

/** Create a new detection-pair object based on existing detections.  This 
  * function will not store the object in the database.  The first detection
  * should always be the earliest.  Both detections should already be stored
  * in the database before the pair is created.
  *
  * @param det1  The MOPS-DC pointer to the first detection of the pair.
  * @param det2  The MOPS-DC pointer to the second detection of the pair.
  *
  * @return ::Succeed if the detection-pair was successfully created, ::Fail 
  *         otherwise.
  *
  * @note This function does not perform any database operations.
  *
  * @sample_code PairTests.c modcp_create
  * @ingroup modcp_general
  **/
ModcPairTP modcp_create(ModcDetTP det1, ModcDetTP det2);

/** Retrieve the two detections contained in a specific detection-pair based
  * on the unique identifier of the pair.  This will return \c NULL if no 
  * matching detection-pair can be found.
  *
  * @param id   The ID of the detection-pair.
  *
  * @returns The array of MOPS-DC pointers for the two detections, or \c NULL.
  * 
  * @note This function performs a retrieval from the database.
  *
  * @ingroup modcp_db
  **/
ModcDetArrayTP modcp_retrieveDetectionsByPairId(	IdTP id);

/** Retrieve the two detections contained in a specific detection-pair.
  *
  * @param pair  The MOPS-DC pointer to the detection-pair.
  *
  * @returns The array of MOPS-DC pointers for the two detections, or \c NULL.
  * 
  * @note This function performs a retrieval from the database.
  *
  * @todo This might need review based on the caching implementation.
  * The question that comes to mind: Why are we forcing a new DB retrieval in
  * this case (only 2 detections) but seeming to do it automatically in other
  * situations that would be more costly (e.g., all detections with an 
  * observation)?
  * @todo Why use this one rather than ::modcp_getDets?
  *
  * @ingroup modcp_db
  **/
ModcDetArrayTP modcp_retrieveDetections(	ModcPairTP pair);

/** Retrieves all detection-pairs based on specific values. This will return 
  * \c NULL if no matching detection-pairs can be found.  The values and their
  * deltas will be used to define bounds used to test each detection in the
  * pair.  If either detection falls within the bounds, it will be returned.
  *
  * @param ra         The Right Ascension (degrees).
  * @param dec        The Declination (degrees).
  * @param epoch      The epoch of the detection (MJD).
  * @param vRa        The Right Ascension component of velocity vector 
  *                   (arcsec/day).
  * @param vDec       The Declination component of velocity vector (arcsec/day).
  * @param deltaRa    The value that will be +/- from \a ra to create a lower 
  *                   and upper bound for the Right Ascension.  If \a ra is 
  *                   ::DOUBLE_NIL then the \a deltaRA parameter will be 
  *                   ignored.
  * @param deltaDec   The value that will be +/- from \a dec to create a lower
  *                   and upper bound for the Declination.  If \a dec is 
  *                   ::DOUBLE_NIL then the \a deltaDec parameter will be 
  *                   ignored.
  * @param deltaEpoch The value that will be +/- from \a epoch to create a 
  *                   lower and upper bound for the epoch. 
  *                   If \a epoch is ::DOUBLE_NIL then the \a deltaEpoch 
  *                   parameter will be ignored.
  * @param deltaVRa   The value that will be +/- from \a vRa to create a 
  *                   lower and upper bound for the R.A. velocity component. 
  *                   If \a vRa is ::DOUBLE_NIL then the \a deltaVRa 
  *                   parameter will be ignored.
  * @param deltaVDec  The value that will be +/- from \a vDec to create a 
  *                   lower and upper bound for the Declination velocity component. 
  *                   If \a vDec is ::DOUBLE_NIL then the \a deltaVDec 
  *                   parameter will be ignored.
  *
  * @return The MOPS-DC pointer to the retrieved object set, or \c NULL.
  *
  * @note This function performs a retrieval from the database.
  *
  * @see modcd_toStruct(), modcd_toStructArray(), modcd_getCount()
  * 
  * @todo The delta of delta can get confusing here.  We need a 
  * \c modcp_retrieveByPattern(PairPatternTP) function.
  * @ingroup modcp_db
  **/
ModcPairTP modcp_retrieveByValue(   	double ra,
                                double dec,
                                MjdTP epoch,
                                double vRa,
                                double vDec,
                                double deltaRa,
                                double deltaDec,
                                double deltaEpoch,
                                double deltaVRa,
                                double deltaVDec);



/** Inserts a new detection-pair in the database.  The first detection should
  * always be the earliest detection.
  *
  * @param det1  The unique ID of the first detection.
  * @param det2  The unique ID of the second detection.
  *
  * @return ::Success if the storage was successful; ::Fail otherwise.
  *
  * @note This function performs an insert into the database.
  * @ingroup modcp_dbin
  **/
SucceedTP modcp_insertByIds(	IdTP det1,
								IdTP det2);

/** Inserts a new detection-pair in the database.  The detection-pair should
  * have been created with the modcp_create() function.
  *
  * @param pair  The MOPS-DC pointer of the detection-pair to insert.
  *
  * @return ::Success if the storage was successful; ::Fail otherwise.
  *
  * @note This function performs an insert into the database.
  * @ingroup modcp_dbin
  **/
SucceedTP	modcp_insert(	ModcPairTP pair);

/** Delete a detection-pair from the database.  The detection-pair must already
  * exist in the database.  The two detections will not be deleted, but they
  * will no longer be associated as a pair.
  *
  * @param pairId  The unique ID of the detection-pair to delete.
  *
  * @return ::Success if the detection-pair was successfully deleted, ::Fail 
  *         otherwise.
  *
  * @note This function performs a delete from the database.
  *
  * @sample_code PairTests.c modcp_delete
  * @ingroup modcp_dbin
  **/
SucceedTP modcp_delete(IdTP pairId);

/** Delete a detection-pair from the database.  The detection-pair must already
  * exist in the database.  The two detections will not be deleted, but they
  * will no longer be associated as a pair.
  *
  * @param det1  The unique ID of the first detection in the pair.
  * @param det2  The unique ID of the second detection in the pair.
  *
  * @return ::Success if the detection-pair was successfully deleted, ::Fail 
  *         otherwise.
  *
  * @note This function performs a delete from the database.
  *
  * @sample_code PairTests.c modcp_deleteByIds
  * @ingroup modcp_dbin
  **/
SucceedTP modcp_deleteByIds(IdTP det1, IdTP det2);

/** Free all memory associated with the MOPS-DC object pointer.  This will
  * clear all memory associated with all ::ModcPairTP information obtained from
  * a retrieval or create operation.  All detection-pair objects associated 
  * with a retrieval will be freed by a single call to this function.
  *
  * @param pair  The MOPS-DC object pointer to free.
  *
  * @return ::Success if the detection-pair was successfully deleted, ::Fail 
  *         otherwise.
  *
  * @note This function does not perform any database operations.
  *
  * @sample_code PairTests.c modcp_free
  * @ingroup modcp_general
  **/
SucceedTP modcp_free(ModcPairTP pair);

/** Free all memory associated with the MOPS-DC pointer array.  This will
  * clear memory associated with ::ModcPairArrayTP only.
  *
  * @param pair  The MOPS-DC object array pointer to free.
  *
  * @return ::Success if the array was successfully deleted, ::Fail 
  *         otherwise.
  *
  * @note This function does not perform any database operations.
  *
  * @sample_code PairTests.c modcp_freeArray
  * @ingroup modcp_general
  **/
SucceedTP modcp_freeArray(ModcPairArrayTP pair);

/** Free all memory associated with the pair structure pointer.  This will
  * clear memory associated with the ::PairTP only.  This should be called on
  * the result of the ::modcp_toStruct() function.
  *
  * @param pair  The structure pointer to free.
  *
  * @return ::Success if the detection-pair was successfully deleted, ::Fail 
  *         otherwise.
  *
  * @note This function does not perform any database operations.
  *
  * @sample_code PairTests.c modcp_freeStruct
  * @ingroup modcp_general
  **/
SucceedTP modcp_freeStruct(PairTP pair);

/** Free all memory associated with the array of pair structures.  This will
  * clear memory associated with the ::PairArrayTP only.  This should be called on
  * the result of the ::modcp_toStructArray() function.
  *
  * @param pair  The structure array to free.
  *
  * @return ::Success if the structure array was successfully deleted, ::Fail 
  *         otherwise.
  *
  * @note This function does not perform any database operations.
  *
  * @sample_code PairTests.c modcp_freeArrayStruct
  * @ingroup modcp_general
  **/
SucceedTP modcp_freeArrayStruct(PairArrayTP pair);

/* //////////////////////////GETTERS///////////////////////////////////////////////////////// */

/** Return the unique ID for the detection-pair object.
  * @param ptr  The MOPS-DC object to access.
  * @return The unique ID of the detection-pair object.
  * @note This function does not perform a retrieval from the database.
  * @ingroup modcp_access
  **/
IdTP modcp_getId( ModcPairTP ptr);

/** Return the set of two detections in this detection-pair.
  * @param ptr  The MOPS-DC object to access.
  * @return The array of two detections in this pair.
  * @note This function does not perform a retrieval from the database.
  * @ingroup modcp_access
  **/
ModcDetArrayTP modcp_getDets( ModcPairTP ptr);

/** Return the two Right Ascension values for this detection-pair.
  * @param ptr  The MOPS-DC object to access.
  * @return The array of two R.A. values for the detections in this pair.
  * @note This function does not perform a retrieval from the database.
  * @ingroup modcp_access
  **/
DoubleArrayTP modcp_getRa( ModcPairTP ptr);

/** Return the two Declination values for this detection-pair.
  * @param ptr  The MOPS-DC object to access.
  * @return The array of two Declination values for the detections in this pair.
  * @note This function does not perform a retrieval from the database.
  * @ingroup modcp_access
  **/
DoubleArrayTP modcp_getDec( ModcPairTP ptr);

/** Return the two epoch values for this detection-pair.
  * @param ptr  The MOPS-DC object to access.
  * @return The array of two epoch (MJD) values for the detections in this pair.
  * @note This function does not perform a retrieval from the database.
  * @ingroup modcp_access
  **/
MjdArrayTP modcp_getEpoch( ModcPairTP ptr);

/** Return the two magnitude values for this detection-pair.
  * @param ptr  The MOPS-DC object to access.
  * @return The array of two magnitude values for the detections in this pair.
  * @note This function does not perform a retrieval from the database.
  * @ingroup modcp_access
  **/
DoubleArrayTP modcp_getMag( ModcPairTP ptr);

/** Return the difference in Right Ascension between the two detections in this
  * detection-pair.  The difference is in arcseconds.
  * @param ptr  The MOPS-DC object to access.
  * @return The delta R.A. between the detections in this pair (arcsec).
  * @note This function does not perform a retrieval from the database.
  * @ingroup modcp_access
  **/
double modcp_getDeltaRa( ModcPairTP ptr);

/** Return the difference in Declination between the two detections in this
  * detection-pair.  The difference is in arcseconds.
  * @param ptr  The MOPS-DC object to access.
  * @return The delta Declination between the detections in this pair (arcsec).
  * @note This function does not perform a retrieval from the database.
  * @ingroup modcp_access
  **/
double modcp_getDeltaDec( ModcPairTP ptr);

/** Return the Right Ascension component of the velocity vector of this 
  * detection-pair.  The velocity is represented in arcsec/day.
  * @param ptr  The MOPS-DC object to access.
  * @return The R.A. component of the velocity vector for this pair
  *         (arcsec/day).
  * @note This function does not perform a retrieval from the database.
  * @ingroup modcp_access
  **/
double modcp_getVRa( ModcPairTP ptr);

/** Return the Declination component of the velocity vector of this 
  * detection-pair.  The velocity is represented in arcsec/day.
  * @param ptr  The MOPS-DC object to access.
  * @return The Dec component of the velocity vector for this pair (arcsec/day).
  * @note This function does not perform a retrieval from the database.
  * @ingroup modcp_access
  **/
double modcp_getVDec( ModcPairTP ptr);

/** Return the next detection-pair object in the pool of available 
  * detection-pairs retrieved from the database.  If \a ptr is referencing 
  * the last detection-pair in the pool, this function will return \c NULL.  
  * If \a ptr was not created from a database retrieval, this function will
  * always return \c NULL.
  *
  * @param ptr  The MOPS-DC object pointer retrieved from the database.
  *
  * @return The next pair in the retrieved set (or \c NULL).
  *
  * @note This function does not perform a retrieval from the database.
  *
  * @sample_code PairTests.c modcp_next
  * @ingroup modcp_general
  **/
ModcPairTP modcp_next( ModcPairTP ptr);

/** Return the previous detection-pair object in the pool of available 
  * detection-pairs retrieved from the database.  If \a ptr is referencing 
  * the first detection-pair in the pool, this function will return \c NULL.
  * If \a ptr was not created from a database retrieval, this function will 
  * always return \c NULL.
  *
  * @param ptr  The MOPS-DC object pointer retrieved from the database.
  *
  * @return The previous pair in the retrieved set (or \c NULL).
  *
  * @note This function does not perform a retrieval from the database.
  *
  * @ingroup modcp_general
  **/
ModcPairTP modcp_prev( ModcPairTP ptr);

/** Return the unique ID of the first detection in this detection-pair.
  * @param ptr  The MOPS-DC object to access.
  * @return The unique ID of the first detection in this pair.
  * @note This function does not perform a retrieval from the database.
  * @ingroup modcp_access
  **/
IdTP modcp_getId1( ModcPairTP ptr);

/** Return the unique ID of the second detection in this detection-pair.
  * @param ptr  The MOPS-DC object to access.
  * @return The unique ID of the second detection in this pair.
  * @note This function does not perform a retrieval from the database.
  * @ingroup modcp_access
  **/
IdTP modcp_getId2( ModcPairTP ptr);

/** @} */

#ifdef __cplusplus
} // end extern "C"
#endif

#endif

