package PS::MOPS::Detectability;

=head1 NAME

PS::MOPS::FITS::Detectability.pm - Manipulate MOPS detectability FITS files

=head1 SYNOPSIS

use PS::MOPS::FITS::Detectability;

$fits = PS::MOPS::FITS::Detectability->new()

=head1 DESCRIPTION

Check the datastore for new files, retrieve any new files, then
insert them into the simulation database.

=head1 PUBLIC FUNCTIONS

=head2 find_fetch_and_update

Check the data store for any new files.  If new files are found, retrieve
them, and enter them into the database.  Then update the data store index
so that the files are not found on the next pass.

=head1 BUGS

=cut

use strict;
use warnings;
use Carp;

use Astro::FITS::CFITSIO qw(:constants);
use File::Temp;
use LWP;

our $VERSION = '0.01';


sub _check_status {
    # Private utility routine to just report the status and die
    # if the status is non-zero.
    my ($fits_status, $msg) = @_;
	if ($fits_status) {
		Astro::FITS::CFITSIO::fits_report_error('STDERR', $fits_status);
		confess "FITS status: $fits_status: $msg";
	}
}


sub write_request {

=item write_request

Write a detectability request to the local datastore.

=cut
	my ($inst, $keys, $dets) = @_;

    my %keydefs = (
        QUERY_ID => [ TSTRING, 20, 'Unique Query identifier' ],
        FPA_ID => [ TSTRING, 20, 'IPP FPA identifier' ],
        'MJD-OBS' => [ TDOUBLE, 0, 'Midpoint time' ],
        FILTER => [ TSTRING, 3, 'Effective filter used (g/r/i/z/y/w)' ],
        OBSCODE => [ TSTRING, 3, 'MPC observatory code'],
        STAGE => [ TSTRING, 20, 'IPP processing stage (chip/stack/warp/diff)'],
    );

    my @cols = qw(
        ROWNUM
        RA1_DEG
        DEC1_DEG
        RA2_DEG
        DEC2_DEG
        MAG
    );

    my %coldefs = (
#        ROWNUM => [ TLONGLONG, '1D1' ],
        ROWNUM => [ TSTRING, '20A' ],
        RA1_DEG => [ TDOUBLE, '1D1' ],
        DEC1_DEG => [ TDOUBLE, '1D1' ],
        RA2_DEG => [ TDOUBLE, '1D1' ],
        DEC2_DEG => [ TDOUBLE, '1D1' ],
        MAG => [ TDOUBLE, '1D1' ],
    );

    my $EXTNAME = 'MOPS_DETECTABILITY_QUERY';
    my $TABLEVER = 'UNKNOWN';
    my $fptr;           # FITS file pointer
	my $fits_status = 0;
	my @null_ttype = ();
	my @null_tform = ();


    # Setup.  Figure out where to write the request.
    my $fieldId = $keys->{fieldId};
    my $nn = $keys->{nn};
    my $dir = $keys->{dir} || '.';


	# Open the file.  Want $dir/DBNAME.NN.FIELD_ID.
    my $filename = "$dir/" . join('.', $inst->dbname, $nn, sprintf("%04d", $fieldId));
    $fptr = Astro::FITS::CFITSIO::create_file('!' . $filename, $fits_status);
    _check_status($fits_status, "cannot create file $filename");

    $fptr->create_img(16, 0, undef, $fits_status);
    _check_status($fits_status, "cannot create img $filename");


	# Create the table (currently empty).
	$fptr->create_tbl(BINARY_TBL,
		0, 0, \@null_ttype, \@null_tform, 0, $EXTNAME, $fits_status);
    _check_status($fits_status, "cannot create table");

	my $row = 1;

	for my $key (sort keys %keydefs) {
		my $type = $keydefs{$key}[0];
		my $width = $keydefs{$key}[1];
		my $comment = $keydefs{$key}[2];
		my $value = $keys->{$key};

		$fptr->write_key($type, $key, $value, $comment, $fits_status);
        _check_status($fits_status, "cannot write key $key to file $filename");
	}

	$fptr->write_key(TSTRING, 'TABLEVER', $TABLEVER, 'Table Version', $fits_status);
    _check_status($fits_status, "cannot write key 'TABLEVER'");

	# Create the columns of the table.  For some reason, I get a segfault
	# Create the matching format string.
	my @tform = map { $coldefs{$_}[1] } @cols;

	# Now create the columns themselves
	$fptr->insert_cols(1, @cols + 0, \@cols, \@tform, $fits_status);
    _check_status($fits_status, "cannot insert columns");


    my $i;
    my $col;
    my @datum;
    my $ttype;
    my $str;
    foreach my $det (@{$dets}) {
        $str = sprintf "%d", $det->detId;
        $fptr->write_col(TSTRING, 1, $row, 1, 1, [$det->detId], $fits_status);
        _check_status($fits_status, "error writing column ROWNUM");
        $fptr->write_col(TDOUBLE, 2, $row, 1, 1, [$det->ra], $fits_status);
        _check_status($fits_status, "error writing column RA1");
        $fptr->write_col(TDOUBLE, 3, $row, 1, 1, [$det->dec], $fits_status);
        _check_status($fits_status, "error writing column DEC1");
        $fptr->write_col(TDOUBLE, 4, $row, 1, 1, [$det->ra], $fits_status);
        _check_status($fits_status, "error writing column RA2");
        $fptr->write_col(TDOUBLE, 5, $row, 1, 1, [$det->dec], $fits_status);
        _check_status($fits_status, "error writing column DEC2");
        $fptr->write_col(TDOUBLE, 6, $row, 1, 1, [$det->mag], $fits_status);
        _check_status($fits_status, "error writing column MAG");
        $row++;
    }

	$fptr->close_file($fits_status);
    _check_status($fits_status, "cannot close file");
}


sub read_response {

=item read_response

Read detectability responses from the specified datastore URL.
Return the responses as a list of (ROWNUM, DETECT_N, DETECT_F)
tuples.

=cut
	my ($url, $ua) = @_;
    my $res;                # result of HTTP fetch
    my $tempdir;            # where we're storing retrieved file
    my $filename;

    $ua = LWP::UserAgent->new() if !$ua;        # create if necessary
    $tempdir = File::Temp::tempdir('detrXXXX', DIR => '/tmp', CLEANUP => 1);
    $filename = "$tempdir/foo";
    $res = $ua->get($url, ':content_file' => $filename);

    my $status = 0;

	my $fptr = Astro::FITS::CFITSIO::open_file($filename, READONLY, $status);
    _check_status($status, "error while reading $filename");

	$fptr->movrel_hdu(1, undef, $status);
    _check_status($status, "error moving to HDU 1 in $filename");

    my $nrows;
    $fptr->get_num_rows($nrows, $status);
    _check_status($status, "error retrieving num_rows");

    my $anynul;     # dummy
    my %results;    # det_num => tuple results
    my @datum;
    my @datum2;
    my @datum3;
    my $row = 1;
    my ($rownum, $detect_n, $detect_f, $target_flux);

    foreach (1..$nrows) {
        # ROWNUM.
        $fptr->read_col(TSTRING, 1, $row, 1, 1, 0, \@datum, $anynul, $status);
        _check_status($status, "error reading column");
        $rownum = $datum[0];

        # DETECT_N
        $fptr->read_col(TUINT, 2, $row, 1, 1, 0, \@datum, $anynul, $status);
        _check_status($status, "error reading column");
        $detect_n = $datum[0];

        # DETECT_F
        $fptr->read_col(TDOUBLE, 3, $row, 1, 1, 0, \@datum, $anynul, $status);
        _check_status($status, "error reading column");
        $detect_f = $datum[0];

        # TARGET_FLUX
        $fptr->read_col(TDOUBLE, 4, $row, 1, 1, 0, \@datum, $anynul, $status);
        _check_status($status, "error reading column");
        $target_flux = $datum[0];

        $results{$rownum} = [ $detect_n, $detect_f, $target_flux ];
        $row++;
	}

	$fptr->close_file($status);
    _check_status($status, "cannot close file");

	return \%results;
}

1;

