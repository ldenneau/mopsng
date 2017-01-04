# This file was automatically generated by SWIG
package PS::MOPS::DB::Orbit;
require Exporter;
require DynaLoader;
@ISA = qw(Exporter DynaLoader);
package PS::MOPS::DB::Orbitc;
bootstrap PS::MOPS::DB::Orbit;
package PS::MOPS::DB::Orbit;

%EXPORT_TAGS = ( 'all' => [ qw(
    modco_toStruct
    modco_toStructArray
    modco_getCount
    modco_create
    modco_retrieve
    modco_retrieveByValue
    modco_retrieveByDetectionId
    modco_insertByValue
    modco_insert
    modco_attributeDetections
    modco_clearDetections
    modco_update
    modco_delete
    modco_free
    modco_freeArray
    modco_freeStruct
    modco_freeArrayStruct
    modco_getId
    modco_getQ
    modco_getE
    modco_getI
    modco_getNode
    modco_getLongPeri
    modco_getTimePeri
    modco_getEpoch
    modco_getDetCount
    modco_getChiSquared
    modco_getOMinusC
    modco_getIsFake
    modco_getFlags
    modco_getUpdated
    modco_getMoid
    modco_getMoidLong
    modco_getObjectName
    modco_getTaxonomicType
    modco_getRotationPeriod
    modco_getAmplitude
    modco_getRotationEpoch
    modco_getHV
    modco_getHSS
    modco_getG
    modco_getAlbedo
    modco_getPoleLat
    modco_getPoleLong
    modco_getDetections
    modco_next
    modco_prev
    modco_nextDetection
    modco_prevDetection
) ] );

@EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

@EXPORT = qw( );


# ---------- BASE METHODS -------------

package PS::MOPS::DB::Orbit;

sub TIEHASH {
    my ($classname,$obj) = @_;
    return bless $obj, $classname;
}

sub CLEAR { }

sub FIRSTKEY { }

sub NEXTKEY { }

sub FETCH {
    my ($self,$field) = @_;
    my $member_func = "swig_${field}_get";
    $self->$member_func();
}

sub STORE {
    my ($self,$field,$newval) = @_;
    my $member_func = "swig_${field}_set";
    $self->$member_func($newval);
}

sub this {
    my $ptr = shift;
    return tied(%$ptr);
}


# ------- FUNCTION WRAPPERS --------

package PS::MOPS::DB::Orbit;

*modco_toStruct = *PS::MOPS::DB::Orbitc::modco_toStruct;
*modco_toStructArray = *PS::MOPS::DB::Orbitc::modco_toStructArray;
*modco_getCount = *PS::MOPS::DB::Orbitc::modco_getCount;
#*modco_create = *PS::MOPS::DB::Orbitc::modco_create;
*modco_retrieve = *PS::MOPS::DB::Orbitc::modco_retrieve;
*modco_retrieveByValue = *PS::MOPS::DB::Orbitc::modco_retrieveByValue;
*modco_retrieveByDetectionId = *PS::MOPS::DB::Orbitc::modco_retrieveByDetectionId;
#*modco_insertByValue = *PS::MOPS::DB::Orbitc::modco_insertByValue;
*modco_insert = *PS::MOPS::DB::Orbitc::modco_insert;
*modco_attributeDetections = *PS::MOPS::DB::Orbitc::modco_attributeDetections;
*modco_clearDetections = *PS::MOPS::DB::Orbitc::modco_clearDetections;
*modco_update = *PS::MOPS::DB::Orbitc::modco_update;
*modco_delete = *PS::MOPS::DB::Orbitc::modco_delete;
*modco_free = *PS::MOPS::DB::Orbitc::modco_free;
*modco_freeArray = *PS::MOPS::DB::Orbitc::modco_freeArray;
*modco_freeStruct = *PS::MOPS::DB::Orbitc::modco_freeStruct;
*modco_freeArrayStruct = *PS::MOPS::DB::Orbitc::modco_freeArrayStruct;
*modco_getId = *PS::MOPS::DB::Orbitc::modco_getId;
*modco_getQ = *PS::MOPS::DB::Orbitc::modco_getQ;
*modco_getE = *PS::MOPS::DB::Orbitc::modco_getE;
*modco_getI = *PS::MOPS::DB::Orbitc::modco_getI;
*modco_getNode = *PS::MOPS::DB::Orbitc::modco_getNode;
*modco_getLongPeri = *PS::MOPS::DB::Orbitc::modco_getLongPeri;
*modco_getTimePeri = *PS::MOPS::DB::Orbitc::modco_getTimePeri;
*modco_getEpoch = *PS::MOPS::DB::Orbitc::modco_getEpoch;
*modco_getDetCount = *PS::MOPS::DB::Orbitc::modco_getDetCount;
*modco_getChiSquared = *PS::MOPS::DB::Orbitc::modco_getChiSquared;
*modco_getOMinusC = *PS::MOPS::DB::Orbitc::modco_getOMinusC;
*modco_getIsFake = *PS::MOPS::DB::Orbitc::modco_getIsFake;
*modco_getFlags = *PS::MOPS::DB::Orbitc::modco_getFlags;
*modco_getUpdated = *PS::MOPS::DB::Orbitc::modco_getUpdated;
*modco_getMoid = *PS::MOPS::DB::Orbitc::modco_getMoid;
*modco_getMoidLong = *PS::MOPS::DB::Orbitc::modco_getMoidLong;
*modco_getObjectName = *PS::MOPS::DB::Orbitc::modco_getObjectName;
*modco_getTaxonomicType = *PS::MOPS::DB::Orbitc::modco_getTaxonomicType;
*modco_getRotationPeriod = *PS::MOPS::DB::Orbitc::modco_getRotationPeriod;
*modco_getAmplitude = *PS::MOPS::DB::Orbitc::modco_getAmplitude;
*modco_getRotationEpoch = *PS::MOPS::DB::Orbitc::modco_getRotationEpoch;
*modco_getHV = *PS::MOPS::DB::Orbitc::modco_getHV;
*modco_getHSS = *PS::MOPS::DB::Orbitc::modco_getHSS;
*modco_getG = *PS::MOPS::DB::Orbitc::modco_getG;
*modco_getAlbedo = *PS::MOPS::DB::Orbitc::modco_getAlbedo;
*modco_getPoleLat = *PS::MOPS::DB::Orbitc::modco_getPoleLat;
*modco_getPoleLong = *PS::MOPS::DB::Orbitc::modco_getPoleLong;
*modco_getDetections = *PS::MOPS::DB::Orbitc::modco_getDetections;
*modco_next = *PS::MOPS::DB::Orbitc::modco_next;
*modco_prev = *PS::MOPS::DB::Orbitc::modco_prev;
*modco_nextDetection = *PS::MOPS::DB::Orbitc::modco_nextDetection;
*modco_prevDetection = *PS::MOPS::DB::Orbitc::modco_prevDetection;

sub modco_create {
    # Create an Observation object without inserting into PSMOPS;
    # return the opaque object as a scalar.

    # alloc ptr for doubles
    my @moid = @{$_[12]};
    my @moidLong_doubleArray = @{_[13]};
    my $d;

    # Last item in input array is ref to array of doubles.  Stuff em in ptr array.
    my $n = 0;
    foreach $d (@moid) {
        doubleArray_setitem($moid_doubleArray, $n, $d);
        $n++;
    }

    my $n = 0;
    foreach $d (@moidLong) {
        doubleArray_setitem($moidLong_doubleArray, $n, $d);
        $n++;
    }

    my $obj = PS::MOPS::DB::Orbit::modco_create(@_[0..11], $moid_doubleArray, $moidLong_doubleArray, @_[14..24]);
    delete_doubleArray($moid_doubleArray);
    delete_doubleArray($moidLong_doubleArray);
    return $obj;
}

sub modco_insertByValue {
    # Insert and create an Observation object;
    # return the opaque object as a scalar.

    # alloc ptr for doubles
    my @moid = @{$_[12]};
    my @moidLong_doubleArray = @{_[13]};
    my $d;

    # Last item in input array is ref to array of doubles.  Stuff em in ptr array.
    my $n = 0;
    foreach $d (@moid) {
        doubleArray_setitem($moid_doubleArray, $n, $d);
        $n++;
    }

    my $n = 0;
    foreach $d (@moidLong) {
        doubleArray_setitem($moidLong_doubleArray, $n, $d);
        $n++;
    }

    my $obj = PS::MOPS::DB::Orbit::modco_insertByValue(@_[0..11], $moid_doubleArray, $moidLong_doubleArray, @_[14..24]);
    delete_doubleArray($moid_doubleArray);
    delete_doubleArray($moidLong_doubleArray);
    return $obj;
}


# ------- VARIABLE STUBS --------

package PS::MOPS::DB::Orbit;

1;