package PS::MOPS::DC::DerivedObject;

use 5.008;
use strict;
use warnings;
use Carp;

use base qw(Exporter Class::Accessor);

our @EXPORT = qw(
    modcdo_retrieve
    modcdo_retrieveOrbits

    $DERIVEDOBJECT_STATUS_NEW
    $DERIVEDOBJECT_STATUS_MERGED
    $DERIVEDOBJECT_STATUS_KILLED
    $DERIVEDOBJECT_STABLE_PASS_NO
    $DERIVEDOBJECT_STABLE_PASS_PENDING
    $DERIVEDOBJECT_STABLE_PASS_YES
    $DERIVEDOBJECT_NAME_PREFIX
);
our %EXPORT_TAGS = ( 'all' => [ qw(
    modcdo_retrieve
    modcdo_retrieveOrbits

    $DERIVEDOBJECT_STATUS_NEW
    $DERIVEDOBJECT_STATUS_MERGED
    $DERIVEDOBJECT_STATUS_KILLED
    $DERIVEDOBJECT_STABLE_PASS_NO
    $DERIVEDOBJECT_STABLE_PASS_PENDING
    $DERIVEDOBJECT_STABLE_PASS_YES
    $DERIVEDOBJECT_NAME_PREFIX
) ] );
our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );
our $VERSION = '0.01';

use Params::Validate;
use PS::MOPS::Lib;
use PS::MOPS::Constants qw(:all);
use PS::MOPS::DC::Instance;
use PS::MOPS::DC::Iterator;
use PS::MOPS::DC::SSM;
use PS::MOPS::DC::Tracklet;


# Our stuff.
our $DERIVEDOBJECT_STATUS_NEW = 'I';                # new, normal status
our $DERIVEDOBJECT_STATUS_MERGED = 'M';             # DO was merged with another
our $DERIVEDOBJECT_STATUS_KILLED = 'K';             # DO has been marked as bad
our $DERIVEDOBJECT_STABLE_PASS_NO = 'N';            # DO does not have stable orbit
our $DERIVEDOBJECT_STABLE_PASS_PENDING = 'P';       # DO reached stable orbit, not yet precovered
our $DERIVEDOBJECT_STABLE_PASS_YES = 'Y';           # DO stable, fully precovered

our $selectall_str = <<"SQL";
select
    do.derivedobject_id derivedobject_id,
    do.orbit_id orbit_id,
    do.object_name object_name,
    do.classification classification,
    do.mops_classification mops_classification,
    do.ssm_id ssm_id,
    do.status status,
    do.stable_pass stable_pass,
    do.updated updated
SQL


__PACKAGE__->mk_accessors(qw(
    derivedobjectId
    orbitId
    objectName
    classification
    mopsClassification
    ssmId
    status
    stablePass
    updated
));

our $by_value_validate_args = {
    orbitId => 0,           # not required; unfound DOs can have no orbits
    objectName => 0,        # usually established at insert()
    classification => 0,
    mopsClassification => 0,
    ssmId => 0,
    status => 0,
    stablePass => 0,
    updated => 0,
};

our $DERIVEDOBJECT_NAME_PREFIX = 'L';
our $B62_TEMPLATE = $DERIVEDOBJECT_NAME_PREFIX . '00000000';      # => 'L00000000'


sub new {
    # Create a new unattached derived object, for later insertion into MOPS DC.
    # Indirect syntax is available: my $dobj = PS::MOPS::DC::DerivedObject->new(),
    # or modcdo_create()
    my $pkg = shift;
    my $inst = shift;
    my %self = validate(@_, $by_value_validate_args);
    $self{_inst} = $inst;
    $self{derivedobjectId} = undef;     # always undef for new DOs until inserted
    $self{status} ||= $DERIVEDOBJECT_STATUS_NEW;            # default to NEW
    $self{stablePass} ||= $DERIVEDOBJECT_STABLE_PASS_NO;    # default
    return bless \%self;
}


sub status {
    my ($self, $status) = @_;
    if (defined($status)) {
        my $dbh = $self->{_inst}->dbh;
        $dbh->do(<<"SQL", undef, $status, $self->derivedobjectId) or croak $dbh->errstr;
update derivedobjects set status=? where derivedobject_id=?
SQL
        $self->{status} = $status;
    }
    else {
        return $self->{status};
    }
}


sub stablePass {
    my ($self, $stable_pass) = @_;
    if (defined($stable_pass)) {
        my $dbh = $self->{_inst}->dbh;
        $dbh->do(<<"SQL", undef, $stable_pass, $self->derivedobjectId) or croak $dbh->errstr;
update derivedobjects set stable_pass=? where derivedobject_id=?
SQL
        $self->{stablePass} = $stable_pass;
    }
    else {
        return $self->{stablePass};
    }
}


sub _new_from_row {
    my ($inst, $row) = @_;
    return undef unless $row;   # sanity check
    my $dobj = PS::MOPS::DC::DerivedObject->new($inst,
        orbitId => $row->{orbit_id},
        objectName => $row->{object_name},
        classification => $row->{classification},
        mopsClassification => $row->{mops_classification},
        ssmId => $row->{ssm_id},
        status => $row->{status},
        stablePass => $row->{stable_pass},
        updated => $row->{updated},
    );
    $dobj->{derivedobjectId} = $row->{derivedobject_id};

    return bless $dobj;      # make object
}


sub modcdo_retrieve {
    # return orbit objects
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, { 
        derivedobjectId => 0,           # by derivedobjectId
        objectName => 0,        # by derived object name
        objectNames => 0,       # by derived object name list
        all => 0,               # all of em
        orderBy => 0,           # ordering
        updated_since => 0,     # only DOs updated since specified date
        orbit_where => 0,       # additional orbit selection clause, used only by 'all'
        trackletId => 0,        # look up by tracklet ID
    });
    my $row;
    my $order = 'order by derivedobject_id';
    my $select_str;

    my @where;
    my $where_str;

    if ($args{derivedobjectId}) {
        unshift @where, 'derivedobject_id=?';
        $where_str = @where ? ('where ' . (join ' and ', @where)) : '';
        $row = $dbh->selectrow_hashref(<<"SQL", undef, $args{derivedobjectId});
$selectall_str
from derivedobjects do
$where_str
SQL
    }
    elsif ($args{trackletId}) {
        # Fetch all derived objects that contain specified tracklet, and
        # return a list of derived objects, since there should always
        # be a single DO.
        unshift @where, 'tracklet_id=?';
        $where_str = @where ? ('where ' . (join ' and ', @where)) : '';
        my $sql = <<"SQL";
$selectall_str
from derivedobjects do join derivedobject_attrib using (derivedobject_id)
$where_str
SQL

        my $sth = $dbh->prepare($sql) or $dbh->errstr;
        $sth->{mysql_use_result} = 1;
        $sth->execute($args{trackletId}) or croak $sth->errstr;

        my @objects;
        my $do_href;
        while ($do_href = $sth->fetchrow_hashref()) {
            push @objects, _new_from_row($inst, $do_href);
        }
        return @objects;
    }
    elsif ($args{objectName}) {
        unshift @where, 'object_name=?';
        $where_str = @where ? ('where ' . (join ' and ', @where)) : '';
        $row = $dbh->selectrow_hashref(<<"SQL", undef, $args{objectName});
$selectall_str
from derivedobjects do
$where_str
SQL
    }
    elsif ($args{objectNames}) {
        unshift @where, 'object_name=?';
        $where_str = @where ? ('where ' . (join ' and ', @where)) : '';
        my $sql = <<"SQL";
$selectall_str
from derivedobjects do
$where_str
SQL

        my $sth = $dbh->prepare($sql) or croak "can't prepare sql: $sql";
        my @results;
        my $aref;
        my $dobj;
        foreach my $objectName (@{$args{objectNames}}) {
            $sth->execute($objectName) or croak "can't execute sql";
            $aref = $sth->fetchrow_arrayref;
            if ($aref) {
                $dobj = PS::MOPS::DC::DerivedObject->new(
                    orbitId => $aref->[1],
                    objectName => $aref->[2],
                    status => $aref->[3],
                );
                $dobj->{derivedobjectId} = $aref->[0];
                push @results, $dobj;
            }
            else {
                warn "couldn't fetch orbit $objectName";
            }
        }
        return \@results;
    }
    elsif ($args{all}) {
        # Handle updated_since to retrieve derived objects modified since some date.
        push @where, "do.updated >= '$args{updated_since}'" if $args{updated_since};

        my $sql;
        if ($args{orderBy}) {
            # Caller specified his own ordering
            croak "bogus ordering: $args{orderBy}" unless $args{orderBy} =~ /^q$/; # only q allowed
            $order = "order by o.$args{orderBy}";
            push @where, 'do.orbit_id=o.orbit_id';

            $where_str = @where ? ('where ' . (join ' and ', @where)) : '';
            $sql = <<"SQL";
$selectall_str
from derivedobjects do, orbits o
$where_str 
$order
SQL
        }
        elsif ($args{orbit_where}) {
            # Caller specified his own ordering
            push @where, 'do.orbit_id=o.orbit_id';
            push @where, $args{orbit_where} if $args{orbit_where};

            $where_str = @where ? ('where ' . (join ' and ', @where)) : '';
            $sql = <<"SQL";
$selectall_str
from derivedobjects do, orbits o
$where_str 
SQL
        }
        else {
            $where_str = @where ? ('where ' . (join ' and ', @where)) : '';
            $sql = <<"SQL";
$selectall_str
from derivedobjects do
$where_str 
SQL
        }

        my $sth = $dbh->prepare($sql) or $dbh->errstr;
        $sth->{mysql_use_result} = 1;
        $sth->execute or croak $sth->errstr;
        return PS::MOPS::DC::Iterator->new(sub {
            return _new_from_row($inst, $sth->fetchrow_hashref);
        });
    }
    else {
        croak "neither derivedobjectId or objectName specified";
    }
                                                                                                                                       
    if ($row) {
        return _new_from_row($inst, $row);
    }
    else {
        return undef;
    }
}
                                                                                                                                       
                                                                                                                                       
sub insert {
    # Generic insert handler; accepts hashref from \%args or Observation object.
    my ($self) = @_;
    my $dbh = $self->{_inst}->dbh;
    my @args = @{$self}{qw(
        orbitId objectName classification mopsClassification ssmId status stablePass
    )};
                                                                                                                                         
    my $rv = $dbh->do(<<"SQL", undef, @args);
insert into derivedobjects (
orbit_id, 
object_name, 
classification, 
mops_classification, 
ssm_id,
status,
stable_pass,
updated
)
values (
?, ?, ?, ?, ?, ?, ?, sysdate()
)
SQL
    if ($rv) {
        # Now convert the numeric derivedobject_id to a MOPS name.
        my $derivedobject_id = $dbh->{'mysql_insertid'};
        my $b62_name = mopslib_toB62($derivedobject_id, $B62_TEMPLATE);
        croak "base 62 conversion failed for $derivedobject_id" unless $b62_name;
        $self->{objectName} = $b62_name;
        $rv = $dbh->do(<<"SQL", undef, $b62_name, $derivedobject_id) or croak "can't set object name for $derivedobject_id";
update derivedobjects set object_name=? where derivedobject_id=?
SQL
        $self->{derivedobjectId} = $derivedobject_id;            # establish orbit ID
    }
    return $rv;
}


sub _update {
    # Update PSMOPS with our current member values.
    my ($self, $dbh) = @_;
    my @args = @{$self}{qw(
        orbitId objectName classification mopsClassification ssmId status stablePass derivedobjectId
    )};

    my $rv = $dbh->do(<<"SQL", undef, @args);
update derivedobjects set
    orbit_id=?,
    object_name=?,
    classification=?,
    mops_classification=?,
    ssm_id=?,
    status=?,
    stable_pass=?,
    updated=sysdate()
where derivedobject_id=?
SQL

    return $rv;
}


sub _attribute {
    # Low-level attribute atracklet to derived object.
    my ($self, $dbh, $trk) = @_;
    my $rv = $dbh->do(<<"SQL", undef, $self->derivedobjectId, $trk->trackletId);
insert into derivedobject_attrib (derivedobject_id, tracklet_id)
values (?, ?)
SQL
    croak sprintf "_attribute failed: %s %s", $self->derivedobjectId, $trk->trackletId unless $rv;
    return $rv;
}


sub _reattribute {
    # Low-level re-attribute tracklet to different derived object.
    # Bug 1097: we now pass the child object into _reattribute() so that we know
    # unequivocally which tracklets to re-assign.  Previously we only looked at
    # tracklet ID, which is not safe in higher-density simulations.
    my ($self, $dbh, $trk, $child_do) = @_;
    my $rv = $dbh->do(<<"SQL", undef, $self->derivedobjectId, $trk->trackletId, $child_do->derivedobjectId);
update derivedobject_attrib set
derivedobject_id=?
where tracklet_id=? and derivedobject_id=?
SQL
    croak sprintf "_reattribute failed: %s %s", $self->derivedobjectId, $trk->trackletId unless $rv;
    return $rv;
}


sub _classify {
    # Update our SSM id by inspecting new added tracklets and determining
    # if they match our current SSM ID.  When the tracklets become mixed
    # then set the SSM ID to NULL.  If no tracklets were provided then
    # load the tracklets and inspect the detections by hand.
    my ($self) = @_;
    my $inst = $self->{_inst};
    my @dets = $self->fetchDetections();      # get detections if not provided
    my ($classification, $ssmName) = modcd_classifyDetections($inst, @dets);
    my $ssmId;
    if ($ssmName) {
        # We have a clean classification.  Save our ssmId and update our D-criteria for orbits.
        my $ssm = modcs_retrieve($inst, objectName => $ssmName);
        $ssmId = $ssm->ssmId if $ssm;
    }

    $self->ssmId($ssmId);
    $self->classification($classification);
}


sub fetchDetections {
    # Return all of this derived object's detections.
    my ($self) = @_;
    use PS::MOPS::DC::Detection;
    my $det_i = modcd_retrieve($self->{_inst}, derivedobject => $self);
    my $det;
    my @dets;
    while ($det = $det_i->next) {
        push @dets, $det;
    }
    return @dets;
}


sub fetchTracklets {
    # Return all of this derived object's tracklets.
    my ($self) = @_;
    use PS::MOPS::DC::Tracklet;
    my $trk_i = modct_retrieve($self->{_inst}, derivedobjectId => $self->derivedobjectId);
    my $trk;
    my @trks;
    while ($trk = $trk_i->next) {
        push @trks, $trk;
    }
    return @trks;
}


sub fetchOrbit {
    # Return current orbit object for this derived object.
    my ($self) = @_;
    use PS::MOPS::DC::Orbit;
    return modco_retrieve($self->{_inst}, derivedobjectName => $self->objectName);
}


sub attributeTracklets {
    # "Attribute" the specified tracklet to this orbit.  This method does
    # not modify orbital parameters; this should have been done by the caller prior
    # to calling attributeTracklet.  In the future we should accept a hash of
    # new orbital parameters.
    my $self = shift;
    my %args = validate(@_, {
        TRACKLETS_AREF => 1,        # arrayref of tracklets
        ORBIT => 0,                 # new orbit for attribution (optional)
        CHILD => 0,                 # original derived object owner of tracklets
        
    });
##    my ($self, $orbit, @tracklets) = @_;              # get self, elems HASHREF, trk obj
    my $orbit = $args{ORBIT};

    # What we need to do:
    # 1. Associate tracklet with this orbit.
    # 2. Mark tracklet as ATTRIBUTED
    my $dbh = $self->{_inst}->dbh;
    $self->{_inst}->pushAutocommit(0);
    eval {  
        foreach my $trk (@{$args{TRACKLETS_AREF}}) {
##            if (!$args{CHILD}) {
##                # New derived object or new attribution.
##                $self->_attribute($dbh, $trk); 
##            }
##            else {
##                # Transfer of tracklets from child DO to $self.
##                $self->_reattribute($dbh, $trk, $args{CHILD});
##            }

            # We now want to always _attribute(), preserving the most recent DERIVEDOBJECT_ATTRIB
            # associations for the retired CHILD object.  Our previous method of re-attributing
            # tracklets discarded some useful information.
            $self->_attribute($dbh, $trk); 
            $trk->status($TRACKLET_STATUS_ATTRIBUTED);
        }

        # Assign new orbit.
        if ($orbit) {
            $self->orbitId($orbit->orbitId);
            $self->mopsClassification(mopslib_classifyObject($orbit));
        }

        # Analyze detections and classify appropriately.
        $self->_classify();

        # Write to DB.
        $self->_update($dbh);
        $dbh->commit;
    };
##    warn $@ if $@;
    croak $@ if $@;
    $self->{_inst}->popAutocommit();
}


sub mergeChild {
    # Merge a child derived object with this one.  This is a multi-step operation that
    # does the following:
    # 1. Assign the child's tracklets to this derivedobject in derivedobject_attrib
    #    by updating the existing rows (not inserting new ones).
    # 2. Change this object's orbit to the specified orbit, which is presumably
    #    the orbit fitted to all detections between the two derived objects.
    # 3. Mark the child as merged 'M'.
    # This process leaves dangling the original parent orbit and the
    # child orbit.
    my ($self, $child, $new_orb) = @_;  # me, child obj, new orbit
    my $dbh = $self->{_inst}->dbh;
    $self->{_inst}->pushAutocommit(0);             # save autocommit state

    $self->orbitId($new_orb->orbitId);

    my @tracklets = $child->fetchTracklets();       # need child's tracklets to append to this object
    $self->attributeTracklets(
        ORBIT => $new_orb,
        TRACKLETS_AREF => \@tracklets,
#        CHILD => $child,               # no longer used
    );
    $self->_classify();                 # set object classification, ssmId
    $self->mopsClassification(mopslib_classifyObject($new_orb));
    $self->_update($dbh);               # update parent row to point to new orbit

    $child->status($DERIVEDOBJECT_STATUS_MERGED);       # mark as merged
    $child->_update($dbh);              # update child row

    $dbh->commit;                       # flush changes
    $self->{_inst}->popAutocommit();               # restore autocommit state
}


1;
__END__


=head1 NAME

PS::MOPS::DC::DerivedObject - Module for manipulating MOPS DC derived objects

=head1 SYNOPSIS

  use PS::MOPS::DC::DerivedObject;

=head1 DESCRIPTION

Manipulate MOPS DC DerivedObject objects.

=head1 METHODS

=item new

Create a new, unattached derived object.

=item modcdo_retrieve

Retrieve one or many derived objects from the specified instance's
database.

=item status

Fetch or update the derived object's 'status' member.

=item stablePass

Fetch or update the derived object's 'stablePass' member.

=item insert

Write an unattached derived object previously created with new() to the
instance's database.

=item fetchDetections

Return all the derived object's detections in a list of Detection objects.

=item fetchTracklets

Return all the derived object's tracklets in a list of Tracklet objects.

=item fetchOrbit

Return the derived object's orbit as an Orbit object.

=item attributeTracklets

Associate one or more tracklets to the derived object.  Update the instance's
database appropriately.

=item mergeChild

Merge a child object with this derived object by re-associating the child object's
tracklets with this derived object and replacing the orbit.

=head2 EXPORT

None by default.

=head1 SEE ALSO

PS::MOPS::DC::Connection
PS::MOPS::DC::Field
PS::MOPS::DC::Orbit

=head1 AUTHOR

Larry Denneau <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright 2004 Institute for Astronomy, University of Hawaii

=cut
