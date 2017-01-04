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

