%include "carrays.i"  // include SWIG array glue
%array_functions(double, doubleArray);

%module "PS::MOPS::DB::Observation"

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

ObservationTP modcm_toStruct(ModcObsTP ptr);
ObservationArrayTP modcm_toStructArray(ModcObsTP ptr);
int modcm_getCount(ModcObsTP ptr);
int modcm_getDetectionCount(ModcObsTP ptr);
ModcDetArrayTP modcm_getDetections( ModcObsTP ptr);
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
ModcObsTP  modcm_retrieve(	IdTP id);
ModcObsTP modcm_retrieveByValue(  	MjdTP epoch,
                                	double ra,
                                	double dec,
                                	double deltaEpoch,
                                	double deltaRa,
                                	double deltaDec);
SucceedTP modcm_insert(ModcObsTP obs);
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
SucceedTP modcm_addDetections(	ModcObsTP obs,
								ModcDetArrayTP dets);
SucceedTP modcm_delete(IdTP obsId);
SucceedTP modcm_free(ModcObsTP obs);	
SucceedTP modcm_freeArray(ModcObsArrayTP obs);	
SucceedTP modcm_freeStruct(ObservationTP obs);
SucceedTP modcm_freeArrayStruct(ObservationArrayTP obs);

IdTP modcm_getId(ModcObsTP ptr);
double modcm_getRa(ModcObsTP ptr);
double modcm_getDec(ModcObsTP ptr);
MjdTP modcm_getEpoch(ModcObsTP ptr);
SurveyTP modcm_getSurveyMode(ModcObsTP ptr);
MjdTP modcm_getTimeStop(ModcObsTP ptr);
MjdTP modcm_getTimeStart(ModcObsTP ptr);
FilterTP modcm_getFilter(ModcObsTP ptr);
double modcm_getLimitingMag(ModcObsTP ptr);
double modcm_getRaSigma(ModcObsTP ptr);
double modcm_getDecSigma (ModcObsTP ptr);
ObservatoryTP modcm_getObservatory (ModcObsTP ptr);
DoubleArrayTP modcm_getEfficiencies(ModcObsTP ptr);
ModcObsTP modcm_next(ModcObsTP ptr);
ModcObsTP modcm_prev(ModcObsTP ptr);
ModcDetTP modcm_nextDetection(ModcObsTP ptr);
ModcDetTP modcm_prevDetection(ModcObsTP ptr);
