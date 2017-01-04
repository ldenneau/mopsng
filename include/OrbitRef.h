/* ----------------------------------------------------------------------------
 *                           * * * UNCLASSIFIED * * *
 * ----------------------------------------------------------------------------
 *
 * $Logfile: /Software/libraries/mops-interface-c/OrbitRef.h $
 *
 * ----------------------------------------------------------------------------
 *
 * $Workfile: OrbitRef.h $
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
 * $History: OrbitRef.h $
 * 
 * *****************  Version 22  *****************
 * User: Wayne L. Smith Date: 9/21/04    Time: 10:36a
 * Updated in $/Software/libraries/mops-interface-c
 * Updated documentation.
 * 
 * *****************  Version 21  *****************
 * User: Wayne L. Smith Date: 9/20/04    Time: 9:45a
 * Updated in $/Software/libraries/mops-interface-c
 * Partial cleanup of documentation.
 * 
 * *****************  Version 20  *****************
 * User: Wayne L. Smith Date: 9/18/04    Time: 6:27p
 * Updated in $/Software/libraries/mops-interface-c
 * Updated documentation.
 * 
 * *****************  Version 19  *****************
 * User: Wayne L. Smith Date: 9/15/04    Time: 2:30p
 * Updated in $/Software/libraries/mops-interface-c
 * Fixed doxygen errros for the "free" functions.
 * 
 * *****************  Version 18  *****************
 * User: Lisa A. Shannon Date: 9/10/04    Time: 4:08p
 * Updated in $/Software/libraries/mops-interface-c
 * modified #ifdef cplusplus to #Ifdef __cplusplus
 * __cplusplus is recognized by gcc and g++ where cplusplus (no
 * underscores) is only recognized by g++
 * 
 * *****************  Version 17  *****************
 * User: Lisa A. Shannon Date: 9/02/04    Time: 9:44a
 * Updated in $/Software/libraries/mops-interface-c
 * added freeArray(), freeStruct(), freeStructArray()
 * 
 * *****************  Version 16  *****************
 * User: Wayne L. Smith Date: 8/13/04    Time: 2:07a
 * Updated in $/Software/libraries/mops-interface-c
 * Fully scrubbed Detection documentation.  Partially scrubbed Orbits,
 * Pairs, Observations.
 * 
 * *****************  Version 15  *****************
 * User: Wayne L. Smith Date: 8/12/04    Time: 8:56p
 * Updated in $/Software/libraries/mops-interface-c
 * Created lots of data types to better indicate expectations.  Changed a
 * few function signatures.  Added some descriptive documentation.
 * 
 * *****************  Version 14  *****************
 * User: Lisa A. Shannon Date: 8/12/04    Time: 5:17p
 * Updated in $/Software/libraries/mops-interface-c
 * Added ...._free method to allow the user to free any Mops...TP pointer
 * 
 * *****************  Version 13  *****************
 * User: Lisa A. Shannon Date: 8/12/04    Time: 3:51p
 * Updated in $/Software/libraries/mops-interface-c
 * changed all references to getNext to next 
 * changed all references to getPrev to prev
 * 
 * *****************  Version 12  *****************
 * User: Lisa A. Shannon Date: 8/12/04    Time: 3:36p
 * Updated in $/Software/libraries/mops-interface-c
 * changed retrieveById methods to be retrieve (i.e removed the ById)
 * 
 * *****************  Version 11  *****************
 * User: Lisa A. Shannon Date: 8/12/04    Time: 3:03p
 * Updated in $/Software/libraries/mops-interface-c
 * changed all methods that retrieve from the database from get... to
 * retrieve...
 * 
 * *****************  Version 10  *****************
 * User: Lisa A. Shannon Date: 8/12/04    Time: 10:42a
 * Updated in $/Software/libraries/mops-interface-c
 * 
 * *****************  Version 9  *****************
 * User: Lisa A. Shannon Date: 8/12/04    Time: 10:04a
 * Updated in $/Software/libraries/mops-interface-c
 * Removed updated parameter from all update and set methods.  Updated is
 * set by the database
 * 
 * *****************  Version 8  *****************
 * User: Wayne L. Smith Date: 8/12/04    Time: 10:00a
 * Updated in $/Software/libraries/mops-interface-c
 * Fixed type mismatch.
 * 
 * *****************  Version 7  *****************
 * User: Wayne L. Smith Date: 8/12/04    Time: 9:58a
 * Updated in $/Software/libraries/mops-interface-c
 * Fixed extern "C" issue for C code.
 * 
 * *****************  Version 6  *****************
 * User: Lisa A. Shannon Date: 8/11/04    Time: 8:41p
 * Updated in $/Software/libraries/mops-interface-c
 * Documentation changes
 * 
 * *****************  Version 5  *****************
 * User: Lisa A. Shannon Date: 8/11/04    Time: 2:18p
 * Updated in $/Software/libraries/mops-interface-c
 * Documentation Changes
 * 
 * *****************  Version 4  *****************
 * User: Wayne L. Smith Date: 8/11/04    Time: 8:44a
 * Updated in $/Software/libraries/mops-interface-c
 * Moved lower-level layer header files into the implementation (*.cpp)
 * rather than this layer's header files.
 * 
 * *****************  Version 3  *****************
 * User: Wayne L. Smith Date: 8/10/04    Time: 7:15p
 * Updated in $/Software/libraries/mops-interface-c
 * Cleaned up documentation using groups.  Refactored all functions to use
 * a prefix specific to ModcOrbitTP DC libraries.
 *
 */

#ifndef __ORBITREF_H
#define __ORBITREF_H

#include "MOPSStructs.h"


/** @file
  * 
  * C-language functions to manipulate orbits.
  *
  * All functions will have the \c modco prefix.
  *
  * @see OrbitTP
  *
  * @version Revision<!--$$Revision: 14 $-->on<!--$$Date: 2004-12-09 09:55:35 -1000 (Thu, 09 Dec 2004) $-->
  * @author Lisa A. Shannon,
  *         Last modified by: <!--$$Author: denneau $-->
  **/


/** @addtogroup modco
  * @{
  * @copydoc OrbitRef.h
  **/

#ifdef __cplusplus
extern "C" {
#endif


/** @defgroup modco_general General Utilities
  * Functions to manipulate and convert orbit objects.
  *
  * New orbits can be created with the modco_create() function, 
  * which will create a new object in memory (i.e., not stored in the
  * database).  The resulting MOPS-DC object pointer (a ::ModcOrbitTP) 
  * can be used to manipulate the object, including storing it in the 
  * database (see @ref modco_dbin) and accessing data fields (see 
  * @ref modco_access).
  * 
  * The MOPS-DC object pointer may reference a single object or an array of 
  * objects when it is the result of a database retrieval (see 
  * @ref modco_db).  The number of objects can be obtained with the
  * modco_getCount() function, and the various objects may be iterated
  * using the modco_next() and modco_prev() functions.
  *
  * All MOPS-DC object pointers can be converted to C-language data 
  * structures.  The modco_toStruct() function is used to create a single
  * data structure, and the modco_toStructArray() function will create
  * an array of data structures.
  *
  * Finally, the storage for a MOPS-DC object pointer can be released 
  * by use of the modco_free() function.  This function should be called
  * when the object pointer is no longer needed.  Additional data structures
  * can be freed with the modco_freeArray(), modco_freeStruct(), and 
  * modco_freeArrayStruct() functions.
  *
  * @note These functions do not perform any database activities.  However,
  *       an active database connection may be required for their use.  See
  *       modc_initialize(StringTP).
  *
  * @ingroup modco
  **/

/** @defgroup modco_access Accessor Functions
  * Functions to access values from the fields of an orbit object.
  * These functions operate on an existing detection object, and do not 
  * perform a database retrieval.
  * @sample_code OrbitTests.c modco_access
  * @ingroup modco
  **/

/** @defgroup modco_db Retrieval Functions
  * Functions to retrieve orbits from the persistant database.
  * 
  * All retrievals will return a MOPS-DC pointer to the resulting
  * objects.  For most retrievals, this pointer will reference an array
  * of objects.  The modco_getCount() function can be used to determine
  * how many objects were retrieved.  If no objects are retrieved, the
  * MOPS-DC pointer will be \c NULL.  See \ref modco_general and 
  * \ref modco_access for more details on using the MOPS-DC pointer.
  * @ingroup modco
  **/

/** @defgroup modco_dbin Storage Functions
  * Functions to store and update orbits in the persistant database.
  * @ingroup modco
  **/

	


/** Converts the specified object pointer to an orbit data structure.
  * This function assumes that the ::ModcOrbitTP pointer was previously 
  * retrieved from the database. If \a ptr was not retrieved from the 
  * database (i.e., it was created using modco_create()), this method will 
  * return \c NULL.
  *
  * @param ptr The MOPS-DC pointer to an orbit object.
  *
  * @return The equivalent orbit data structure (or \c NULL if \a ptr
  *         was not retrieved from the database).
  *
  * @note The caller of this function is responsible for freeing the space 
  *       allocated for the ::OrbitTP.  This may be done using \c free().
  * @note This function does not perform a retrieval from the database.
  *
  * @sample_code OrbitTests.c modco_convertToOrbit
  * @ingroup modco_general
  **/
OrbitTP modco_toStruct(ModcOrbitTP ptr);

/** Converts all of the orbit objects associated with the specified
  * object pointer into an array of orbit data structures.
  * This function assumes that the ::ModcOrbitTP pointer was previously 
  * retrieved from the database. If \a ptr was not retrieved from the 
  * database (i.e., it was created using modco_create()), this method will 
  * return \c NULL.
  *
  * The \a ptr will generally refer to a set of orbit objects.  
  * The returned array will be of size modco_getCount(ModcOrbitTP). 
  *
  * @param ptr The MOPS-DC pointer to a set of orbit objects.
  *
  * @return The equivalent array of orbit data structures (or \c NULL 
  *         if \a ptr was not retrieved from the database).
  *
  * @note The caller of this function is responsible for freeing the space 
  *       allocated for the ::OrbitTP.  This may be done using \c free().
  * @note This function does not perform a retrieval from the database.
  *
  * @see modco_getCount(ModcOrbitTP)
  *
  * @sample_code OrbitTests.c modco_convertToOrbitArray
  * @ingroup modco_general
  **/
OrbitArrayTP modco_toStructArray(ModcOrbitTP ptr);

/** Returns the number of orbit objects associated with the specified
  * object pointer.  This function assumes that the ::ModcOrbitTP pointer was 
  * previously retrieved from the database. If \a ptr was not retrieved from 
  * the database (i.e., it was created using modco_create()), this function 
  * will return <code>(int)</code>::Fail.
  *
  * @param ptr  The MOPS-DC pointer that originated from an orbit retrieval.
  *
  * @return The number of orbit associated with the ::ModcOrbitTP.
  *
  * @note This function does not perform a retrieval from the database.
  * @note If \a ptr was not retrieved from the database, this function will 
  *       return <code>(int)</code>::Fail.
  *
  * @see modco_toStructArray()
  *
  * @sample_code OrbitTests.c modco_convertToOrbitArray
  * @ingroup modco_general
  **/
int modco_getCount(ModcOrbitTP ptr);

/** Create a new orbit object.  This function will not store the object
  * in the database.
  *
  * Passing the ::DOUBLE_NIL value as any of the \c double parameters 
  * indicates that the parameter should remain uninitialized.  This is 
  * permitted for the following parameters:
  * @todo List the allowable ::DOUBLE_NIL parameters to create.
  *
  * @param q				The Perihelion distance (AU)
  * @param e				The Eccentricity (unitless; 0-1)
  * @param i				The Inclination (Degrees)
  * @param node				The Longitude of Ascending Node (Degrees)
  * @param longPeri			Longitude of the Perihelion (Degrees)
  * @param timePeri			Time of Perihelion Passage (MJD)
  * @param epoch			Epoch of Osculation (MJD)
  * @param chiSquared		chi^2 (arcsec^2) (RMS residual)
  * @param oMinusC			Observed - Calc(max) (arcsec) (MAX residual)
  * @param isFake			::True if the observation is simulated; ::False 
  *                         otherwise
  * @param orbitFlags		Orbit flags.  @todo Describe meaning of each orbit flag.
  * @param covMatrix		Covariance matrix (SUT 6X6 matrix)
  * @param moid				Minimum Orbit Intersection Distances (an
  *                         array of two \c double values)
  * @param moidLong			Earth longitude at moid[x] (degrees) (an array of
  *                         two \c double values)
  * @param objectName		Assigned Name
  * @param taxonomicType	Taxonomic type (eventually, an enumerated list)
  * @param rotationPeriod	Rotation period (seconds)
  * @param amplitude		Lightcurve Amplitue (magnitude)
  * @param rotationEpoch	Rotation Epoch (MJD)
  * @param hV				Absolute Magnitude in Visible band (mag)
  * @param hSS				Absolute Magnitude in Solar-System band (mag)
  * @param g				Slope Parameter for light curve (unitless)
  * @param albedo			Albedo (unitless; 0-1)
  * @param poleLat			Pole Orientation Latitude (degrees)
  * @param poleLong         Pole Orientation Longitude (degrees)
  *
  * @note This function does not perform any database operations.
  * @note If any \c double value is ::DOUBLE_NIL then the parameter will 
  *       remain uninitialized.  
  *       If any ::StringTP value is \c NULL then the parameter will remain
  *       uninitialized.
  *       If a required parameter is uninitialized, this function will return 
  *       \c NULL.
  *         
  * @see modco_insert(ModcOrbitTP),
  *      modco_attributeDetections(ModcOrbitTP, ModcDetArrayTP)
  *
  * @sample_code OrbitTests.c modco_create
  * @ingroup modco_general
  **/
ModcOrbitTP modco_create(	double q,
        					double e,
        					double i,
        					double node,
        					double longPeri,
        					MjdTP timePeri,
        					MjdTP epoch,
        					double chiSquared,
        					double oMinusC,
        					BoolTP isFake,
        					int orbitFlags,
      						CovMatrixTP covMatrix,
        					DoubleArrayTP moid,
        					DoubleArrayTP moidLong,
        					StringTP objectName,
        					TaxonomicTP taxonomicType,
        					double rotationPeriod,
        					double amplitude,
        					MjdTP rotationEpoch,
        					double hV,
        					double hSS,
        					double g,
        					double albedo,
        					double poleLat,
							double poleLong);

/** Retrieves a single orbit based on a unique identifier.  This will
  * return \c NULL if the ID cannot be found.
  *
  * @param orbId  The ID of the orbit.
  *
  * @return The MOPS-DC pointer to the retrieved object set, or \c NULL.
  *
  * @note This function performs a retrieval from the database.
  *
  * @see modco_toStruct(), modco_toStructArray(), modco_getCount()
  * 
  * @sample_code OrbitTests.c modco_retrieve
  * @ingroup modco_db
  **/
ModcOrbitTP modco_retrieve(IdTP orbId);

/** Retrieves all orbits based on specific values.  This will return \c NULL if
  * no matching orbits can be found.
  *
  * @param q              The perihelion distance (AU).
  * @param e              The eccentricity (unitless, 0-1).
  * @param i              The inclination (degrees).
  * @param node           The Longitude of Ascending Node (degrees).
  * @param longPeri       The Longitude of Perihelion (degrees).
  * @param timePeri       Time of Perihelion Passage (MJD).
  * @param epoch          The Epoch of Osculation (MJD).
  * @param deltaQ         The value that will be +/- from \a q to create a
  *                       lower and upper bound for the perihelion distance.
  *                       If \a q is ::DOUBLE_NIL then then \a deltaQ 
  *                       parameter will be ignored.
  * @param deltaE         The value that will be +/- from \a e to create a
  *                       lower and upper bound for the perihelion distance.
  *                       If \a e is ::DOUBLE_NIL then then \a deltaE 
  *                       parameter will be ignored.
  * @param deltaI         The value that will be +/- from \a i to create a
  *                       lower and upper bound for the perihelion distance.
  *                       If \a i is ::DOUBLE_NIL then then \a deltaI 
  *                       parameter will be ignored.
  * @param deltaNode      The value that will be +/- from \a node to create a
  *                       lower and upper bound for the perihelion distance.
  *                       If \a node is ::DOUBLE_NIL then then \a deltaNode 
  *                       parameter will be ignored.
  * @param deltaLongPeri  The value that will be +/- from \a longPeri to create a
  *                       lower and upper bound for the perihelion distance.
  *                       If \a longPeri is ::DOUBLE_NIL then then \a deltaLongPeri 
  *                       parameter will be ignored.
  * @param deltaTimePeri  The value that will be +/- from \a timePeri to create a
  *                       lower and upper bound for the perihelion distance.
  *                       If \a timePeri is ::DOUBLE_NIL then then \a deltaTimePeri 
  *                       parameter will be ignored.
  * @param deltaEpoch     The value that will be +/- from \a epoch to create a
  *                       lower and upper bound for the perihelion distance.
  *                       If \a epoch is ::DOUBLE_NIL then then \a deltaEpoch 
  *                       parameter will be ignored.
  * @param isFake        ::True if the detection is simulated, ::False otherwise.
  *
  * @todo Add a retrieval-by-value function that doesn't care about 
  * \a isFake value in DB.  This  can be handled for most fields by passing
  * ::DOUBLE_NIL but that won't work for a ::BoolTP
  * \e Alternative: Something like \c modcd_retrieveByPattern(OrbitPatternTP)
  * where \c OrbitPatternTP is a structure containing ranges for the various
  * fields and a flag indicating whether they should be used.
  *
  * @return The MOPS-DC pointer to the retrieved object set, or \c NULL.
  *
  * @note This function performs a retrieval from the database.
  *
  * @see modco_toStruct(), modco_toStructArray(), modco_getCount()
  * 
  * @ingroup modco_db
  **/
ModcOrbitTP modco_retrieveByValue(		double q,
								double e, 
								double i, 
								double node, 
								double longPeri, 
								double timePeri, 
								double epoch, 
								double deltaQ, 
								double deltaE, 
								double deltaI, 
								double deltaNode, 
								double deltaLongPeri, 
								double deltaTimePeri, 
								double deltaEpoch, 
								BoolTP isFake);



/** Retrieves all orbits to which a specific detection is attributed.  This 
  * will return \c NULL if the detection ID is cannot be attributed to any 
  * orbits.
  *
  * @param detId The unique ID of a detection.
  *
  * @return The MOPS-DC pointer to the retrieved object set, or \c NULL.
  *
  * @note This function performs a retrieval from the database.
  *
  * @see modco_toStruct(), modco_toStructArray(), modco_getCount()
  *
  * @ingroup modco_db
  **/
ModcOrbitTP modco_retrieveByDetectionId(	IdTP detId);

/** Creates and inserts an orbit object into the database. 
  *
  * Passing the ::DOUBLE_NIL value as any of the \c double parameters 
  * indicates that the parameter should remain uninitialized.  This is 
  * permitted for the following parameters:
  * @todo List the allowable ::DOUBLE_NIL parameters to create.
  *
  * @param q				The Perihelion distance (AU).
  * @param e				The Eccentricity (unitless; 0-1).
  * @param i				The Inclination (Degrees).
  * @param node				The Longitude of Ascending Node (Degrees).
  * @param longPeri			Longitude of the Perihelion (Degrees).
  * @param timePeri			Time of Perihelion Passage (MJD).
  * @param epoch			Epoch of Osculation (MJD).
  * @param chiSquared		chi^2 (arcsec^2) (RMS residual).
  * @param oMinusC			Observed - Calc(max) (arcsec) (MAX residual).
  * @param isFake			::True if the observation is simulated; ::False 
  *                         otherwise.
  * @param orbitFlags		Orbit flags.  @todo Describe meaning of each orbit flag.
  * @param covMatrix		Covariance matrix (SUT 6X6 matrix).
  * @param moid				Minimum Orbit Intersection Distances (an
  *                         array of two \c double values).
  * @param moidLong			Earth longitude at moid[x] (degrees) (an array of
  *                         two \c double values).
  * @param objectName		Assigned Name.
  * @param taxonomicType	Taxonomic type (eventually, an enumerated list).
  * @param rotationPeriod	Rotation period (seconds).
  * @param amplitude		Lightcurve Amplitue (magnitude).
  * @param rotationEpoch	Rotation Epoch (MJD).
  * @param hV				Absolute Magnitude in Visible band (mag).
  * @param hSS				Absolute Magnitude in Solar-System band (mag).
  * @param g				Slope Parameter for light curve (unitless).
  * @param albedo			Albedo (unitless; 0-1).
  * @param poleLat			Pole Orientation Latitude (degrees).
  * @param poleLong         Pole Orientation Longitude (degrees).
  *
  * @return The MOPS-DC pointer to the created orbit, or \c NULL if the insert
  *         failed.
  *
  * @note This function performs an insert into the database.
  * @note If any \c double value is ::DOUBLE_NIL then the parameter will 
  *       remain uninitialized.  
  *       If any ::StringTP value is \c NULL then the parameter will remain
  *       uninitialized.
  *       If a required parameter is uninitialized, this function will return 
  *       \c NULL.
  *         
  * @see modco_insert(ModcOrbitTP),
  *      modco_attributeDetections(ModcOrbitTP, ModcDetArrayTP)
  *
  * @ingroup modco_dbin
  **/
ModcOrbitTP modco_insertByValue(	double q,
        					double e,
        					double i,
        					double node,
        					double longPeri,
        					MjdTP timePeri,
        					MjdTP epoch,
        					double chiSquared,
        					double oMinusC,
        					BoolTP isFake,
        					FlagTP orbitFlags,
      						CovMatrixTP covMatrix,
        					DoubleArrayTP moid,
        					DoubleArrayTP moidLong,
        					StringTP objectName,
        					TaxonomicTP taxonomicType,
        					double rotationPeriod,
        					double amplitude,
        					MjdTP rotationEpoch,
        					double hV,
        					double hSS,
        					double g,
        					double albedo,
        					double poleLat,
							double poleLong);

/** Inserts a previously created orbit object into the database.  The \a orbit
  * should have been previously created with the modco_create() function.
  *
  * @param orbit  The MOPS-DC pointer to the orbit to insert.
  *
  * @return ::Success if the orbit was successfully inserted, ::Fail otherwise.
  *
  * @note This function performs an insert into the database.
  *         
  * @see modco_insertByValue(), 
  *      modco_attributeDetections(ModcOrbitTP, ModcDetArrayTP),
  *      modco_getUpdated()
  *
  * @ingroup modco_dbin
  **/
SucceedTP modco_insert(ModcOrbitTP orbit);


/** Attribute a set of detections to an orbit.   Use the 
  * ::modcd_attributeToOrbit(ModcDetTP, ModcOrbitTP) function to attribute a 
  * single detection.  The \a orbit and all detections in \a dets must already
  * exist in the database (i.e., the pointers are the result of previous
  * retrievals).
  * 
  * @param orbit   The MOPS-DC orbit object.
  * @param dets    The array of MOPS-DC detection objects to attribute.
  *
  * @return ::Success if the detections were successfully attributed, ::Fail 
  *         otherwise.
  *
  * @note This function performs an insert into the database.
  *
  * @see modcd_clearOrbit(), modcd_clearOrbitById(), 
  *      ::modcd_attributeToOrbit(), modco_clearDetections()
  * @ingroup modco_dbin
  **/
SucceedTP modco_attributeDetections(		ModcOrbitTP orbit,
								ModcDetArrayTP dets);

/** Removes a a set of attributed detections from an orbit.  Use the
  * detection-based ::modcd_clearOrbit() and ::modcd_clearOrbitById() to 
  * remove the attribution of a single detection.  The orbit and all detections
  * must already exist in the database, and all detections must already be 
  * attributed to the orbit.
  *
  * @param orb   The MOPS-DC orbit object.
  * @param dets  The array of MOPS-DC detection objects.
  *
  * @return ::Success if the the detections were successfully cleared from 
  *         the orbit, ::Fail otherwise.
  *
  * @note This function performs a delete from the database.
  *
  * @see modcd_clearOrbit(), modcd_clearOrbitById()
  * @ingroup modco_dbin
  **/
SucceedTP modco_clearDetections(	ModcOrbitTP orb,
									ModcDetArrayTP dets);

/** Updates the parameters of an orbit.  The orbit must already exist within the 
  * database.  Passing the ::DOUBLE_NIL value as any of the \c double parameters 
  * indicates that the parameter should remain unchanged.  Passing \c NULL for
  * a ::StringTP or array value indicates that the parameter should remain
  * unchanged.
  * @todo How do we indicate that we want to set a value to uninitialized (i.e.,
  * database NULL)?  Presumably this isn't possible with this API, but that
  * may be OK.
  *
  * @param ptr              The MOPS-DC orbit object to update.
  * @param q				The Perihelion distance (AU).
  * @param e				The Eccentricity (unitless; 0-1).
  * @param i				The Inclination (Degrees).
  * @param node				The Longitude of Ascending Node (Degrees).
  * @param longPeri			Longitude of the Perihelion (Degrees).
  * @param timePeri			Time of Perihelion Passage (MJD).
  * @param epoch			Epoch of Osculation (MJD).
  * @param chiSquared		chi^2 (arcsec^2) (RMS residual).
  * @param oMinusC			Observed - Calc(max) (arcsec) (MAX residual).
  * @param isFake			::True if the observation is simulated; ::False 
  *                         otherwise.
  * @param orbitFlags		Orbit flags.  @todo Describe meaning of each orbit flag.
  * @param covMatrix		Covariance matrix (SUT 6X6 matrix).
  * @param moid				Minimum Orbit Intersection Distances (an
  *                         array of two \c double values).
  * @param moidLong			Earth longitude at moid[x] (degrees) (an array of
  *                         two \c double values).
  * @param objectName		Assigned Name.
  * @param taxonomicType	Taxonomic type (eventually, an enumerated list).
  * @param rotationPeriod	Rotation period (seconds).
  * @param amplitude		Lightcurve Amplitue (magnitude).
  * @param rotationEpoch	Rotation Epoch (MJD).
  * @param hV				Absolute Magnitude in Visible band (mag).
  * @param hSS				Absolute Magnitude in Solar-System band (mag).
  * @param g				Slope Parameter for light curve (unitless).
  * @param albedo			Albedo (unitless; 0-1).
  * @param poleLat			Pole Orientation Latitude (degrees).
  * @param poleLong         Pole Orientation Longitude (degrees).
  *
  * @return ::Success if the updated was successful, ::Fail otherwise.
  *
  * @note This function performs an update to the database.
  * @note If any \c double value is ::DOUBLE_NIL then the parameter will 
  *       remain unchanged.  
  *       If any ::StringTP value is \c NULL then the parameter will remain
  *       unchanged.
  *       If any array value is \c NULL then the parameter will remain 
  *       unchanged.
  * @note If the orbit does not already exist in the database, this function
  *       will return ::Fail.
  *         
  * @see modco_create(), modco_getUpdated()
  *
  * @ingroup modco_dbin
  **/
SucceedTP modco_update(	ModcOrbitTP	ptr,
						double q,
        				double e,
        				double i,
        				double node,
        				double longPeri,
        				MjdTP timePeri,
        				MjdTP epoch,
        				double chiSquared,
        				double oMinusC,
        				BoolTP isFake,
        				FlagTP orbitFlags,
      					CovMatrixTP covMatrix,
        				DoubleArrayTP moid,
        				DoubleArrayTP moidLong,
        				StringTP objectName,
        				TaxonomicTP taxonomicType,
        				double rotationPeriod,
        				double amplitude,
        				MjdTP rotationEpoch,
        				double hV,
        				double hSS,
        				double g,
        				double albedo,
        				double poleLat,
						double poleLong);

/** Delete an orbit from the database.  The orbit must already exist
  * in the database.  Detections that are attributed to the orbit will
  * not be deleted from the database.
  *
  * @param orbitId  The unique ID of the orbit to delete.
  *
  * @return ::Success if the detection was successfully deleted, ::Fail 
  *         otherwise.
  *
  * @note This function performs a delete from the database.
  *
  * @sample_code OrbitTests.c modco_delete
  * @ingroup modco_dbin
  **/
SucceedTP modco_delete(IdTP orbitId);

/** Free all memory associated with the MOPS-DC object pointer.  This will
  * clear all memory associated with all ::ModcOrbitTP information obtained
  * from a retrieval or create operation.  All orbit objects associated with a
  * retrieval will be freed by a single call to this function.
  *
  * @param orbit  The MOPS-DC object pointer to free.
  *
  * @return ::Success if the orbit was successfully deleted, ::Fail 
  *         otherwise.
  *
  * @note This function does not perform any database operations.
  *
  * @sample_code OrbitTests.c modco_free
  * @ingroup modco_general
  **/
SucceedTP modco_free(ModcOrbitTP orbit);	

/** Free all memory associated with the MOPS-DC pointer array.  This will
  * clear memory associated with ::ModcOrbitArrayTP only.  This should be
  * called on the result of the ::modcd_getOrbits() function.
  *
  * @param orbits  The MOPS-DC object array pointer to free.
  *
  * @return ::Success if the array was successfully deleted, ::Fail 
  *         otherwise.
  *
  * @note This function does not perform any database operations.
  *
  * @ingroup modco_general
  **/
SucceedTP modco_freeArray(ModcOrbitArrayTP orbits);	

/** Free all memory associated with the orbit structure pointer.  This will
  * clear memory associated with the ::OrbitTP only.  This should be called on
  * the result of the ::modco_toStruct() function.
  *
  * @param orbit  The structure pointer to free.
  *
  * @return ::Success if the orbit was successfully deleted, ::Fail 
  *         otherwise.
  *
  * @note This function does not perform any database operations.
  *
  * @ingroup modco_general
  **/
SucceedTP modco_freeStruct(OrbitTP orbit);

/** Free all memory associated with the array of orbit structures.  This will
  * clear memory associated with the ::OrbitArrayTP only.  This should be called on
  * the result of the ::modco_toStructArray() function.
  *
  * @param orbits  The structure array to free.
  *
  * @return ::Success if the structure array was successfully deleted, ::Fail 
  *         otherwise.
  *
  * @note This function does not perform any database operations.
  *
  * @sample_code OrbitTests.c modco_freeArrayStruct
  * @ingroup modco_general
  **/
SucceedTP modco_freeArrayStruct(OrbitArrayTP orbits);	

/* ////////////////////////GETTERS////////////////////////////////////////// */

/** Return the unique ID of the orbit object.  This will be \c NULL if the
  * orbit was not retrieved from the database.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The unique ID of the orbit.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modco_access
  **/
IdTP modco_getId( ModcOrbitTP ptr);

/** Return the perihelion distance (in AU) of the orbit.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The perihelion distance of the orbit.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modco_access
  **/
double modco_getQ( ModcOrbitTP ptr);

/** Return the eccentricity of the orbit.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The eccentricity of the orbit.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modco_access
  **/
double modco_getE( ModcOrbitTP ptr);

/** Return the inclination (degrees) of the orbit.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The inclination of the orbit.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modco_access
  **/
double modco_getI( ModcOrbitTP ptr);

/** Return the longitude of ascending node (degrees) of the orbit.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The longitude of ascending node of the orbit.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modco_access
  **/
double modco_getNode( ModcOrbitTP ptr);

/** Return the longitude of perihelion (degrees) of the orbit.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The longitude of perihelion of the orbit.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modco_access
  **/
double modco_getLongPeri( ModcOrbitTP ptr);

/** Return the time of perihelion passage (MJD) of the orbit.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The time of perihelion passage of the orbit.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modco_access
  **/
double modco_getTimePeri( ModcOrbitTP ptr);

/** Return the epoch of osculation (MJD) of the orbit.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The epoch of osculation of the orbit.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modco_access
  **/
MjdTP modco_getEpoch( ModcOrbitTP ptr);

/** Return the number of detections attributed to this orbit.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The detection count for the orbit.
  *
  * @note This method does not perform a retrieval from the database.
  *
  * @see modco_getDetections()
  *
  * @ingroup modco_access
  **/
int modco_getDetCount( ModcOrbitTP ptr);

/** Return the Chi-Square of the orbit.  This is the RMS residual.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The Chi-Square of the orbit.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modco_access
  **/
double modco_getChiSquared( ModcOrbitTP ptr);

/** Return the maximum difference between observed and calculated position for 
  * the orbit.  This is essentially the maximum residual value (in arcsec).
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The maximum residual of the orbit (arcsec).
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modco_access
  **/
double modco_getOMinusC( ModcOrbitTP ptr);

/** Return a flag indicating that the orbit is simulated.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return ::True if the orbit is simulated; ::False otherwise.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modco_access
  **/
BoolTP modco_getIsFake( ModcOrbitTP ptr);

/** Return the orbit flags for the orbit.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The flags of the orbit.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modco_access
  **/
FlagTP modco_getFlags( ModcOrbitTP ptr);

/** Return the system time of the most recent update to this orbit.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The most recent update time (system time) of the orbit.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modco_access
  **/
time_t modco_getUpdated( ModcOrbitTP ptr);

/** Return the Minimum Orbit Intersection Distances (AU) of the orbit.  The 
  * returned array will always contain two values.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The two MOIDs of the orbit.
  *
  * @note This method does not perform a retrieval from the database.
  *
  * @see modco_getMoidLong()
  * @ingroup modco_access
  **/
DoubleArrayTP modco_getMoid( ModcOrbitTP ptr);

/** Return the Earth longitude of the MOIDs of the orbit (degrees).  The
  * returned array will always contain two values (one for each MOID).
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The xxx of the orbit.
  *
  * @note This method does not perform a retrieval from the database.
  *
  * @see modco_getMoid()
  * @ingroup modco_access
  **/
DoubleArrayTP modco_getMoidLong( ModcOrbitTP ptr);

/** Return the assigned name of the object in this orbit.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The name of the orbital object.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modco_access
  **/
StringTP modco_getObjectName( ModcOrbitTP ptr);

/** Return the taxonomic type of the object in this orbit.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The taxonomic type of the orbital object.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modco_access
  **/
TaxonomicTP modco_getTaxonomicType( ModcOrbitTP ptr);

/** Return the rotation period (seconds) of the object in this orbit.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The rotation period of the orbital object.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modco_access
  **/
double modco_getRotationPeriod( ModcOrbitTP ptr);

/** Return the lightcurve amplitude (a magnitude) of the object in this orbit.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The lightcurve amplitude of the orbital object.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modco_access
  **/
double modco_getAmplitude( ModcOrbitTP ptr);

/** Return the rotation epoch (MJD) of the object in this orbit.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The rotation epoch of the orbital object.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modco_access
  **/
MjdTP modco_getRotationEpoch( ModcOrbitTP ptr);

/** Return the visible-band absolute magnitude of the object in this orbit.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The visible-band absolute magnitude of the orbital object.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modco_access
  **/
double modco_getHV( ModcOrbitTP ptr);

/** Return the solar-system-band absolute magnitude of the object in this orbit.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The solar-system-band absolute magnitude of the orbital object.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modco_access
  **/
double modco_getHSS( ModcOrbitTP ptr);

/** Return the slope parameter for the light curve of the object in this orbit.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The slope parameter of the orbital object.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modco_access
  **/
double modco_getG( ModcOrbitTP ptr);

/** Return the albedo of the object in this orbit.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The albedo of the orbital object.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modco_access
  **/
double modco_getAlbedo( ModcOrbitTP ptr);

/** Return the latitude of the polar orientation (degrees) for the object 
  * in this orbit.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The polar orientation latitude of the orbital object.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modco_access
  **/
double modco_getPoleLat( ModcOrbitTP ptr);

/** Return the longitude of the polar orientation (degrees) for the object 
  * in this orbit.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The polar orientation longitude of the orbital object.
  *
  * @note This method does not perform a retrieval from the database.
  * @ingroup modco_access
  **/
double modco_getPoleLong ( ModcOrbitTP ptr);

/** Return the set of detections that have been attributed to this orbit.
  * 
  * @param ptr  The MOPS-DC object to access.
  *
  * @return The array of MOPS-DC detection objects.
  *
  * @note This method does not perform a retrieval from the database.
  *
  * @see modco_getDetCount()
  *
  * @ingroup modco_access
  **/
ModcDetArrayTP modco_getDetections( ModcOrbitTP ptr);

/** Return the next orbit object in the pool of available orbits
  * retrieved from the database.  If \a ptr is referencing the last 
  * orbit in the pool, this function will return \c NULL.  If \a ptr
  * was not created from a database retrieval, this function will always
  * return \c NULL.
  *
  * @param ptr  The MOPS-DC object pointer retrieved from the database.
  *
  * @return The next orbit in the retrieved set (or \c NULL).
  *
  * @note This function does not perform a retrieval from the database.
  *
  * @sample_code OrbitTests.c modco_next
  * @ingroup modco_general
  **/
ModcOrbitTP	modco_next(ModcOrbitTP ptr);

/** Return the previous orbit object in the pool of available orbits
  * retrieved from the database.  If \a ptr is referencing the first 
  * orbit in the pool, this function will return \c NULL.  If \a ptr
  * was not created from a database retrieval, this function will always
  * return \c NULL.
  *
  * @param ptr  The MOPS-DC object pointer retrieved from the database.
  *
  * @return The previous orbit in the retrieved set (or \c NULL).
  *
  * @note This function does not perform a retrieval from the database.
  *
  * @ingroup modco_general
  **/
ModcOrbitTP	modco_prev(ModcOrbitTP ptr);

/** Return the next detection attributed to the specified orbit.
  * If \a ptr is referencing the last detection in the pool, this will return
  * \c NULL.  If \a ptr was not created from a database retrieval, this 
  * function will always return \c NULL.
  *
  * @param ptr  The MOPS-DC orbit retrieved from the database.
  *
  * @return The next attributed detection (or \c NULL).
  *
  * @note This function does not perform a retrieval from the database.
  *
  * @see modco_getDetections()
  * @ingroup modco_access
  **/
ModcDetTP modco_nextDetection( ModcOrbitTP ptr);

/** Return the previous detection attributed to the specified orbit.
  * If \a ptr is referencing the first detection in the pool, this will return
  * \c NULL.  If \a ptr was not created from a database retrieval, this 
  * function will always return \c NULL.
  *
  * @param ptr  The MOPS-DC orbit retrieved from the database.
  *
  * @return The previous attributed detection (or \c NULL).
  *
  * @note This function does not perform a retrieval from the database.
  *
  * @see modco_getDetections()
  * @ingroup modco_access
  **/
ModcDetTP modco_prevDetection( ModcOrbitTP ptr);

/** @} */

#ifdef __cplusplus
} // end extern "C"
#endif

#endif
