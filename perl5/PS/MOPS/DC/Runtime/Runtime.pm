package PS::MOPS::DC::Runtime;

use 5.008;
use strict;
use warnings;
use Carp;

use base qw(Exporter Class::Accessor);
our @EXPORT = qw(
    modcr_listOCs
    modcr_listMJDs
    modcr_insert
    modcr_retrieve

    $MOPS_SUBSYSTEM_GLOBAL
);
our $VERSION = '0.01';


# Our stuff here.
__PACKAGE__->mk_accessors(qw(
    eventId
    subsystem
    message
    data
    date_submitted
    status
));


# Subsystem definitions.  Basically these are defined within modules, not here, except for
# $MOPS_SUBSYSTEM_GLOBAL.
our $MOPS_SUBSYSTEM_GLOBAL = 'GLOBAL';


# Field member descriptions:  see POD below.
use Params::Validate qw(:all);
use PS::MOPS::Constants qw(:all);
use PS::MOPS::DC::Instance;
use PS::MOPS::DC::Iterator;


# Used when creating field or inserting by value.
our $by_value_validate_args = {
    eventId => { default => undef },
    subsystem => 1,
    message => 1,
    data => 0,
    date_submitted => 0,
    status => 0,
};


our $selectall_sql;
our $DEC;
$selectall_sql = <<"SQL";
select
    r.event_id, 
    r.subsystem, r.message, r.data, r.date_submitted, r.status
SQL


sub new {
    my $pkg = shift;
    my $inst = shift;
    my %self = validate(@_, $by_value_validate_args);
    $self{_inst} = $inst;
    return bless \%self;
}


sub _new_from_row {
    # Return a new Detection object from DBI row handle.
    my ($inst, $row) = @_;
    return undef if !$row;  # sanity check

    # Fill in defaults.
#    $row->{refMag} = $row->{mag} unless exists($row->{refmag});

    my $det;
    $det = PS::MOPS::DC::Runtime->new(
        $inst,
        eventId => $row->{event_id},
        subsystem => $row->{subsystem},
        message => $row->{message},
        data => $row->{data},
        dateSubmitted => $row->{date_submitted},
        status => $row->{status},
    );
    return bless $det;
}


sub _insert {
    # Generic insert handler; accepts hashref from \%args or Field object.
    my ($dbh, $stuff) = @_;
    my @args = @{$stuff}{qw(
        subsystem message data status
    )};
    my $rv;

    $rv = $dbh->do(<<"SQL", undef, @args);
insert into runtime (
    subsystem, message, data, status
)
values (
    ?, ?, ?, ?
)
SQL
    if ($rv) {
        $rv = $dbh->{'mysql_insertid'} if $rv;
        $stuff->{eventId} = $rv;
    }
    else {
        croak "Couldn't insert runtime event: " . $dbh->errstr;
    }
    return $rv;
}


sub insert {
    my $self = shift;
    return _insert($self->{_inst}->dbh, $self); # call generic handler
}


sub status {
    my ($self, $status) = @_;
    if ($status) {
        my $dbh = $self->{_inst}->dbh;
        $dbh->do(<<"SQL", undef, $status, $self->eventId) or croak $dbh->errstr;
update runtime set status=? where event_id=?
SQL
        $self->{status} = $status;
    }
    else {
        return $self->{status};
    }
}


sub modcr_listOCs {
    # return list of OCs that have fields in them.
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, { 
        before => 0
    });
    my $row;
   
    my $where = '';
    $where = "where ocnum < $args{before}" if $args{before};

    my $res = $dbh->selectcol_arrayref(<<"SQL") or die "can't prepare SQL";
select distinct ocnum
from `fields`
$where
order by ocnum desc
SQL
    return $res ? @$res : undef;
}


sub modcr_listMJDs {
    # return list of MJDs that have fields in them.
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, { 
        before => 0,
    });
    my $row;
   
    my $where = '';
    $where = "where epoch_mjd < $args{before}" if $args{before};

    my $res = $dbh->selectcol_arrayref(<<"SQL") or die "can't prepare SQL";
select distinct floor(epoch_mjd)
from `fields`
$where
order by epoch_mjd desc
SQL
    return $res ? @$res : undef;
}


sub modcr_insert {
    # Insert a runtime message into the current instance's runtime
    # table.
    my $inst = shift;
    my $msg = PS::MOPS::DC::Runtime->new($inst, @_);
    $msg->insert;
}


1;
__END__

=head1 NAME

PS::MOPS::DC::Runtime - Module for obtaining runtime MOPS DC information.

=head1 SYNOPSIS

  use PS::MOPS::DC::Instance;
  use PS::MOPS::DC::Runtime;
  my $inst = PS::MOPS::DC::Instance->new(DBNAME => 'psmops_test');
  my @mjds = modcr_listMJDs($inst);
  my @ocnums = modcr_listOCs($inst);

=head1 DESCRIPTION

This module contains routines for obtaining runtime information
about the MOPS database.  Some routines to extract other
database-wide information are included as well.

=item modcr_listMJDs

Return a list of integer MJDs for all MOPS fields in the instance.

=item modcr_listOCs

Return a list of OC numbers for all MOPS fields in the instance.

=head1 AUTHOR

Larry Denneau <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2004 Institute for Astronomy, University of Hawaii

=cut
