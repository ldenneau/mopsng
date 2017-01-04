package PS::MOPS::DC::Global;

use 5.008;
use strict;
use warnings;

use base qw(Exporter Class::Accessor);
our @EXPORT = qw(
    modcg_listOCs
    modcg_listMJDs
);
our $VERSION = '0.01';


# Field member descriptions:  see POD below.
use Params::Validate qw(:all);


sub modcg_listOCs {
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


sub modcg_listMJDs {
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


1;
__END__

=head1 NAME

PS::MOPS::DC::Global - Module for obtaining global MOPS DC information.

=head1 SYNOPSIS

  use PS::MOPS::DC::Instance;
  use PS::MOPS::DC::Global;
  my $inst = PS::MOPS::DC::Instance->new(DBNAME => 'psmops_test');
  my @mjds = modcg_listMJDs($inst);
  my @ocnums = modcg_listOCs($inst);

=head1 DESCRIPTION

This module contains routines for obtaining instance-global information about an
existing MOPS instance.

=item modcg_listMJDs

Return a list of integer MJDs for all MOPS fields in the instance.

=item modcg_listOCs

Return a list of OC numbers for all MOPS fields in the instance.

=head1 AUTHOR

Larry Denneau <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2004 Institute for Astronomy, University of Hawaii

=cut
