# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 1.t'

#########################

use strict;
use warnings;
use Test::More tests => 2;
use Test::MockObject;
use PS::MOPS::MITI;

#########################

# Set up mock objects.
my $obs = Test::MockObject->new();
$obs->mock('obscode', sub { 'qqq' });
my $det = Test::MockObject->new();
$det->mock('objectName', sub { 'Jose' });
$det->mock('epoch', sub { 53373.416666 });
$det->mock('ra', sub { 100.0 });
$det->mock('dec', sub { 5.0 });
$det->mock('mag', sub { 20.1 });
$det->mock('detId', sub { '42' });
$det->mock('obscode', sub { 'qqq' });


sub samish {
    # Returns whether strings are same without regard to whitespace.
    # (whitespace is converted to a single space character).
    my ($i, $j) = @_;
    my @i = split /\s+/, $i;
    my @j = split /\s+/, $j;

    return 
        ($i[0] eq $j[0])        # ID
        and ($i[1] == $j[1])    # EPOCH_MJD
        and ($i[2] == $j[2])    # RA_DEG
        and ($i[3] == $j[3])    # DEC_DEG
        and ($i[4] == $j[4])    # MAG
        and ($i[5] eq $j[5])    # OBSCODE
        ;
}


sub test_miti_format_obsdet {
    my $same1;
    my $same2;
    my $line;

    $line = miti_format_obsdet($obs, $det);
    $same1 = samish($line, <<"EOF");
Jose    53373.416666    100.0   5.0 20.1    qqq
EOF
    
    # format with ID specified
    $line = miti_format_obsdet($obs, $det, '0001');
    $same2 = samish($line, <<"EOF");
0001    53373.416666    100.0   5.0 20.1    qqq Jose
EOF

    return $same1 and $same2;
}


sub test_miti_format_det {
    my $same1;
    my $same2;
    my $line;

    # format with ID specified
    $line = miti_format_det($det);
    $same1 = samish($line, <<"EOF");
42    53373.416666    100.0   5.0 20.1    qqq Jose
EOF

    # format with ID specified
    $line = miti_format_det($det, '0001');
    $same2 = samish($line, <<"EOF");
0001    53373.416666    100.0   5.0 20.1    qqq Jose
EOF

    return $same1 and $same2;
}

ok(test_miti_format_obsdet, 'miti_format_obsdet');
ok(test_miti_format_det, 'miti_format_det');
