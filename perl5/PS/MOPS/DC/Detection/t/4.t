#########################

use Test::More tests => 1;
use PS::MOPS::Constants qw(:all);
use PS::MOPS::DC::Instance;
use PS::MOPS::DC::Detection;

#########################

my $inst = PS::MOPS::DC::Instance->new(DBNAME => 'psmops_unittest');

my %dummy_det = (
    detId => 42,
    fieldId => 69,
    ra => 30,
    dec => 45,
    mag => 22,
    refMag => 22.5,
    epoch => 53900,
    filter => 'DD',
    isSynthetic => 'Y',
    status => $DETECTION_STATUS_FOUND,
    s2n => 6.66,
    raSigma => 0,
    decSigma => 0,
    magSigma => 0,
    length_deg => 1,
    orient_deg => 5,
);

sub _compare_det_hashref {
    my ($det, $hashref) = @_;
    my $boo = ($det->s2n and $det->s2n == $hashref->{s2n});
    return  (
        ($det->detId == $hashref->{detId}) and
        ($det->fieldId == $hashref->{fieldId}) and
        ($det->ra == $hashref->{ra}) and
        ($det->dec == $hashref->{dec}) and
        ($det->mag == $hashref->{mag}) and
        ($det->refMag == $hashref->{refMag}) and
        ($det->epoch == $hashref->{epoch}) and
        ($det->filter eq $hashref->{filter}) and
        ($det->isSynthetic eq $hashref->{isSynthetic}) and
        ($det->status eq $hashref->{status}) and
        ($det->raSigma == $hashref->{raSigma}) and
        ($det->decSigma == $hashref->{decSigma}) and
        ($det->magSigma == $hashref->{magSigma}) and
        ($det->orient_deg eq $hashref->{orient_deg}) and
        ($det->length_deg eq $hashref->{length_deg}) and
        $boo
    );
}


sub serialize {
    my %fields = %dummy_det;
    my $det = PS::MOPS::DC::Detection->new($inst, %fields);
    my $str = $det->serialize();
    my $det2 = modcd_deserialize($inst, $str);
    my $v = _compare_det_hashref($det2, \%fields);
    return $v;
}

ok(serialize(), 'serialize');
