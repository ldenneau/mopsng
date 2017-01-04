package PS::MOPS::DC::Residuals;

use 5.008;
use strict;
use warnings;
use Carp;

use base qw(Exporter Class::Accessor);

our @EXPORT = qw(
    modcr_retrieve
);
our $VERSION = '0.01';

use Params::Validate ':all';
use Astro::SLA;
use PS::MOPS::Lib;
use PS::MOPS::Constants qw(:all);
use PS::MOPS::DC::Instance;
use PS::MOPS::DC::Detection;
use PS::MOPS::DC::Iterator;

# Our stuff here.
__PACKAGE__->mk_accessors(qw(
    detId
    trackletId
    raResid_deg
    decResid_deg
    raError_deg
    decError_deg
    astReject
));

# Used when creating or inserting by value.
our $new_validate_args = {
    detId => 1,
    trackletId => 1,
    raResid_deg => 1,
    decResid_deg => 1,
    raError_deg => 0,
    decError_deg => 0,
    astReject => 0,
};



our $selectall_sql = <<"SQL";
select
    det_id,
    tracklet_id,
    ra_resid_deg,
    dec_resid_deg,
    ra_error_deg,
    dec_error_deg,
    ast_reject
from residuals
SQL


sub new {   
    # Instantiate a residuals object.
    my $pkg = shift;
    my $inst = shift;
    my %self = validate(@_, $new_validate_args);
    $self{_inst} = $inst;

    # Defaults
    # detId, trackletId, raResid_deg, decResid_deg, raError_deg, decError_deg required
    $self{astReject} ||= 0;
    

    return bless \%self;
}


# Methods
sub insert {
    my ($self) = @_;
    my $inst = $self->{_inst};
    my $dbh = $inst->dbh;
    my $sth;

    # Need to check for existing first, and update if so.
    my @existing = $dbh->selectrow_array("select det_id, tracklet_id from residuals where det_id=? and tracklet_id=?", undef, $self->detId, $self->trackletId);
    if (@existing) {
        # Exists, so update.
        $sth = $dbh->prepare(<<"SQL") or die "prepare failed";
update residuals
set 
    ra_resid_deg=?, dec_resid_deg=?, ra_error_deg=?, dec_error_deg=?, ast_reject=? 
where det_id=? and tracklet_id=?
SQL
        $sth->execute(@{$self}{qw(
            raResid_deg decResid_deg raError_deg decError_deg astReject
            detId trackletId
        )}) or die $sth->errstr;
        $sth->finish or die $sth->errstr;
    }
    else {
        # Doesn't exist, so insert.
        $sth = $dbh->prepare(<<"SQL") or die "prepare failed";
insert into residuals (
det_id, tracklet_id, 
ra_resid_deg, dec_resid_deg, ra_error_deg, dec_error_deg, ast_reject
)
values (
?, ?,
?, ?, ?, ?, ?
)
SQL
        $sth->execute(@{$self}{qw(
            detId trackletId
            raResid_deg decResid_deg raError_deg decError_deg astReject
        )}) or die $sth->errstr;
        $sth->finish or die $sth->errstr;
    }

}


sub modcr_retrieve {
    # Return a single residuals object given a detection ID.
    my $inst = shift;
    my $dbh = $inst->dbh;
    my %args = validate(@_, {
        detId => 1,
    });

    if ($args{detId}) {
        my $sth = $dbh->prepare(<<"SQL") or croak $dbh->errstr;
select 
    det_id detId, tracklet_id trackletId,
    ra_resid_deg raResid_deg, dec_resid_deg decResid_deg,
    ra_error_deg raError_deg, dec_error_deg decError_deg,
    ast_reject astReject
from residuals
where det_id=?
SQL
        $sth->execute($args{detId}) or croak $dbh->errstr;
        return PS::MOPS::DC::Residiual->new(
            $inst,
            %{$sth->fetchrow_hashref()},
        );
    }
    else {
        confess "unsupported invocation method";
    }
}


1;
__END__

=head1 NAME

PS::MOPS::DC::Residuals - Perl extension for manipulating MOPS postfit residuals

=head1 SYNOPSIS

  use PS::MOPS::DC::Residuals

=head1 DESCRIPTION

Coming.

=head2 EXPORT

modcr_retrieve

=head1 SEE ALSO

PS::MOPS::DC
PS::MOPS::DC::Detection

=head1 AUTHOR

Larry Denneau, denneau@ifa.hawaii.edu

=head1 COPYRIGHT AND LICENSE

Copyright 2010 by Larry Denneau, Institute for Astronomy, University of Hawaii.

=cut

