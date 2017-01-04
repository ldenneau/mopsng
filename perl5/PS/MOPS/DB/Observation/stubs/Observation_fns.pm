sub modcm_create {
    # Create an Observation object without inserting into PSMOPS;
    # return the opaque object as a scalar.

    # alloc ptr for doubles
    my $DoubleArray = new_doubleArray(10);
    my $d;

    # Last item in input array is ref to array of doubles.  Stuff em in ptr array.
    my $n = 0;
    foreach $d (@{$_[-1]}) {
        doubleArray_setitem($DoubleArray, $n, $d);
        $n++;
    }
    my $obj = PS::MOPS::DB::Observationc::modcm_create(@_[0..10], $DoubleArray);
    delete_doubleArray($DoubleArray);
    return $obj;
}

sub modcm_insertByValue {
    # Create and insert a new Observation object; return the opaque object.

    # alloc ptr for doubles
    my $DoubleArray = new_doubleArray(10);
    my $d;

    # Last item in input array is ref to array of doubles.  Stuff em in ptr array.
    my $n = 0;
    foreach $d (@{$_[-1]}) {
        doubleArray_setitem($DoubleArray, $n, $d);
        $n++;
    }
    my $obj = PS::MOPS::DB::Observationc::modcm_insertByValue(@_[0..10], $DoubleArray);
    delete_doubleArray($DoubleArray);
    return $obj;
}
