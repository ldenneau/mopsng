package PS::MOPS::IDManager;

use 5.008;
use strict;
use warnings;
use Carp;

our $VERSION = '0.01';


# Our stuff.
use Params::Validate;
use subs qw(
    analyze
    query
    num_objs_using_member
    arbitrate
);


sub new {
    # Return a new object that will hold data and results for an ID management operation.
    my $pkg = shift;
#    my %self = validate(@_);
    my %self;
    $self{_USAGE} = {};             # tables of det/trk usage
    $self{_OBJS} = [];              # source objects added by add()
    $self{_OBJS_BY_NAME} = {};      # look up object data by name
    return bless \%self;
}


sub add {
    my ($self, $obj) = @_;
    push @{$self->{_OBJS}}, $obj;     # just add to list of objs
    $self->{_OBJS_BY_NAME}->{$obj->{ID}} = $obj;
}


sub analyze {
    my ($self) = @_;
    my $obj;
    my $member_id;

    # First, build our internal usage tree of dets/trks and the objects that
    # use them.  We do this now, after all objects have been added, so that
    # we guarantee that each usage list for each tracklet is sorted by FOM,
    # best-to-worst.
    my @sorted_objs = sort {
        $b->{FOM} <=> $a->{FOM}
    } @{$self->{_OBJS}};
    $self->{_OBJS} = \@sorted_objs;       # replace list with sorted list

    foreach $obj (@sorted_objs) {
        foreach $member_id (@{$obj->{MEMBERS}}) {
            push @{$self->{_USAGE}->{$member_id}}, $obj->{ID};
        }
    }

    # Debuggish.  Remove det/trk IDs with only one user-object, so we can easily see others.
    my @delete;
    foreach my $id (keys %{$self->{_USAGE}}) {
        push @delete, $id if scalar @{$self->{_USAGE}->{$id}} < 2;
    }
    foreach my $id (@delete) {
        delete ${$self->{_USAGE}}{$id};
    }

    # Now, the plan is:
    # 1. Sort the source derived object ID list by residual, in ascending order.
    # 2. For each derived object (in sorted order), examine its member tracklets, and
    # if a tracklet has multiple associations, perform an arbitration against the
    # group of derived objects that use the tracklet.  The arbitration should
    # set accept/reject flags for each of the derived objects.  The set of "arbitratees"
    # must be the set of derived objects that have not (yet) been rejected.
    # 3. If our object has been rejected, simply stop and go on to the next object.

    # 1. Generate sorted list (by residual) of all derived objects.  We want to give
    # the lowest-residual objects first crack at everything.
    foreach $obj (@sorted_objs) {
        next if $obj->{REJECTED};              # already rejected by somebody, on to next object

        # For each obj, examine each member det/trk.
        foreach $member_id (@{$obj->{MEMBERS}}) {
            $self->arbitrate($obj, $member_id);           # will set flags in objs, including $obj
            last if $obj->{REJECTED};
        }
    }
}


sub query_rejected {
    # Return the analysis result of a derived object.
    my ($self, $do_id) = @_;
    die("unfound object ID $do_id") unless exists(${$self->{_OBJS_BY_NAME}}{$do_id});
    return $self->{_OBJS_BY_NAME}->{$do_id}->{REJECTED};
}


sub users {
    my ($self, $member_id) = @_;
    # Return a list of objects using specified member ID.
    return grep {!$self->{_OBJS_BY_NAME}->{$_}->{REJECTED}} @{$self->{_USAGE}->{$member_id}};
}


sub arbitrate {
    my ($self, $ref_obj, $member_id) = @_;

    # For this member_id (usually a tracklet ID), get its list of derived objects
    # that use this member.  If the number is > 1, then select none or one of
    # the using objects based on the provided FOM.  If the number is equal to
    # one, simply return.
    # $ref_obj is the source object under investigation in the analyze() loop.  Since
    # all objects are sorted by FOM, this object should have the highest FOM.
    my @user_objs = map { $self->{_OBJS_BY_NAME}->{$_} } $self->users($member_id);
    my $num_objects = scalar @user_objs;        # readability
    return if $num_objects <= 1;            # one (or zero!) users, don't reject

    # If we have exactly two using DOs, choose one or neither.  If there are
    # three or more, make a decision based on the two best objects, then apply the decision
    # to the entire group, e.g. if $ref_obj is accepted, reject the remaining objects, and
    # if neither is selected, reject all objects.  Note that no rejection flags are set
    # on the group if $ref_obj is rejected other than for $ref_obj.  Also note that we don't
    # really need to even *look* at identity of the top two objects: if their FOMs are close,
    # we reject everybody, and if the first is much better, we keep it (do not reject it).
    # For insurance we'll verify that the first is our ref_obj, since we sorted everybody
    # by descending FOM at the beginning.

    if ($user_objs[0]->{FOM} > 1.5 * $user_objs[1]->{FOM}) {
        # First object has "much better" FOM than others.  So we're done -- just bail.
        unless ($user_objs[0]->{ID} eq $ref_obj->{ID}) {
            die(sprintf "Assertion failed: best user object is not zeroth: %s", $user_objs[0]->{ID});
        }
        # Reject the others.
        foreach my $obj (@user_objs[1..$#user_objs]) {
            $obj->{REJECTED} = $ref_obj->{ID};      # store obj that killed 
        }
        return;
    }
    else {
        # First object has similar FOM to second object.  Reject all of them.
        foreach my $obj (@user_objs) {
            $obj->{REJECTED} = $ref_obj->{ID};      # store obj that killed
        }
        return;
    }
}


1;
__END__

=head1 NAME

PS::MOPS::IDManager - Module for select of conflicting objects populated by detections or tracklets

=head1 SYNOPSIS

    use PS::MOPS::IDManager;
    $mgr = new PS::MOPS::IDManager;

    # Repeat for all derived objects.
    $id = $do->derivedobjectId;
    $residual = $orbit->resid;
    $stuff = {
        ID => $id,
        MEMBERS => [
            map { $_->trackletId } $do->fetchTracklets(),
        ],
        FOM => 1 / ($residual || 999),   # FOM == figure of merit for selection, higher = better
    };
    $mgr->add($stuff);

    # After adding all derived objects.
    $mgr->analyze();
  
    # Now look at who's left.
    if ($mgr->query($id)) {
        keep_derivedobject($do);
    }


=head1 DESCRIPTION

Analyze tracklet or detection memberships in a list of derived objects to
identify conflicting identifications, and select "winners" in the case of
conflicts if possible.

=head1 AUTHOR

Larry Denneau <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright 2008 Institute for Astronomy, University of Hawaii

=cut
