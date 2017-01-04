%module "PS::MOPS::DB::Utilities"
%{
#include "modc.h"   // typedefs, etc.
%}
%include MOPS.h     // show to SWIG

/* The current organization of SAIC code puts some useful version-related
functions in VersionTools.  We'll include them in this module. */
StringTP modcv_versionNumber();
int modcv_build();
int modcv_server();
StringTP modcv_date();
StringTP modcv_release();
StringTP modcv_full();
StringTP modcv_id();
StringTP modcv_description();
