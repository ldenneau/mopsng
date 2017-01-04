package PS::MOPS::DC::Shape;

use 5.008;
use strict;
use warnings;

use base qw(Exporter Class::Accessor);

our @EXPORT = qw(
    modcsh_insert
    modcsh_retrieve
);

our $VERSION = '0.01';


# Our stuff.
__PACKAGE__->mk_accessors(qw(
    shapeId
    ssmId
    g
    p
    period_d
    amp_mag
    a_km
    b_km
    c_km
    beta_deg
    lambda_deg
    ph_a
    ph_d
    ph_k
));

use Params::Validate;
use PS::MOPS::DC::Instance;
use PS::MOPS::DC::Iterator;

our $by_value_validate_args = {
    shapeId => 0,
    ssmId => 1,
    g => 1,
    p => 1,
    period_d => 1,
    amp_mag => 1,
    a_km => 1,
    b_km => 1,
    c_km => 1,
    beta_deg => 1,
    lambda_deg => 1,
    ph_a => 1,
    ph_d => 1,
    ph_k => 1,
};


sub new {
    # Create a new unattached Shape object, for later insertion into MOPS DC.
    # Indirect syntax is available: my $orb = PS::MOPS::DC::Shape->new(),
    # or modcsh_create()
    my $pkg = shift;
    my $inst = shift;
    my %self = validate(@_, $by_value_validate_args);
#    my %self = @_;     # whoa, validate() is SLOOWW
    $self{_inst} = $inst;
    $self{shapeId} = undef;   # always undef for new shapes until inserted
    return bless \%self;
}


sub modcsh_retrieve {
    # return shape object
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, { 
        shapeId => 0, 
        ssmId => 0,
        objectName => 0,
    });
    my $row;
    my $shape;
    my $where;
    my $val;
    my $sth;

    if ($args{objectName}) {
        $sth = $dbh->prepare(<<"SQL") or die "can't prepare SQL";
select 
    sh.shape_id,
    sh.ssm_id,
    sh.g, sh.p, sh.period_d, sh.amp_mag,
    sh.a_km, sh.b_km, sh.c_km, sh.beta_deg, sh.lambda_deg,
    sh.ph_a, sh.ph_d, sh.ph_k
from shapes sh, ssm m
where sh.ssm_id=m.ssm_id
and m.object_name=?
SQL
        $val = $args{objectName};
    }
    else {
        if ($args{shapeId}) {
            $where = 'where shape_id=?';
            $val = $args{shapeId};
        }
        elsif ($args{ssmId}) {
            $where = 'where ssm_id=?';
            $val = $args{ssmId};
        }
        else {
            die "neither shapeId or ssmId specified";
        }

        $sth = $dbh->prepare(<<"SQL") or die "can't prepare SQL";
select
    shape_id,
    ssm_id,
    g, p, period_d, amp_mag,
    a_km, b_km, c_km, beta_deg, lambda_deg,
    ph_a, ph_d, ph_k
from shapes
$where
SQL
    }

    $sth->execute($val) or die "can't execute SQL";
    my $aref = $sth->fetchrow_arrayref;
    if ($aref) {
        my %tmp;        # tmp hash
        @tmp{qw[
            shapeId ssmId g p period_d amp_mag a_km b_km c_km beta_deg lambda_deg ph_a ph_d ph_k
        ]} = @{$aref};  # assign array items to hash keys
        $shape = PS::MOPS::DC::Shape->new($inst, %tmp);
        $shape->{shapeId} = $aref->[0];
    }
    return $shape;
}
                                                                                                                                   
sub _insert {
    # Generic insert handler; accepts hashref from \%args or Shape object.
    # Returns inserted object, or None.
    my ($inst, $shape) = @_;
    my $dbh = $inst->dbh;
    my @args = @{$shape}{qw(
        ssmId g p period_d amp_mag a_km b_km c_km beta_deg lambda_deg ph_a ph_d ph_k
    )};
                                                                                                                                     
    my $rv = $dbh->do(<<"SQL", undef, @args);
insert into shapes (
ssm_id, g, p, period_d, amp_mag, a_km, b_km, c_km, beta_deg, lambda_deg, ph_a, ph_d, ph_k
)
values (
?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?
)
SQL
    if ($rv) {
        my $shape_id = $dbh->{'mysql_insertid'} if $rv;
        $shape->{shapeId} = $shape_id;
        $rv = $shape;
    }
    return $rv;
}


sub modcsh_insert {
    # insert previously created shape into DC.
    my $inst = shift;
    my $dbh = $inst->dbh;
    my ($shape) = validate_pos(@_, 1);
    return _insert($inst, $shape);
}


#sub modcsh_insertByValue {
## insert previously created shape into DC.
#my ($orb) = validate_pos(@_, 1);
#my $dbh = get_dbh();
#return _insert($dbh, $orb);
#}
                                                                                                                                     
                                                                                                                                     

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

PS::MOPS::DC::Shape - Module for manipulating MOPS Shape objects

=head1 SYNOPSIS

  use PS::MOPS::DC::Shape;

=head1 DESCRIPTION

Manipulate MOPS DC Shape objects.

=head2 EXPORT

None by default.

=head1 SEE ALSO

PS::MOPS::DC::Instance

=head1 AUTHOR

Larry Denneau <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright 2004 Institute for Astronomy, University of Hawaii

=cut
