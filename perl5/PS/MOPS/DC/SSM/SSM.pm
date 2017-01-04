package PS::MOPS::DC::SSM;

use 5.008;
use strict;
use warnings;
use Carp;

use base qw(Exporter Class::Accessor);

our @EXPORT = qw(
    modcs_insert
    modcs_insertByValue
    modcs_retrieve
    modcs_objectName2ssmId
    modcs_ssmId2objectName
);

our $VERSION = '0.01';


# Our stuff.
__PACKAGE__->mk_accessors(qw(
    ssmId
    q
    e
    i
    node
    argPeri
    timePeri
    epoch
    moid_1
    hV
    g
    objectName
    descId
));

use Params::Validate;
use PS::MOPS::DC::Instance;
use PS::MOPS::DC::Iterator;

our $by_value_validate_args = {
    q => 1,                 # semi-major axis q, AU
    e => 1,                 # eccentricity e, deg
    i => 1,                 # inclination i, deg
    node => 1,              # longitude of ascending node, deg
    argPeri => 1,           # argument of perihelion, deg
    timePeri => 1,          # time of perihelion, MJD
    epoch => 1,             # epoch, MJD
    hV => 1,                # magnitude
    g => 0,                 # slope parameter
    objectName => 1,        # MOPS synthetic object name
    moid_1 => 0,            # MOID1
    descId => 0,            # descriptive info
};


sub new {
    # Create a new unattached SSM orbit object, for later insertion into MOPS DC.
    # Indirect syntax is available: my $orb = PS::MOPS::DC::SSM->new(),
    # or modcm_create()
    my $pkg = shift;
    my $inst = shift;
    my %self = validate(@_, $by_value_validate_args);
    $self{_inst} = $inst;
    $self{ssmId} = undef;   # always undef for new orbits until inserted
    return bless \%self;
}


sub modcs_retrieve {
    # return orbit object
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, { ssmId => 0, objectName => 0, objectNames => 0, 
        all => 0, X => 0, C => 0, tiny => 0, minId => 0, maxId => 0, where => 0 });
    my $row;

    if ($args{all} or $args{X} or $args{C} or $args{minId} or $args{where}) {
        my $where_clause = ($args{where} or '');
        if (!$where_clause) {
            if ($args{minId}) {
                $where_clause = "where ssm_id >= $args{minId} and ssm_id <= $args{maxId}";
            }
        }

        my $sth = $dbh->prepare(<<"SQL") or croak "can't prepare SQL";
select
    ssm_id,
    q, e, i, node, arg_peri, time_peri, epoch, h_v, g,
    object_name, moid_1, desc_id
from ssm
$where_clause
order by ssm_id
SQL
        $sth->{mysql_use_result} = 1;
        $sth->execute or croak "can't execute SQL";
        return PS::MOPS::DC::Iterator->new(sub {
            my $aref = $sth->fetchrow_arrayref;
            my $orb;
            if ($aref) {
                $orb = PS::MOPS::DC::SSM->new($inst,
                    q => $aref->[1],
                    e => $aref->[2],
                    i => $aref->[3],
                    node => $aref->[4],
                    argPeri => $aref->[5],
                    timePeri => $aref->[6],
                    epoch => $aref->[7],
                    hV => $aref->[8],
                    g => $aref->[9],
                    objectName => $aref->[10],
                    moid_1 => $aref->[11],
                    descId => $aref->[12],
                );
                $orb->{ssmId} = $aref->[0];
                return $orb;
            }
            else {
                return undef;
            }
        });
    }
    elsif ($args{ssmId}) {
        $row = $dbh->selectrow_hashref(<<"SQL", undef, $args{ssmId});
select
    ssm_id,
    q, e, i, node, arg_peri, time_peri, epoch, h_v, g,
    object_name, moid_1, desc_id
from ssm
where ssm_id=?
SQL
    }
    elsif ($args{objectNames}) {
        my $sth = $dbh->prepare(<<"SQL") or croak "can't prepare sql";
select
    ssm_id,
    q, e, i, node, arg_peri, time_peri, epoch, h_v, g,
    object_name, moid_1, desc_id
from ssm
where object_name=?
SQL
        my @results;
        my $aref;
        my $orb;
        foreach my $objectName (@{$args{objectNames}}) {
            $sth->execute($objectName) or croak "can't execute sql";
            $aref = $sth->fetchrow_arrayref;
            if ($aref) {
                $orb = PS::MOPS::DC::SSM->new($inst,
                    q => $aref->[1],
                    e => $aref->[2],
                    i => $aref->[3],
                    node => $aref->[4],
                    argPeri => $aref->[5],
                    timePeri => $aref->[6],
                    epoch => $aref->[7],
                    hV => $aref->[8],
                    g => $aref->[9],
                    objectName => $aref->[10],
                    moid_1 => $aref->[11],
                    descId => $aref->[12],
                );
                $orb->{ssmId} = $aref->[0];
                push @results, $orb;
            }
            else {
                warn "couldn't fetch ssm object $objectName";
            }
        }
        return \@results; 
    }
    elsif ($args{objectName}) {
        $row = $dbh->selectrow_hashref(<<"SQL", undef, $args{objectName});
select
    ssm_id,
    q, e, i, node, arg_peri, time_peri, epoch, h_v, g,
    object_name, moid_1, desc_id
from ssm
where object_name=?
SQL
    }
    else {
        croak "neither ssmId or objectName specified";
    }
                                                                                                                                       
    my $orb;
    if ($row) {
        $orb = PS::MOPS::DC::SSM->new($inst,
            q => $row->{q},
            e => $row->{e},
            i => $row->{i},
            node => $row->{node},
            argPeri => $row->{arg_peri},
            timePeri => $row->{time_peri},
            epoch => $row->{epoch},
            hV => $row->{h_v},
            g => $row->{g},
            objectName => $row->{object_name},
            moid_1 => $row->{moid_1},
            descId => $row->{desc_id},
        );
        $orb->{ssmId} = $row->{ssm_id};
    }
    return $orb;
}
                                                                                                                                       
                                                                                                                                       
sub _insert {
    # Generic insert handler; accepts hashref from \%args or Observation object.
    my ($inst, $stuff) = @_;
    my $dbh = $inst->dbh;
    my @args = @{$stuff}{qw(
        q e i node argPeri timePeri epoch hV g objectName moid_1 descId
    )};
                                                                                                                                         
    my $rv = $dbh->do(<<"SQL", undef, @args);
insert into ssm (
q, e, i, node, arg_peri, time_peri, epoch, h_v, g, object_name, moid_1, desc_id
)
values (
?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?
)
SQL
    $rv = $dbh->{'mysql_insertid'} if $rv;
    return $rv;
}
                                                                                                                                         
                                                                                                                                         
sub modcs_insert {
    # insert previously created observation into DC.
    my $inst = shift;
    my $dbh = $inst->dbh;
    my ($orb) = validate_pos(@_, 1);
    return _insert($inst, $orb);
}


sub modcs_insertByValue {
    # insert previously created observation into DC.
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, $by_value_validate_args);
    return _insert($inst, \%args);
}
                                                                                                                                         
sub modcs_objectName2ssmId {
    # Return the SSM ID of the specified object.
    my $inst = shift;
    my $dbh = $inst->dbh;
    my ($objectName) = @_;
    my @row = $dbh->selectrow_array(<<"SQL", undef, $objectName);
select ssm_id from ssm where object_name=?
SQL
    return $row[0];
}
                                                                                                                                         
sub modcs_ssmId2objectName {
    # Return the SSM ID of the specified object.
    my $inst = shift;
    my $dbh = $inst->dbh;
    my ($objectName) = @_;
    my @row = $dbh->selectrow_array(<<"SQL", undef, $objectName);
select object_name from ssm where ssm_id=?
SQL
    return $row[0];
}


1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

PS::MOPS::DC::SSM - Module for manipulating MOPS SSM objects

=head1 SYNOPSIS

  use PS::MOPS::DC::SSM;

=head1 DESCRIPTION

Manipulate MOPS DC SSM objects.

=head2 EXPORT

None by default.

=head1 SEE ALSO

PS::MOPS::DC::Connection
PS::MOPS::DC::Observation

=head1 AUTHOR

Larry Denneau <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright 2004 Institute for Astronomy, University of Hawaii

=cut
