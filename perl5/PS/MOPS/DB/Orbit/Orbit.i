%include "carrays.i"  // include SWIG array glue
%array_functions(double, doubleArray);

%module "PS::MOPS::DB::Orbit"

%{
#include "modc.h"
%}

// Need to map some typedefs to SWIG types.
typedef double MjdTP;
typedef double * DoubleArrayTP;
typedef char * SurveyTP;
typedef char * FilterTP;
typedef char * ObservatoryTP;
typedef long IdTP;
typedef int BoolTP;
typedef int FlagTP;


OrbitTP modco_toStruct(ModcOrbitTP ptr);
OrbitArrayTP modco_toStructArray(ModcOrbitTP ptr);
int modco_getCount(ModcOrbitTP ptr);
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
ModcOrbitTP modco_retrieve(IdTP orbId);
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
ModcOrbitTP modco_retrieveByDetectionId(	IdTP detId);
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
SucceedTP modco_insert(ModcOrbitTP orbit);

SucceedTP modco_attributeDetections(		ModcOrbitTP orbit,
								ModcDetArrayTP dets);
SucceedTP modco_clearDetections(	ModcOrbitTP orb,
									ModcDetArrayTP dets);
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
SucceedTP modco_delete(IdTP orbitId);
SucceedTP modco_free(ModcOrbitTP orbit);	
SucceedTP modco_freeArray(ModcOrbitArrayTP orbits);	
SucceedTP modco_freeStruct(OrbitTP orbit);
SucceedTP modco_freeArrayStruct(OrbitArrayTP orbits);	

IdTP modco_getId( ModcOrbitTP ptr);
double modco_getQ( ModcOrbitTP ptr);
double modco_getE( ModcOrbitTP ptr);
double modco_getI( ModcOrbitTP ptr);
double modco_getNode( ModcOrbitTP ptr);
double modco_getLongPeri( ModcOrbitTP ptr);
double modco_getTimePeri( ModcOrbitTP ptr);
MjdTP modco_getEpoch( ModcOrbitTP ptr);
int modco_getDetCount( ModcOrbitTP ptr);
double modco_getChiSquared( ModcOrbitTP ptr);
double modco_getOMinusC( ModcOrbitTP ptr);
BoolTP modco_getIsFake( ModcOrbitTP ptr);
FlagTP modco_getFlags( ModcOrbitTP ptr);
time_t modco_getUpdated( ModcOrbitTP ptr);
DoubleArrayTP modco_getMoid( ModcOrbitTP ptr);
DoubleArrayTP modco_getMoidLong( ModcOrbitTP ptr);
StringTP modco_getObjectName( ModcOrbitTP ptr);
TaxonomicTP modco_getTaxonomicType( ModcOrbitTP ptr);
double modco_getRotationPeriod( ModcOrbitTP ptr);
double modco_getAmplitude( ModcOrbitTP ptr);
MjdTP modco_getRotationEpoch( ModcOrbitTP ptr);
double modco_getHV( ModcOrbitTP ptr);
double modco_getHSS( ModcOrbitTP ptr);
double modco_getG( ModcOrbitTP ptr);
double modco_getAlbedo( ModcOrbitTP ptr);
double modco_getPoleLat( ModcOrbitTP ptr);
double modco_getPoleLong ( ModcOrbitTP ptr);
ModcDetArrayTP modco_getDetections( ModcOrbitTP ptr);
ModcOrbitTP	modco_next(ModcOrbitTP ptr);
ModcOrbitTP	modco_prev(ModcOrbitTP ptr);
ModcDetTP modco_nextDetection( ModcOrbitTP ptr);
ModcDetTP modco_prevDetection( ModcOrbitTP ptr);
