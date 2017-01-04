package PS::MOPS::DC::Config;

use 5.008;
use strict;
use warnings;

use base qw(Exporter Class::Accessor);

our @EXPORT = qw(
    modcfg_insert
    modcfg_retrieve
);
our $VERSION = '0.01';

use PS::MOPS::Constants;

# Field member descriptions:  see POD below.
__PACKAGE__->mk_accessors(qw(
    derivedobjectId
    eventId
    insertTimestamp
    status
));


sub modcfg_insert {
    # Write a config file to the config table by appending to the table.
    my ($inst, $config_text) = @_;
    
    my $dbh = $inst->dbh;
    my $sql = <<"SQL";
insert into config (config_text)
values (?)
SQL
    return $dbh->do($sql, 
                    undef, 
                    $config_text) or die "can't do SQL: $sql";
}


sub modcfg_retrieve {
    # Return the current (latest) config entry from the table.  This value
    # is unstructured text; it still needs to be parsed using PS::MOPS::Config.
    # Return the string retrieved from the DB.
    my ($inst) = @_;
    
    my $dbh = $inst->dbh;
    my $sql = <<"SQL";
select config_text
from config
order by config_id desc
limit 1
SQL

    my $res = $dbh->selectcol_arrayref($sql);
    return @{$res} ? $res->[0] : '';                # return row contents or empty string
}


1;
__END__

=head1 NAME

PS::MOPS::DC::Config - module for inserting/retrieving config data from MOPS database

=head1 SYNOPSIS

  use PS::MOPS::Config;         # parsing of config text, load from file
  use PS::MOPS::DC::Config;     # modcfg_XXX; insert/retreive from DB

  $config_text = modcfg_retrieve();
  $config_data = PS::MOPS::Config->new_from_string();


=head1 DESCRIPTION

This module manages the MOPS end-of-night (EON) queue.  Objects placed in
this queue are retrieved after all linking jobs are completed for a night.
Objects created in the linking process or a prior attribution are placed
into this queue for subsequent orbit identification and precovery.

First, the EON process retrieves all objects from the queue and performs
an ID2 orbit identification.  The survivors (unmerged objects) from this
step are marked with status 'P' indicating they are eligible for precovery.
The merged objects are marked with status 'X' indicating they are retired
from the queue.

After the EON processing is completed, the queue is flushed.

=head1 AUTHOR

Larry Denneau <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2007 Institute for Astronomy, University of Hawaii

=cut
