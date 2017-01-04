package PS::MOPS::DC::IngestItem;

use 5.008;
use strict;
use warnings;

use base qw(Exporter Class::Accessor);

our $VERSION = '0.01';

use Params::Validate ':all';
use PS::MOPS::Constants;

# Field member descriptions:  see POD below.
__PACKAGE__->mk_accessors(qw(
    fileId
    fileSize
    fileType
    ingestDate
    status
));


our $validate_args = {
    fileId => 1,
    fileSize => 1,
    fileType => 1,
    status => 0,
};


sub new {
    my $pkg = shift;
    my $inst = shift;
    my %self = validate(@_, $validate_args);
    $self{_inst} = $inst;
    $self{status} ||= $PS::MOPS::Constants::INGEST_STATUS_NEW;
    
    my $dbh = $inst->dbh;

    # See if this object is already in the queue.
    $self{_exists} = $dbh->do("select file_id from ingest_status where file_id=?", undef, $self{fileId});
    if ($self{_exists} > 0) {
        $self{status} = $PS::MOPS::Constants::INGEST_STATUS_INGESTED;
    }

    return bless \%self;
}


sub mark_ingested {
    my ($self) = @_;
    my $sql;
    
    $sql = <<"SQL";
insert into ingest_status (file_id, file_size, file_type, ingest_date, status) values (
?, ?, ?, current_timestamp(), ?
)
SQL

    my $dbh = $self->{_inst}->dbh;
    my $sth = $dbh->prepare($sql);
    $sth->execute(@{$self}{qw(fileId fileSize FileType)}, $PS::MOPS::Constants::INGEST_STATUS_INGESTED) 
        or die $dbh->errstr;
}


sub fetch_oldest {
=pod

Module routine to fetch oldest ingested item so that we can query for
newer items in incoming datastore.

=cut
    my ($inst) = @_;
    my $sql;
    
    $sql = <<"SQL";
select file_id, file_size, file_type, status from ingest_status order by ingest_date desc limit 1
SQL
    
    my $dbh = $inst->dbh;
    my @row = $dbh->selectrow_array($sql);
    if (@row) {
        return PS::MOPS::DC::IngestItem->new($inst, 
            fileId => $row[0], 
            fileSize => $row[1], 
            fileType => $row[2], 
            status => $row[3]
        );
    }
    else {
        return undef;
    }
}

1;
__END__

=head1 NAME

PS::MOPS::DC::IngestItem - module for manipulating datastore ingest items

=head1 SYNOPSIS

  use PS::MOPS::DC::IngestItem;

  $item = PS::MOPS::DC::IngestItem->new($inst, $fileId, $fileSize, $fileType);
  if ($item->{status} ne $PS::MOPS::Constants::INGEST_STATUS_INGESTED) {
    ingest($fileId);
  }
  else {
    msg("File ID $fileId already ingested");
  }

=head1 DESCRIPTION

Provides a simple class for recording datastore ingest items as we
retrieve them to prevent duplicate retrievals from IPP.

=head1 AUTHOR

Larry Denneau <denneau@ifa.hawaii.edu>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2009 Institute for Astronomy, University of Hawaii

=cut
