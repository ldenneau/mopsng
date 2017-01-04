/* ----------------------------------------------------------------------------
 *                           * * * UNCLASSIFIED * * *
 * ----------------------------------------------------------------------------
 *
 * $Logfile: /Software/libraries/mops-interface-c/ObservationRef.h $
 *
 * ----------------------------------------------------------------------------
 *
 * $Workfile: ObservationRef.h $
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
 * $History: ObservationRef.h $
 * 
 * *****************  Version 29  *****************
 * User: Wayne L. Smith Date: 9/21/04    Time: 6:10p
 * Updated in $/Software/libraries/mops-interface-c
 * Added getDetections() function.  Reversed order of start/stop time
 * arguments.
 * 
 * *****************  Version 28  *****************
 * User: Wayne L. Smith Date: 9/21/04    Time: 5:53p
 * Updated in $/Software/libraries/mops-interface-c
 * Final documentation text review.
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
 * User: Lisa A. Shannon Date: 9/10/04    Time: 4:08p
 * Updated in $/Software/libraries/mops-interface-c
 * modified #ifdef cplusplus to #Ifdef __cplusplus
 * __cplusplus is recognized by gcc and g++ where cplusplus (no
 * underscores) is only recognized by g++
 * 
 * *****************  Version 24  *****************
 * User: Lisa A. Shannon Date: 9/03/04    Time: 12:16p
 * Updated in $/Software/libraries/mops-interface-c
 * added note to delete() that it is not yet implemented
 * 
 * *****************  Version 23  *****************
 * User: Lisa A. Shannon Date: 9/02/04    Time: 9:41a
 * Updated in $/Software/libraries/mops-interface-c
 * added freeArray(), freeStruct(), freeStructArray()
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
 * User: Lisa A. Shannon Date: 8/12/04    Time: 3:36p
 * Updated in $/Software/libraries/mops-interface-c
 * changed retrieveById methods to be retrieve (i.e removed the ById)
 * 
 * *****************  Version 17  *****************
 * User: Lisa A. Shannon Date: 8/12/04    Time: 3:03p
 * Updated in $/Software/libraries/mops-interface-c
 * changed all methods that retrieve from the database from get... to
 * retrieve...
 * 
 * *****************  Version 16  *****************
 * User: Lisa A. Shannon Date: 8/12/04    Time: 10:42a
 * Updated in $/Software/libraries/mops-interface-c
 * 
 * *****************  Version 15  *****************
 * User: Wayne L. Smith Date: 8/12/04    Time: 9:58a
 * Updated in $/Software/libraries/mops-interface-c
 * Fixed extern "C" issue for C code.
 * 
 * *****************  Version 14  *****************
 * User: Lisa A. Shannon Date: 8/11/04    Time: 8:41p
 * Updated in $/Software/libraries/mops-interface-c
 * Documentation changes
 * 
 * *****************  Version 13  *****************
 * User: Lisa A. Shannon Date: 8/11/04    Time: 4:51p
 * Updated in $/Software/libraries/mops-interface-c
 * 1.  Modified modcm_create to return  ModcObsTP
 * 2.  changed decSigma in create to be a double
 * 3.  MOdified 2nd parameter for setObservationDetections to be a
 * ModcDetArrayTP
 * 
 * *****************  Version 12  *****************
 * User: Lisa A. Shannon Date: 8/11/04    Time: 3:00p
 * Updated in $/Software/libraries/mops-interface-c
 * Made all methods that return string return StringTP
 * 
 * *****************  Version 11  *****************
 * User: Lisa A. Shannon Date: 8/11/04    Time: 2:25p
 * Updated in $/Software/libraries/mops-interface-c
 * Removed update method since objects of this kind are immutable
 * 
 * *****************  Version 10  *****************
 * User: Lisa A. Shannon Date: 8/11/04    Time: 2:15p
 * Updated in $/Software/libraries/mops-interface-c
 * More Documentation Changes
 * 
 * *****************  Version 9  *****************
 * User: Lisa A. Shannon Date: 8/11/04    Time: 1:15p
 * Updated in $/Software/libraries/mops-interface-c
 * Changed <p>Notes:  to \note
 * 
 * *****************  Version 8  *****************
 * User: Lisa A. Shannon Date: 8/11/04    Time: 12:57p
 * Updated in $/Software/libraries/mops-interface-c
 * Made documentation changes:1.  Implemented ModcDetTP, ModcOrbitTP,
 * ModcObsTP, FIlterTP2.  Enchanced documentation
 * 
 * *****************  Version 7  *****************
 * User: Wayne L. Smith Date: 8/11/04    Time: 8:44a
 * Updated in $/Software/libraries/mops-interface-c
 * Moved lower-level layer header files into the implementation (*.cpp)
 * rather than this layer's header files.
 * 
 * *****************  Version 6  *****************
 * User: Wayne L. Smith Date: 8/10/04    Time: 7:15p
 * Updated in $/Software/libraries/mops-interface-c
 * Cleaned up documentation using groups.  Refactored all functions to use
 * a prefix specific to MopsObsTP DC libraries.
 * 
 */

#ifndef __OBSERVATIONREF_H
#define __OBSERVATIONREF_H

#include "MOPSStructs.h"

/** @file
  * 
  * C-language functions to manipulate observations (i.e., metadata).
  *
  * All functions will have the \c modcm prefix.
  *
  * @see ObservationTP
  *
  * @version Revision<!--$$Revision: 14 $-->on<!--$$Date: 2004-12-09 09:55:35 -1000 (Thu, 09 Dec 2004) $-->
  * @author Lisa A. Shannon,
  *         Last modified by: <!--$$Author: denneau $-->
  **/

/** @addtogroup modcm
  * @{
  * @copydoc ObservationRef.h
  **/

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup modcm_general General Utilities
  * Functions to manipulate and convert observation (metadata) objects.
  * 
  * New observations can be created with the modcm_create() function, 
  * which will create a new object in memory (i.e., not stored in the
  * database).  The resulting MOPS-DC object pointer (a ::ModcObsTP) 
  * can be used to manipulate the object, including storing it in the 
  * database (see @ref modcm_dbin) and accessing data fields (see 
  * @ref modcm_access).
  * 
  * The MOPS-DC object pointer may reference a single object or an array of 
  * objects when it is the result of a database retrieval (see 
  * @ref modcm_db).  The number of objects can be obtained with the
  * modcm_getCount() function, and the various objects may be iterated
  * using the modcm_next() and modcm_prev() functions.
  *
  * All MOPS-DC object pointers can be converted to C-language data 
  * structures.  The modcm_toStruct() function is used to create a single
  * data structure, and the modcm_toStructArray() function will create
  * an array of data structures.
  *
  * Finally, the storage for a MOPS-DC object pointer can be released 
  * by use of the modcm_free() function.  This function should be called
  * when the object pointer is no longer needed.  Additional data structures
  * can be freed with the modcm_freeArray(), modcm_freeStruct(), and 
  * modcm_freeArrayStruct() functions.
  *
  * @note These functions do not perform any database activities.  However,
  *       an active database connection may be required for their use.  See
  *       modc_initialize(StringTP).
  *
  * @ingroup modcm
  **/

/** @defgroup modcm_access Accessor Functions
  * Functions to access values from the fields of an observation object.
  * These functions operate on an existing detection object, and do not 
  * perform a database retrieval.
  * @sample_code ObsTests.c modcm_access
  * @ingroup modcm
  **/

/** @defgroup modcm_db Retrieval Functions
  * Functions to retrieve observations from the persistant database.
  * 
  * All retrievals will return a MOPS-DC pointer to the resulting
  * objects.  For most retrievals, this pointer will reference an array
  * of objects.  The modcm_getCount() function can be used to determine
  * how many objects were retrieved.  If no objects are retrieved, the
  * MOPS-DC pointer will be \c NULL.  See \ref modcm_general and 
  * \ref modcm_access for more details on using the MOPS-DC pointer.
  * @ingroup modcm
  **/

/** @defgroup modcm_dbin Storage Functions
  * Functions to store and update observations in the persistant database.
  * @ingroup modcm
  * Note that there is no \c update capability.  All metadata values are
  * immutable once created.  Additional detections may be associated with
  * an existing observation with the ::modcm_addDetections() function.
  **/


/** Converts the specified object pointer to an observation data structure.
  * This function assumes that the ::ModcObsTP pointer was previously 
  * retrieved from the database. If \a ptr was not retrieved from the 
  * database (i.e., it was created using modcm_create()), this method will 
  * return \c NULL.
  *
  * @param ptr The MOPS-DC pointer to an observation object.
  *
  * @return The equivalent observation data structure (or \c NULL if \a ptr
  *         was not retrieved from the database).
  *
  * @note The caller of this function is responsible for freeing the space 
  *       allocated for the ::ObservationTP.  This may be done using \c free().
  * @note This function does not perform a retrieval from the database.
  *
  * @sample_code ObsTests.c modcm_convertToObservation
  * @ingroup modcm_general
  **/
ObservationTP modcm_toStruct(ModcObsTP ptr);

/** Converts all of the observation objects associated with the specified
  * object pointer into an array of observation data structures.
  * This function assumes that the ::ModcObsTP pointer was previously 
  * retrieved from the database. If \a ptr was not retrieved from the 
  * database (i.e., it was created using modcm_create()), this method will 
  * return \c NULL.
  *
  * The \a ptr will generally refer to a set of observation objects.  
  * The returned array will be of size modcm_getCount(ModcObsTP). 
  *
  * @param ptr The MOPS-DC pointer to a set of observation objects.
  *
  * @return The equivalent array of observation data structures (or \c NULL 
  *         if \a ptr was not retrieved from the database).
  *
  * @note The caller of this function is responsible for freeing the space 
  *       allocated for the ::ObservationTP.  This may be done using \c free().
  * @note This function does not perform a retrieval from the database.
  *
  * @see modcm_getCount(ModcObsTP)
  *
  * @sample_code ObsTests.c modcm_convertToObservationArray
  * @ingroup modcm_general
  **/
ObservationArrayTP modcm_toStructArray(ModcObsTP ptr);

/** Returns the number of observation objects associated with the specified
  * object pointer.  This function assumes that the ::ModcObsTP pointer was 
  * previously retrieved from the database. If \a ptr was not retrieved from 
  * the database (i.e., it was created using modcm_create()), this function 
  * will return <code>(int)</code>::Fail.
  *
  * @param ptr  The MOPS-DC pointer that originated from an observation 
  *             retrieval.
  *
  * @return The number of observation associated with the ::ModcObsTP.
  *
  * @note This function does not perform a retrieval from the database.
  * @note If \a ptr was not retrieved from the database, this function will 
  *       return <code>(int)</code>::Fail.
  *
  * @see modcm_toStructArray()
  *
  * @sample_code ObsTests.c modcm_convertToObservationArray
  * @ingroup modcm_general
  **/
int modcm_getCount(ModcObsTP ptr);

/** Return the number of detections associated with this observation.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The detection count for the orbit.
  *
  * @note This method does not perform a retrieval from the database.
  *
  * @todo Caching strategy.  Do we really want to retrieve all detections for
  * an observation whenever we pull an observation record?  Granted, we can
  * by pretty clever behind the scenes, and only pull these things (count, 
  * etc.) when the functions are called (but we'd need to document it as 
  * performing DB activity, rather than saying "none").
  *
  * @see modcm_getDetections()
  *
  * @ingroup modcm_access
  **/
int modcm_getDetectionCount(ModcObsTP ptr);

/** Return the set of detections associated with this observation.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The array of MOPS-DC detection objects.
  *
  * @note This method does not perform a retrieval from the database.
  *
  * @see modcm_getDetectionCount()
  *
  * @ingroup modcm_access
  **/
ModcDetArrayTP modcm_getDetections( ModcObsTP ptr);

/** Create a new observation object.  This function will not store the object
  * in the database.
  *
  * Passing the ::DOUBLE_NIL value as any of the \c double parameters 
  * indicates that the parameter should remain uninitialized.  This is 
  * permitted for the following parameters:
  * @todo List the allowable ::DOUBLE_NIL parameters to create.
  *
  * @param epoch        The epoch of the observation (MJD, full precision).
  * @param ra			The Right Ascension (degrees).
  * @param dec          The declination (degrees).
  * @param surveyMode	The survey mode.
  * @param timeStart    Start time of the observation (MJD).
  * @param timeStop		Stop time of the observation (MJD).
  * @param filter		The filter for this observation.
  * @param limitingMag	The limiting magnitude of this observation (magnitude). 
  * @param raSigma		Error in right ascension (arcsec).
  * @param decSigma		Error in declination (arcsec).
  * @param observatory	The IAU/MPC Observatory Code.
  * @param de			Detection efficiency parameters.
  *
  * @note This function does not perform any database operations.
  * @note If any \a double value is ::DOUBLE_NIL then the parameter will remain
  *       uninitialized.  If a required parameter is uninitialized, this
  *       function will return \c NULL.
  *
  * @see modcm_addDetections(ModcObsTP, ModcDetArrayTP)
  * @ingroup modcm_general
  **/
ModcObsTP modcm_create( 	MjdTP epoch,
        					double ra,
							double dec,
        					SurveyTP surveyMode,
        					MjdTP timeStart,
       						MjdTP timeStop,
        					FilterTP filter,
        					double limitingMag,
        					double raSigma,
        					double decSigma,
							ObservatoryTP observatory,
							DoubleArrayTP de);

/** Retrieves a single observation based on a unique identifier.  This will
  * return \c NULL if the ID cannot be found.
  *
  * @param id  The ID of the observation.
  *
  * @return The MOPS-DC pointer to the retrieved object set, or \c NULL.
  *
  * @note This function performs a retrieval from the database.
  *
  * @see modcm_toStruct(), modcm_toStructArray(), modcm_getCount()
  * 
  * @ingroup modcm_db
  **/
ModcObsTP  modcm_retrieve(	IdTP id);

/** Retrieves all orbits based on specific values.  This will return \c NULL if
  * no matching orbits can be found.
  *
  * @param epoch       The epoch of the observation (MJD).
  * @param ra          The Right Ascension (degrees).
  * @param dec         The Declination (degrees).
  * @param deltaEpoch  The value that will be +/- from \a epoch to create
  *                    a lower and upper bound for the epoch of the 
  *                    observation. If \a epoch is ::DOUBLE_NIL then the
  *                    \a deltaEpoch parameter will be ignored.
  * @param deltaRa     The value that will be +/- from \a ra to create
  *                    a lower and upper bound for the R.A. of the 
  *                    observation. If \a ra is ::DOUBLE_NIL then the
  *                    \a deltaRa parameter will be ignored.
  * @param deltaDec    The value that will be +/- from \a dec to create
  *                    a lower and upper bound for the Declination of the 
  *                    observation. If \a dec is ::DOUBLE_NIL then the
  *                    \a deltaDec parameter will be ignored.
  *
  * @todo Alternative: Something like \c modcm_retrieveByPattern(ObsPatternTP)
  * where \c ObsPatternTP is a structure containing ranges for the various
  * fields and a flag indicating whether they should be used.
  *
  * @return The MOPS-DC pointer to the retrieved object set, or \c NULL.
  *
  * @note This function performs a retrieval from the database.
  *
  * @see modcm_toStruct(), modcm_toStructArray(), modcm_getCount()
  * 
  * @ingroup modcm_db
  **/
ModcObsTP modcm_retrieveByValue(  	MjdTP epoch,
                                	double ra,
                                	double dec,
                                	double deltaEpoch,
                                	double deltaRa,
                                	double deltaDec);

/** Inserts a previously created observation object into the database.  The 
  * \a obs should have been previously created with the modcm_create() function.
  *
  * @param obs  The MOPS-DC pointer to the observation to insert.
  *
  * @return ::Success if the observation was successfully inserted, ::Fail 
  *         otherwise.
  *
  * @note This function performs an insert into the database.
  *         
  * @see modcm_insertByValue()
  *
  * @ingroup modcm_dbin
  **/
SucceedTP modcm_insert(ModcObsTP obs);

/** Creates and inserts an observation object into the database. 
  *
  * Passing the ::DOUBLE_NIL value as any of the \c double parameters 
  * indicates that the parameter should remain uninitialized.  This is 
  * permitted for the following parameters:
  * @todo List the allowable ::DOUBLE_NIL parameters to create.
  *
  * @param epoch        The epoch of the observation (MJD, full precision).
  * @param ra			The Right Ascension (degrees).
  * @param dec          The declination (degrees).
  * @param surveyMode	The survey mode.
  * @param timeStart    Start time of the observation (MJD).
  * @param timeStop		Stop time of the observation (MJD).
  * @param filter		The filter for this observation.
  * @param limitingMag	The limiting magnitude of this observation (magnitude). 
  * @param raSigma		Error in right ascension (arcsec).
  * @param decSigma		Error in declination (arcsec).
  * @param observatory	The IAU/MPC Observatory Code.
  * @param de			Detection efficiency parameters.
  *
  * @return The MOPS-DC pointer to the created observation, or \c NULL if the 
  *         insert failed.
  *
  * @note This function performs an insert into the database.
  * @note If any \c double value is ::DOUBLE_NIL then the parameter will 
  *       remain uninitialized.  
  *       If any ::StringTP value is \c NULL then the parameter will remain
  *       uninitialized.
  *       If a required parameter is uninitialized, this function will return 
  *       \c NULL.
  *         
  * @see modcm_create(), modcm_insert(ModcObsTP)
  *
  * @ingroup modcm_dbin
  **/
ModcObsTP modcm_insertByValue( 	MjdTP epoch,
        					double ra,
							double dec,
        					SurveyTP surveyMode,
        					MjdTP timeStart,
       						MjdTP timeStop,
        					FilterTP filter,
        					double limitingMag,
        					double raSigma,
        					double decSigma,
							ObservatoryTP observatory,
							DoubleArrayTP de);

/** Associate a set of detections with an observation.  The observation must
  * exist in the database.  None of the detections can exist in the
  * database.
  *
  * @param obs   The MOPS-DC observation object.
  * @param dets  The array of MOPS-DC detection objects.
  *
  * @return ::Success if the detections were successfully added, ::Fail
  *         otherwise.
  *
  * @note This function performs an insert into the database.
  * @ingroup modcm_dbin
  **/
SucceedTP modcm_addDetections(	ModcObsTP obs,
								ModcDetArrayTP dets);

/** Delete an observation from the database.  The observation must already exist
  * in the database.  All detections associated with the observation will
  * also be deleted.
  *
  * @param obsId  The unique ID of the observation to delete.
  *
  * @return ::Success if the observation was successfully deleted, ::False 
  *         otherwise.
  *
  * @note This function performs a delete from the database.
  *
  * @ingroup modcm_dbin
  **/
SucceedTP modcm_delete(IdTP obsId);

/** Free all memory associated with the MOPS-DC object pointer.  This will
  * clear all memory associated with all ::ModcObsTP information obtained from
  * a retrieval or create operation.  All observations (and detection) objects
  * associated with a retrieval will be freed by a single call to this 
  * function.
  *
  * @param obs  The MOPS-DC object pointer to free.
  *
  * @return ::Success if the observation was successfully deleted, ::Fail
  *         otherwise.
  *
  * @note This function does not perform any database operations.
  *
  * @ingroup modcm_general
  **/
SucceedTP modcm_free(ModcObsTP obs);	

/** Free all memory associated with the MOPS-DC pointer array.  This will
  * clear memory associated with ::ModcObsArrayTP only.
  *
  * @param obs  The MOPS-DC object array pointer to free.
  *
  * @return ::Success if the array was successfully deleted, ::Fail 
  *         otherwise.
  *
  * @note This function does not perform any database operations.
  *
  * @ingroup modcm_general
  **/
SucceedTP modcm_freeArray(ModcObsArrayTP obs);	

/** Free all memory associated with the observation structure pointer.  This will
  * clear memory associated with the ::ObservationTP only.  This should be called on
  * the result of the ::modcm_toStruct() function.
  *
  * @param obs  The structure pointer to free.
  *
  * @return ::Success if the detection-pair was successfully deleted, ::Fail 
  *         otherwise.
  *
  * @note This function does not perform any database operations.
  *
  * @ingroup modcm_general
  **/
SucceedTP modcm_freeStruct(ObservationTP obs);

/** Free all memory associated with the array of observation structures.  This will
  * clear memory associated with the ::ObservationArrayTP only.  This should be called on
  * the result of the ::modcm_toStructArray() function.
  *
  * @param obs  The structure array to free.
  *
  * @return ::Success if the structure array was successfully deleted, ::Fail 
  *         otherwise.
  *
  * @note This function does not perform any database operations.
  *
  * @ingroup modcm_general
  **/
SucceedTP modcm_freeArrayStruct(ObservationArrayTP obs);

/* ///////////////////////GETTERS//////////////////////////////////////// */

/** Return the unique ID of the observation object.  This will be \c NULL if 
  * the observation was not retrieved from the database.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The unique ID of the observation.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modcm_access
  **/
IdTP modcm_getId(ModcObsTP ptr);

/** Return the Right Ascension (degrees) of the observation object.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The R.A. of the observation.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modcm_access
  **/
double modcm_getRa(ModcObsTP ptr);

/** Return the Declination (degrees) of the observation object.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The Declination of the observation.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modcm_access
  **/
double modcm_getDec(ModcObsTP ptr);

/** Return the epoch (MJD) of the observation object.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The epoch of the observation.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modcm_access
  **/
MjdTP modcm_getEpoch(ModcObsTP ptr);

/** Return the survey mode for the observation object.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The survey mode of the observation.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modcm_access
  **/
SurveyTP modcm_getSurveyMode(ModcObsTP ptr);

/** Return the stop time (MJD) of the observation object.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The stop time of the observation.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modcm_access
  **/
MjdTP modcm_getTimeStop(ModcObsTP ptr);

/** Return the start time (MJD) of the observation object.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The start time of the observation.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modcm_access
  **/
MjdTP modcm_getTimeStart(ModcObsTP ptr);

/** Return the filter for the observation object.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The filter for the observation.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modcm_access
  **/
FilterTP modcm_getFilter(ModcObsTP ptr);

/** Return the limiting magnitude of the observation object.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The limiting magnitude of the observation.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modcm_access
  **/
double modcm_getLimitingMag(ModcObsTP ptr);

/** Return the error in Right Ascension (arcsec) for the observation object.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The R.A. error for the observation.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modcm_access
  **/
double modcm_getRaSigma(ModcObsTP ptr);

/** Return the error in Declination (arcsec) for the observation object.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The Declination error for the observation.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modcm_access
  **/
double modcm_getDecSigma (ModcObsTP ptr);

/** Return the IAU/MPC observatory code for the observation object.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The observatory code for the observation.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modcm_access
  **/
ObservatoryTP modcm_getObservatory (ModcObsTP ptr);

/** Return the array of detection efficiencies for the observation object.
  *
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The detection efficiencies of the observation.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modcm_access
  **/
DoubleArrayTP modcm_getEfficiencies(ModcObsTP ptr);

/** Return the next observation object in the pool of available observations
  * retrieved from the database.  If \a ptr is referencing the last 
  * observation in the pool, this function will return \c NULL.  If \a ptr
  * was not created from a database retrieval, this function will always
  * return \c NULL.
  *
  * @param ptr  The MOPS-DC object pointer retrieved from the database.
  *
  * @return The next observation in the retrieved set (or \c NULL).
  *
  * @note This function does not perform a retrieval from the database.
  *
  * @sample_code ObsTests.c modcm_next
  * @ingroup modcm_general
  **/
ModcObsTP modcm_next(ModcObsTP ptr);

/** Return the previous observation object in the pool of available 
  * observations retrieved from the database.  If \a ptr is referencing the 
  * first observation in the pool, this function will return \c NULL.  If 
  * \a ptr was not created from a database retrieval, this function will 
  * always return \c NULL.
  *
  * @param ptr  The MOPS-DC object pointer retrieved from the database.
  *
  * @return The previous observation in the retrieved set (or \c NULL).
  *
  * @note This function does not perform a retrieval from the database.
  *
  * @ingroup modcm_general
  **/
ModcObsTP modcm_prev(ModcObsTP ptr);

/** Return the next detection associated with the specified observation.
  * If \a ptr is referencing the last detection in the pool, this will return
  * \c NULL.  If \a ptr was not created from a database retrieval, this 
  * function will always return \c NULL.
  *
  * @param ptr  The MOPS-DC observation retrieved from the database.
  *
  * @return The next associated detection (or \c NULL).
  *
  * @note This function does not perform a retrieval from the database.
  *
  * @see modcm_getDetections()
  * @ingroup modcm_access
  **/
ModcDetTP modcm_nextDetection(ModcObsTP ptr);

/** Return the previous detection associated with the specified observation.
  * If \a ptr is referencing the first detection in the pool, this will return
  * \c NULL.  If \a ptr was not created from a database retrieval, this 
  * function will always return \c NULL.
  *
  * @param ptr  The MOPS-DC observation retrieved from the database.
  *
  * @return The next associated detection (or \c NULL).
  *
  * @note This function does not perform a retrieval from the database.
  *
  * @see modcm_getDetections()
  * @ingroup modcm_access
  **/
ModcDetTP modcm_prevDetection(ModcObsTP ptr);


/** @} */

#ifdef __cplusplus
} // end extern "C"
#endif

#endif
