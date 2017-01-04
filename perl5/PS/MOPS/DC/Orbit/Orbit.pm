package PS::MOPS::DC::Orbit;

use 5.008;
use strict;
use warnings;
use Carp;

use base qw(Exporter Class::Accessor);

our @EXPORT = qw(
    modco_insert
    modco_insertByValue
    modco_retrieve
    modco_deserialize
);
our $VERSION = '0.01';

use Params::Validate;
use PS::MOPS::Constants qw(:all);
use PS::MOPS::Lib;
use PS::MOPS::DC::Instance;
use PS::MOPS::DC::Iterator;
use PS::MOPS::DC::Tracklet;

# Our stuff.
our @covariance_cols = qw(
    cov_01
    cov_02 cov_03
    cov_04 cov_05 cov_06
    cov_07 cov_08 cov_09 cov_10
    cov_11 cov_12 cov_13 cov_14 cov_15
    cov_16 cov_17 cov_18 cov_19 cov_20 cov_21
);
our $COVARIANCE_LENGTH = scalar @covariance_cols;   # length


our $selectall_str = sprintf <<"SQL", join(', ', @covariance_cols);
select
    orbit_id, q, e, i, node, arg_peri, time_peri, epoch, h_v, residual, chi_squared, moid_1, arc_length_days, conv_code,
    %s
SQL

our $selectderivedobjects_str = sprintf <<"SQL", join(', ', @covariance_cols);
select
    o.orbit_id orbit_id, 
    q, e, i, node, arg_peri, time_peri, epoch, h_v, residual, chi_squared, moid_1, arc_length_days,
    do.object_name object_name, conv_code,
    %s
from orbits o, derivedobjects do
where o.orbit_id=do.orbit_id
SQL

# SQL to select all existing unmerged (parent) derived objects that are not
# currently in the EON queue or are in the queue but not waiting for ORBITID
# (status != NEW).
our $selectorbitid_data_str = sprintf <<"SQL", join(', ', @covariance_cols);
select
    o.orbit_id orbit_id, 
    q, e, i, node, arg_peri, time_peri, epoch, h_v, residual, chi_squared, moid_1, arc_length_days,
    do.object_name object_name, conv_code,
    %s
from orbits o 
join derivedobjects do using (orbit_id)
left join eon_queue eonq using (derivedobject_id)
where eonq.derivedobject_id is null
or eonq.status <> '$EONQUEUE_STATUS_NEW'
and do.status <> '$DERIVEDOBJECT_STATUS_MERGED'
SQL

# SQL to select all existing unmerged (parent) derived objects that are 
# currently in the EON queue awaiting ORBITID (status = NEW).
our $selectorbitid_query_str = sprintf <<"SQL", join(', ', @covariance_cols);
select
    o.orbit_id orbit_id, 
    q, e, i, node, arg_peri, time_peri, epoch, h_v, residual, chi_squared, moid_1, arc_length_days,
    do.object_name object_name, conv_code,
    %s
from orbits o 
join derivedobjects do using (orbit_id)
join eon_queue eonq using (derivedobject_id)
where eonq.status = '$EONQUEUE_STATUS_NEW'
and do.status <> '$DERIVEDOBJECT_STATUS_MERGED'
SQL

# SQL to select all surviving objects in EON queue so that
# we can compute other stuff (MOID, etc).
our $select_moidqueue_str = sprintf <<"SQL", join(', ', @covariance_cols);
select
    o.orbit_id orbit_id, 
    q, e, i, node, arg_peri, time_peri, epoch, h_v, residual, chi_squared, moid_1, arc_length_days,
    o.orbit_id object_name, conv_code,
    %s
from orbits o 
join moid_queue mq using (orbit_id)
SQL


__PACKAGE__->mk_accessors(qw(
    orbitId
    q
    e
    i
    node
    argPeri
    timePeri
    epoch
    hV
    residual
    chiSquared
    covariance
    moid_1
    arcLength_days
    objectName
    convCode
));

our $by_value_validate_args = {
    orbitId => 0,
    q => 1,
    e => 1,
    i => 1,
    node => 1,
    argPeri => 1,
    timePeri => 1,
    epoch => 1,
    hV => 1,
    residual => 0,
    chiSquared => 0,
    covariance => 0,
    moid_1 => 0,
    arcLength_days => 0,
    objectName => 0,
    convCode => 0,
};


sub new {
    # Create a new unattached orbit object, for later insertion into MOPS DC.
    # Indirect syntax is available: my $orb = PS::MOPS::DC::Orbit->new(),
    # or modcm_create()
    my $pkg = shift;
    my $inst = shift;
    my %self = validate(@_, $by_value_validate_args);
    $self{_inst} = $inst;
#    $self{orbitId} = undef;                     # always undef for new orbits until inserted

    # Check covariance matrix.
    if ($self{covariance} && scalar @{$self{covariance}} != $COVARIANCE_LENGTH) {
        croak sprintf "covariance matrix not right length: %d", scalar @{$self{covariance}};
    }

    return bless \%self;
}


sub new_from_iodtxt {
    # Create a new unattached orbit object from IOD text output.  No
    # validation is done on the input string.
    my ($pkg, $inst, $line) = @_;
    my @stuff = split /\s+/, $line;
    croak "wrong number of arguments: $line" unless @stuff >= 10;
    my %self = (
        _inst => $inst,
        q => $stuff[0],
        e => $stuff[1],
        i => $stuff[2],
        node => $stuff[3],
        argPeri => $stuff[4],
        timePeri => $stuff[5],
        hV => $stuff[6],
        epoch => $stuff[7],
        residual => $stuff[9],
    );
    $self{orbitId} = undef;                     # always undef for new orbits until inserted

    return bless \%self;
}


sub _new_from_row {
    my ($inst, $row) = @_;
    return undef unless $row;   # sanity check
    my $orb = PS::MOPS::DC::Orbit->new($inst,
        q => $row->{q},
        e => $row->{e},
        i => $row->{i},
        node => $row->{node},
        argPeri => $row->{arg_peri},
        timePeri => $row->{time_peri},
        epoch => $row->{epoch},
        hV => $row->{h_v},
        residual => $row->{residual},
        chiSquared => $row->{chi_squared},
        moid_1 => $row->{moid_1},
        arcLength_days => $row->{arc_length_days},
        objectName => $row->{object_name},
        convCode => $row->{conv_code},
    );

    # Build covariance array.
    if (defined($row->{cov_01})) {       # looks like we have covariance
        $orb->{covariance} = [
            @{$row}{@covariance_cols},
        ];
    }

    $orb->{orbitId} = $row->{orbit_id};

    return bless $orb;      # make object
}


sub modco_retrieve {
    # return orbit objects
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, { 
        orbitId => 0,           # by orbitId
        derived => 0,           # all derived
        derivedobjectName => 0, # single derived object 
        derivedobjectNames => 0,# multiple derived objects
        orderBy => 0,           # sort by criterion for multiple result set
        updated_since => 0,     # only derived objects updated since specified date
        orbitid_data => 0,      # retrieve ORBITID DATA orbits (existing DOs)
        orbitid_query => 0,     # retrieve ORBITID QUERY orbits (EON queue DOs)
        moid_queue => 0,        # retrieve EON survivors for extra processing
    });
    my $row;

    if ($args{orbitId}) {
        $row = $dbh->selectrow_hashref(<<"SQL", undef, $args{orbitId});
$selectall_str
from orbits
where orbit_id=?
SQL
    }
    elsif ($args{derivedobjectNames}) {
        my $sth = $dbh->prepare(<<"SQL") or croak "can't prepare sql";
$selectderivedobjects_str
and do.object_name=?
SQL
        my @results;
        my $href;
        foreach my $objectName (@{$args{derivedobjectNames}}) {
            $sth->execute($objectName) or croak "can't execute sql";
            $href = $sth->fetchrow_hashref;
            if ($href) {
                push @results, _new_from_row($inst, $href);
            }
            else {
                warn "couldn't fetch orbit $objectName";
            }
        }
        return \@results;
    }
    elsif ($args{derivedobjectName}) {
        $row = $dbh->selectrow_hashref(<<"SQL", undef, $args{derivedobjectName});
$selectderivedobjects_str
and do.object_name=?
SQL
    }
    elsif ($args{derived}) {
        # Snag all unmerged derived objects.
        my $order;
        my $sth;
        my $id = "and do.status <> '$DERIVEDOBJECT_STATUS_MERGED'";       # all unmerged DOs
        my $since = '';

        if ($args{orderBy}) {
            # Caller specified own ordering
            croak "bogus ordering: $args{orderBy}" unless $args{orderBy} =~ /^q$/; # only q allowed
            $order = "order by $args{orderBy}";
        }
        else {
            $order = "order by do.derivedobject_id";
        }

        # Handle updated_since to retrieve derived objects modified since some date.
        if ($args{updated_since}) {
            $since = "and do.updated >= '$args{updated_since}'";
        }

        $sth = $dbh->prepare(<<"SQL");
$selectderivedobjects_str
$id
$since
$order
SQL

        $sth->{mysql_use_result} = 1;
        $sth->execute or croak $dbh->errstr;
        return PS::MOPS::DC::Iterator->new(sub {
            return _new_from_row($inst, $sth->fetchrow_hashref);
        });
    }
    elsif ($args{orbitid_data}) {
        # Snag all derived objects that are NOT currently in the EON queue waiting for
        # orbit identification ("ORBITID").
        my $sth = $dbh->prepare(<<"SQL");
$selectorbitid_data_str
SQL
        $sth->{mysql_use_result} = 1;
        $sth->execute or croak $dbh->errstr;
        return PS::MOPS::DC::Iterator->new(sub {
            return _new_from_row($inst, $sth->fetchrow_hashref);
        });
    }
    elsif ($args{orbitid_query}) {
        # Snag all derived objects that are NOT currently in the EON queue waiting for
        # orbit identification ("ORBITID").
        my $sth = $dbh->prepare(<<"SQL");
$selectorbitid_query_str
SQL
        $sth->{mysql_use_result} = 1;
        $sth->execute or croak $dbh->errstr;
        return PS::MOPS::DC::Iterator->new(sub {
            return _new_from_row($inst, $sth->fetchrow_hashref);
        });
    }
    elsif ($args{moid_queue}) {
        # Snag all orbits in MOID queue.
        my $sth = $dbh->prepare(<<"SQL");
$select_moidqueue_str
SQL
        $sth->{mysql_use_result} = 1;
        $sth->execute or croak $dbh->errstr;
        return PS::MOPS::DC::Iterator->new(sub {
            return _new_from_row($inst, $sth->fetchrow_hashref);
        });
    }

    if ($row) {
        return _new_from_row($inst, $row);
    }
    else {
        return undef;
    }
}


sub _insert {
    # Generic insert handler; accepts hashref from \%args or Observation object.
    my ($dbh, $orbspec) = @_;
    my @args;
    my $sql;
   
    @args = @{$orbspec}{qw(
        q e i node argPeri timePeri epoch hV residual chiSquared moid_1 arcLength_days convCode
    )};

    if ($orbspec->covariance) {
        $sql = sprintf <<"SQL", join(', ', @covariance_cols);
insert into orbits (
q, e, i, node, arg_peri, time_peri, epoch, h_v, residual, chi_squared, moid_1, arc_length_days, conv_code,
%s
)
values (
?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,
?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?
)
SQL
        push @args, @{$orbspec->{covariance}};

    }
    else {

        $sql = <<"SQL";
insert into orbits (
q, e, i, node, arg_peri, time_peri, epoch, h_v, residual, chi_squared, moid_1, arc_length_days, conv_code
)
values (
?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?
)
SQL
    }

    my $rv = $dbh->do($sql, undef, @args);
    if ($rv) {
        $orbspec->{orbitId} = $dbh->{'mysql_insertid'};     # last inserted ID
    }
                                                                                                              
    return $rv;
}


sub _update {
    # Update PSMOPS with our current member values.
    my ($dbh, $orb) = @_;
    my @args = @{$orb}{qw(
        q e i node argPeri timePeri epoch hV residual chiSquared moid_1 arcLength_days convCode orbitId
    )};
                                                                                                                                         
    my $rv = $dbh->do(<<"SQL", undef, @args);
update orbits set
    q=?, 
    e=?, 
    i=?, 
    node=?, 
    arg_peri=?, 
    time_peri=?, 
    epoch=?, 
    h_v=?, 
    residual=?,
    chi_squared=?,
    moid_1=?,
    arc_length_days=?,
    conv_code=?
where orbit_id=?

SQL

    return $rv;
}

sub _attribute {
    my ($dbh, $orb, $trk) = @_;
    my $rv = $dbh->do(<<"SQL", undef, $orb->orbitId, $trk->trackletId);
insert into orbit_attrib (orbit_id, tracklet_id)
values (?, ?)
SQL
    croak sprintf "_attribute failed: %s %s", $orb->orbitId, $trk->trackletId unless $rv;
    return $rv;
}
                                                                                                                                     
sub insert {
    my ($self) = @_;
    my $dbh = $self->{_inst}->dbh;
    return _insert($dbh, $self);
}


sub _chkundef {
    return defined($_[0]) ? $_[0] : 'undef';
}


my @_serialization_members = qw(
    orbitId
    q 
    e 
    i 
    node 
    argPeri 
    timePeri
    epoch
    hV
    residual
    chiSquared
    moid_1
    arcLength_days
    convCode
);


sub serialize {
    # Write out an orbit to a format we can use to create an orbit later.
    my ($self) = @_;

    if ($self->covariance && @{$self->covariance}) {
        return join(" ", 
            'MIF-OC',
            (map { _chkundef($self->{$_}) } @_serialization_members),
            (map { _chkundef($_) } @{$self->covariance}),               # append covariance matrix
        );
    }
    else {
        return join(" ", 
            'MIF-O',
            map { _chkundef($self->{$_}) } 
                @_serialization_members,
        );
    }
}


sub _dchkundef {
    return $_[0] eq 'undef' ? undef : $_[0];
}


sub modco_deserialize {
    # Create a MOPS orbit from serialization string.  Lines prefixed with MIF-OC
    # have additional covariance elements; MIF-O lines contain just cometary elements.
    my ($inst, $str) = @_;
    return undef if !$str;
    
    my @stuff = split /\s+/, $str;
    my @cov_elements = splice(@stuff, scalar @_serialization_members + 1);  # extract covariance off of @stuff (+1 for leading "MIF-O")
    my %hash;
    @hash{
        @_serialization_members
    } = map {
        _dchkundef($stuff[$_])
    } 1..($#_serialization_members + 1);

    if ($str =~ /^MIF-OC/) {
        return PS::MOPS::DC::Orbit->new($inst, %hash, covariance => \@cov_elements);
    }
    elsif ($str =~ /MIF-O/) {
        return PS::MOPS::DC::Orbit->new($inst, %hash);
    }
    else {
        return undef;
    }
}


1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

PS::MOPS::DC::Orbit - Module for manipulating MOPS DC orbit definitions

=head1 SYNOPSIS

  use PS::MOPS::DC::Orbit;

=head1 DESCRIPTION

This module provides the interface for storage, retrieval and
manipulation of orbits calculated by MOPS software.  Orbits are
six-parameter representation of orbits withing the Solar System.

Every time MOPS calculates an orbit as a result of derived object
creation, attribution, precovery, or orbit identification, the orbit
may be stored in the orbits table.  Derived objects will always point
to a row in the Orbits DC which represents the current orbit for
the derived object.

Orbit parameters and their units are:

  q (AU)
  e (deg)
  i (deg)
  node (deg)
  arg_peri (deg)
  time_peri (MJD)
  epoch (MJD)
  
=head1 SEE ALSO

PS::MOPS::DC::DerivedObject

=head1 AUTHOR

Larry Denneau <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright 2004 Institute for Astronomy, University of Hawaii

=cut
