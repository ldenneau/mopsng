/* ----------------------------------------------------------------------------
 *                           * * * UNCLASSIFIED * * *
 * ----------------------------------------------------------------------------
 *
 * $Logfile: /Software/libraries/mops-interface-c/MOPSStructs.h $
 *
 * ----------------------------------------------------------------------------
 *
 * $Workfile: MOPSStructs.h $
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
 * $History: MOPSStructs.h $
 * 
 * *****************  Version 20  *****************
 * User: Lisa A. Shannon Date: 9/20/04    Time: 2:09p
 * Updated in $/Software/libraries/mops-interface-c
 * Modified length and angle in DetSTRUCT to double.  That way if the
 * database gives us back a larger value than expected, we won't have
 * unexpected heap problems.
 * 
 * *****************  Version 19  *****************
 * User: Lisa A. Shannon Date: 9/15/04    Time: 4:32p
 * Updated in $/Software/libraries/mops-interface-c
 * Added middle tier back in because I forgot that struct is not the same
 * as typedef.
 * 
 * *****************  Version 17  *****************
 * User: Wayne L. Smith Date: 9/15/04    Time: 1:59p
 * Updated in $/Software/libraries/mops-interface-c
 * Shuffled documentation around to hide the middle-tier indirection for
 * data structures.
 * 
 * *****************  Version 16  *****************
 * User: Wayne L. Smith Date: 9/14/04    Time: 1:47p
 * Updated in $/Software/libraries/mops-interface-c
 * Modifed int, float, double arrays to use straight types, rather than
 * abstracted pointer types (i.e., removed IntTP, FloatTP, DoubleTP).
 * 
 * *****************  Version 15  *****************
 * User: Lisa A. Shannon Date: 9/10/04    Time: 4:08p
 * Updated in $/Software/libraries/mops-interface-c
 * modified #ifdef cplusplus to #Ifdef __cplusplus
 * __cplusplus is recognized by gcc and g++ where cplusplus (no
 * underscores) is only recognized by g++
 * 
 * *****************  Version 14  *****************
 * User: Lisa A. Shannon Date: 9/02/04    Time: 9:38a
 * Updated in $/Software/libraries/mops-interface-c
 * added MjdArraySTRUCT, MjdArrayStructTP and changed MjdArrayTP from a
 * double* to an MjdArrayStructTP*
 * 
 * *****************  Version 13  *****************
 * User: Lisa A. Shannon Date: 8/19/04    Time: 9:12a
 * Updated in $/Software/libraries/mops-interface-c
 * Integrated versions
 * Added ArrayTP structures that now include count
 * 
 * *****************  Version 12  *****************
 * User: Wayne L. Smith Date: 8/12/04    Time: 8:55p
 * Updated in $/Software/libraries/mops-interface-c
 * Created lots of data types to better indicate expectations.
 * 
 * *****************  Version 11  *****************
 * User: Lisa A. Shannon Date: 8/12/04    Time: 10:02a
 * Updated in $/Software/libraries/mops-interface-c
 * Removed SpatialCoord from structures
 * Uncommented CovMatrix to make available in structure
 * 
 * *****************  Version 10  *****************
 * User: Wayne L. Smith Date: 8/12/04    Time: 9:45a
 * Updated in $/Software/libraries/mops-interface-c
 * Shuffled structure definitions to comply with both C and C++ standards.
 * 
 * *****************  Version 9  *****************
 * User: Wayne L. Smith Date: 8/11/04    Time: 6:10p
 * Updated in $/Software/libraries/mops-interface-c
 * Fixed documentation to point to new Bool enum member names.
 * 
 * *****************  Version 8  *****************
 * User: Lisa A. Shannon Date: 8/11/04    Time: 5:52p
 * Updated in $/Software/libraries/mops-interface-c
 * Modified type for decSigma in ObservSTRUCT to a double
 * 
 * *****************  Version 7  *****************
 * User: Lisa A. Shannon Date: 8/11/04    Time: 2:17p
 * Updated in $/Software/libraries/mops-interface-c
 * Implemented IdTP, FIlterTP, TaxonomicTP
 * Added PairArrayTP, DetectionArrayTP, ObservatoinArrayTP, OrbitArrayTP
 * 
 * *****************  Version 6  *****************
 * User: Wayne L. Smith Date: 8/10/04    Time: 7:15p
 * Updated in $/Software/libraries/mops-interface-c
 * Cleaned up documentation using groups.  Refactored all functions to use
 * a prefix specific to MOPS DC libraries.
 * 
 * *****************  Version 5  *****************
 * User: Wayne L. Smith Date: 8/10/04    Time: 5:48p
 * Updated in $/Software/libraries/mops-interface-c
 * Cleaned up doxygen documentation.  Adjusted the structures slightly.
 * 
 */


/** @file
  *
  * Data structure definitions.
  *
  * @version Revision<!--$$Revision: 14 $-->on<!--$$Date: 2004-12-09 09:55:35 -1000 (Thu, 09 Dec 2004) $-->
  * @author Lisa A. Shannon,
  *         Last modified by: <!--$$Author: denneau $-->
  **/

#ifndef __MOPSSTRUCTS_H
#define __MOPSSTRUCTS_H

#ifdef __cplusplus 
extern "C" {
#endif

#include "MOPS.h"
#include "time.h"

#ifndef __DOXYGEN_SHOULD_SKIP_THIS__
#endif /* __DOXYGEN_SHOULD_SKIP_THIS__ */

/** @name Structure Types
  * Types used to represent the data structures for MOPS objects and arrays.
  * @{
  **/

/** Represents the structure for an orbit.
  **/
typedef struct OrbitSTRUCT OrbitStructTP;

/** Represents the structure for an observation (metadata).
  **/
typedef struct ObservSTRUCT ObservStructTP;

/** Represents the structure for a detection.
  **/
typedef struct DetSTRUCT DetStructTP;

/** Represents the structure for a detection pair.
  **/
typedef struct PairSTRUCT PairStructTP;

/** Represents the structure for an array of orbit structures (i.e., ::OrbitTP).
  **/
typedef struct OrbitArraySTRUCT OrbitArrayStructTP;

/** Represents the structure for an array of observation (metadata) structures 
  * (i.e., ::ObservationTP).
  **/
typedef struct ObservArraySTRUCT ObservArrayStructTP;

/** Represents the structure for an array of detection structures (i.e., 
  * ::DetectionTP).
  **/
typedef struct DetArraySTRUCT DetArrayStructTP;

/** Represents the structure for an array of detection-pair structures 
  * (i.e., ::PairTP).
  **/
typedef struct PairArraySTRUCT PairArrayStructTP;

/** Represents the structure for an array of MJD values (i.e., ::MjdTP).
  **/
typedef struct MjdArraySTRUCT MjdArrayStructTP;

/** Represents the structure for an array of \c float values.
  **/
typedef struct FloatArraySTRUCT FloatArrayStructTP;

/** Represents the structure for an array of \c double values.
  **/
typedef struct DoubleArraySTRUCT DoubleArrayStructTP;

/** Represents the structure for an array of \c int values.
  **/
typedef struct IntArraySTRUCT IntArrayStructTP;

/** Represents the structure for an array of MOPS-DC Detection Pointers 
  * (i.e., ::ModcDetTP).
  **/
typedef struct ModcDetArraySTRUCT ModcDetArrayStructTP;

/** Represents the structure for an array of MOPS-DC Observation Pointers 
  * (i.e., ::ModcObsTP).
  **/
typedef struct ModcObsArraySTRUCT ModcObsArrayStructTP;

/** Represents the structure for an array of MOPS-DC Detection-Pair structures 
  * (i.e., ::ModcPairTP).
  **/
typedef struct ModcPairArraySTRUCT ModcPairArrayStructTP;

/** Represents the structure for an array of MOPS-DC Orbit structures 
  * (i.e., ::ModcOrbitTP).
  **/
typedef struct ModcOrbitArraySTRUCT ModcOrbitArrayStructTP;

/** @} */

/** @name Array Data Types
  * Types used to represent arrays.  Each of these is a structure that includes
  * a field for the pointer to the appropriate values and a count of the number
  * of elements.
  * @{
  **/

/** Used to indicate an array of ::OrbitTP values.
  * @ingroup types
  **/
typedef OrbitArrayStructTP* OrbitArrayTP;

/** Used to indicate an array of ::ObservationTP values.
  * @ingroup types
  **/
typedef ObservArrayStructTP* ObservationArrayTP;

/** Used to indicate an array of ::DetectionTP values.
  * @ingroup types
  **/
typedef DetArrayStructTP* DetectionArrayTP;

/** Used to indicate an array of ::PairTP values.
  * @ingroup types
  **/
typedef PairArrayStructTP* PairArrayTP;

/** Used to indicate an array of \c float values.
  * @ingroup types
  **/ 
typedef FloatArrayStructTP* 	FloatArrayTP;

/** Used to indicate an array of \c double values.
  * @ingroup types
  **/ 
typedef DoubleArrayStructTP* 	DoubleArrayTP;

/** Used to indicate an array of ::MjdTP values.
  * @ingroup types
  **/ 
typedef MjdArrayStructTP* 	MjdArrayTP;

/** Used to indicate an array of \c int values.
  * @ingroup types
  **/ 
typedef IntArrayStructTP* 	IntArrayTP;

/** Used to indicate an array of ::ModcDetTP values.
  * @ingroup types
  **/ 
typedef ModcDetArrayStructTP* 	ModcDetArrayTP;

/** Used to indicate an array of ::ModcObsTP values.
  * @ingroup types
  **/ 
typedef ModcObsArrayStructTP* 	ModcObsArrayTP;

/** Used to indicate an array of ::ModcOrbitTP values.
  * @ingroup types
  **/ 
typedef ModcOrbitArrayStructTP* 	ModcOrbitArrayTP;

/** Used to indicate an array of ::ModcPairTP values.
  * @ingroup types
  **/ 
typedef ModcPairArrayStructTP* 	ModcPairArrayTP;

/** @} */


/** @name Data Structure Pointer Types
  * Pointer types for the MOPS object data structure types.
  * @{
  **/

/** Pointer type for an orbit structure.
  * @ingroup types
  **/
typedef OrbitStructTP* OrbitTP;

/** Pointer type for an observation (metadata) structure.
  * @ingroup types
  **/
typedef ObservStructTP* ObservationTP;

/** Pointer type for a detection structure.
  * @ingroup types
  **/
typedef DetStructTP* DetectionTP;

/** Pointer type for a detection-pair structure.
  * @ingroup types
  **/
typedef PairStructTP* PairTP;


/** @} */


/** @struct time_t
  * Time structure from standard "time.h" header.
  * @todo Insert documentation for \c time_t structure from "time.h".
  **/

/** C-language structure that represents an observation (i.e., metadata).
  * Note that this <em>does not</em> contain an array of all detections
  * that were seen with this observation.
  **/
struct ObservSTRUCT {
		IdTP observationId;			/**< Unique identifier. */
		MjdTP epoch;				/**< The epoch (MJD, full precision). */
		double ra;					/**< The Right Ascension (degrees). */
		double dec;					/**< The Declination (degrees). */
		SurveyTP surveyMode;		/**< The survey mode. */
		MjdTP timeStart;			/**< Start time of observation (MJD) */
		MjdTP timeStop;				/**< End time of observation (MJD) */
		FilterTP filter;			/**< Filter identifier. */
		double limitingMag;			/**< Limiting magnitude of this observation (i.e., a quality indicator) (magnitude). */
		double raSigma;				/**< Error in Right Ascension (arcsec). */
		double decSigma;			/**< Error in Declination (arcsec). */
		ObservatoryTP observatory;	/**< The IAU/MPC Observatory Code. */
		DoubleArrayTP de;			/**< Detection Efficiency Parameters.  This is an array of ten \c double values. */
};

/** C-language structure that represents an array of observation structures.
  * 
  * @see ::ObservationTP
  **/
struct ObservArraySTRUCT {
	ObservationTP* observations; /**< The observation values. */
	int count;                   /**< The number of values in this array. */
};


/** C-language structure that represents a detection.  This will contain an 
  * array of all orbits to which this detection has been attributed.
  *
  * @see ::OrbitSTRUCT
  **/
struct DetSTRUCT {
		IdTP	detId;				/**< Unique identifier. */
		double ra;					/**< The Right Ascension (degrees). */
		double dec;					/**< The Declination (degrees). */
		MjdTP epoch;				/**< Epoch of detection (MJD) */
		double mag;					/**< Magnitude of detection (magnitude)*/
		FilterTP filter;			/**< Filter identifier. */	
		BoolTP isFake;				/**< ::True if detection is simulated; otherwise ::False. */
		FlagTP flags;				/**< Detection flags.  @todo Describe the meaning of each flag. */	
		double raSigma;				/**< Error in Right Ascension (arcsec). */	
		double decSigma;			/**< Error in Declination (arcsec). */		
		double magSigma;			/**< Error in Magnitude (magnitude). */
		double length;				/**< Length of detection (arcsec). @see DetSTRUCT::angle */	
		double angle;				/**< Angle of detection (degrees, clockwise from North).  @see DetSTRUCT::length */
		ObservationTP observation;	/**< The observation object (i.e., metadata) associated with this detection. */
		OrbitArrayTP orbits;		/**< The orbits to which this detection is attributed.  This is an array of DetSTRUCT::orbitCount orbit pointers. */
		int orbitCount;				/**< The number of orbits to which this detection is attributed.  This is the size of the DetSTRUCT::orbits array. */
};

/** C-language structure that represents an array of detection structures.
  * 
  * @see ::DetectionTP
  **/
struct DetArraySTRUCT {
	DetectionTP* detections; /**< The detection values. */
	int count;               /**< The number of values in this array. */
};

/** C-language structure that represents an orbit.  This will contain an array 
  * of all detections that have been attributed to the orbit.
  *
  * @see ::DetSTRUCT
  **/
struct OrbitSTRUCT {		
		IdTP id;				/**< Unique identifier. */
		double q;				/**< The Perihelion distance (AU).*/
		double e;				/**< The Eccentricity (unitless; 0-1).*/
		double i;				/**< The Inclination (degrees).*/
		double node;			/**< The Longitude of Ascending Node (degrees). */	
		double longPeri;		/**< Longitude of the Perihelion (degrees). */
		MjdTP timePeri;			/**< Time of Perihelion Passage (MJD). */
		MjdTP epoch;			/**< Epoch of Osculation (MJD). */
		int detCount;			/**< Detection Count. */
		double chiSquared;		/**< chi^2 (arcsec^2) (RMS residual). */
		double oMinusC;			/**< Observed - Calc(max) (arcsec) (MAX residual). */
		BoolTP isFake;			/**< ::True if the orbit is simulated; ::False otherwise. */
		FlagTP flags;			/**< Orbit flags.  @todo Describe the meaning of each flag. */	
		time_t updated;			/**< Update timestamp for this orbit. */
		CovMatrixTP covMatrix;	/**< Covariance matrix (SUT 6X6 matrix). */
		DoubleArrayTP moid;		/**< Minimum Orbit Intersection Distances.  This is an array of two \c double values. */
		DoubleArrayTP moidLong;	/**< Earth longitude at moid[x] (degrees).  This is an array of two \c double values. */
		StringTP objectName;	/**< Assigned Name. */
		TaxonomicTP taxonomicType;	/**< Taxonomic type (eventually, an enumerated list). */
		double rotationPeriod;	/**< Rotation period (seconds). */
		double amplitude;		/**< Lightcurve Amplitude (magnitude). */
		MjdTP rotationEpoch;	/**< Rotation Epoch (MJD). */
		double hV;				/**< Absolute Magnitude in Visible band (mag). */
		double hSS;				/**< Absolute Magnitude in Solar-System band (mag). */
		double g;				/**< Slope Parameter for light curve (unitless). */
		double albedo;			/**< Albedo (unitless; 0-1). */
		double poleLat;			/**< Pole Orientation Latitude (degrees). */
		double poleLong;		/**< Pole Orientation Longitude (degrees). */
		DetectionArrayTP detections;	/**< The detections attributed to this Orbit. This is an array of OrbitSTRUCT::detCount detection pointers. */
}; 

/** C-language structure that represents an array of orbit structures. 
  *
  * @see ::OrbitTP
  **/
struct OrbitArraySTRUCT {
	OrbitTP* orbits; /**< The orbit values. */
	int count;       /**< The number of values in this array. */
};

/** C-language structure that represents a pair of detections.  The first 
  * detection should always be the earliest one of the two.
  *
  * @see ::DetectionTP
  **/
struct PairSTRUCT {
		IdTP	pairId;			/**< Unique identifier. */
		DetectionArrayTP dets;	/**< The two detections in this pair. This is a pointer to an array of DetectionTP. */
		DoubleArrayTP ra;		/**< The Right Ascension values of the two detections.  This is a pointer to an array of Right Ascensions. */
		DoubleArrayTP dec;		/**< The Declination values of the two detections.  This is a pointer to an array of Declinations. */
		MjdArrayTP epoch;		/**< The epochs of the two detections.  This is a pointer to an arry of epochs. */
		DoubleArrayTP mag;		/**< The magnitudes of the two detections.  This is a pointer to an array of magnitudes. */
		double deltaRa;			/**< The Right Ascension component of the position difference between the two detections (arcsec). */
		double deltaDec;		/**< The Declination component of the position difference between the two detections (arcsec). */
		double vRa;				/**< The Right Ascension component of the velocity (arcsec/day). */
		double vDec;			/**< The Declination component of the velocity (arcsec/day). */
};

/** C-language structure that represents an array of a detection-pair structures.
  *
  * @see ::PairTP
  **/
struct PairArraySTRUCT {
	PairTP* pairs; /**< The pair values. */
	int count;     /**< The number of values in this array. */
};


/** C-language structure that represents an array of a ::ModcDetTP values.
  *
  * @see ::ModcDetTP
  **/
struct ModcDetArraySTRUCT {
	ModcDetTP* detections; /**< The detection pointer values. */
	int count;             /**< The number of values in this array. */
};

/** C-language structure that represents an array of ::ModcOrbitTP values.
  *
  * @see ::ModcOrbitTP
  **/
struct ModcOrbitArraySTRUCT {
	ModcOrbitTP* orbits; /**< The orbit pointer values. */
	int count;           /**< The number of values in this array. */
};

/** C-language structure that represents an array of a ::ModcObsTP values.
  *
  * @see ::ModcObsTP
  **/
struct ModcObsArraySTRUCT { 
	ModcObsTP* observations;  /**< The observation pointer values. */
	int count;                /**< The number of values in this array. */
};

/** C-language structure that represents an array of a ::ModcPairTP values.
  *
  * @see ::ModcPairTP
  **/
struct ModcPairArraySTRUCT {
	ModcPairTP* pairs; /**< The pair pointer values. */
	int count;         /**< The number of values in this array. */
};

/** C-language structure that represents an array of \c double values.
  **/
struct DoubleArraySTRUCT {
	double* values; /**< The \c double values. */
	int count;      /**< The number of values in this array. */
};

/** C-language structure that represents an array of ::MjdTP values.
  **/
struct MjdArraySTRUCT {
	MjdTP* values; /**< The MJD values. */
	int count;     /**< The number of values in this array. */
};

/** C-language structure that represents an array of \c int values.
  **/
struct IntArraySTRUCT {
	int* values; /**< The \c int values. */
	int count;   /**< The number of values in this array. */
};

/** C-language structure that represents an array of \c float values.
  **/
struct FloatArraySTRUCT {
	float* values; /**< The \c float values. */
	int count;     /**< The number of values in this array. */
};


#ifdef __cplusplus 
}
#endif

#endif

