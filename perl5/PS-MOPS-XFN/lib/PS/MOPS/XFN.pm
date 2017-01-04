package PS::MOPS::XFN;

use 5.008;
use strict;
use warnings;

use base qw(Exporter);
our %EXPORT_TAGS = ( 'all' => [ qw(
    xfnCallFunction
    xfnSSMShapeMag
) ] );
our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );
our $VERSION = '0.01';

use Params::Validate;
use File::Temp;
use File::Slurp;


# SSH stuff; move to lib
my $SSH = 'ssh -q -n';
my $SSH_USER = 'mops';  # get from config
my $SCP = 'scp -q';


sub xfnCallFunction {
    my %args = validate(@_, {
            prog => 1,              # program to execute
            input => 1,             # input list (objects or keys, depending on keyidx)
            infn => 1,              # code ref to convert input to lines of text
            outfn => 1,             # code ref to convert output lines to hashref
            keyidx => 0,            # if set, col index of key in results; otherwise operate using lists
            host => 0,              # remote host to distribute job
        }
    );

    # Executable programs must conform to 'program --options'.  Input is
    # passed on stdin and output on stdout.

    # Set up temporary files for STDOUT and STDERR.
    my $tmp_STDOUT = File::Temp::tempnam('/tmp', 'xfnstdoutXXXXXXXX');
    my $tmp_STDERR = File::Temp::tempnam('/tmp', 'xfnstderrXXXXXXXX');
    my $input_file = File::Temp::tempnam('/tmp', 'xfninXXXXXXXX');
    my $output_file = File::Temp::tempnam('/tmp', 'xfnoutXXXXXXXX');
    my @input_lines;
    my @output;
    my $res;

    my $host = $args{host} || '';

    eval {
        my $item;
        foreach $item (@{$args{input}}) {
            push @input_lines, &{$args{infn}}($item);           # convert item to string using user-supplied fn
        }
        write_file($input_file, grep { defined $_ } @input_lines);  # write only non-undef lines

        if ($host) {
            # Remote host specified for job.  Copy input/output files.
            my $RMI = "/tmp/mopsxi$$.$host";    # remote input file
            my $RMO = "/tmp/mopsxo$$.$host";    # remote output file

            my $cmd = <<"EOF";
( $SCP $input_file $SSH_USER\@$host:$RMI; ( $SSH $SSH_USER\@$host "sh -c 'bin/$args{prog} $RMI $RMO 2>/dev/null >/dev/null; cat $RMO; rm $RMI $RMO </dev/null 2>&1 >/dev/null'") > $output_file.tmp; mv $output_file.tmp $output_file)
EOF
            system($cmd) == 0 or die "remote command failed: $cmd";
        }
        else {
            system("$args{prog} $input_file $output_file >$tmp_STDOUT 2>$tmp_STDERR") == 0
                or die "xfn failed: $args{prog} $input_file $output_file";
        }

        # Munge output back into Perlishness.
        my $line;
        if (exists($args{keyidx})) {
            my @output_lines = read_file($output_file);
            my %output_hash;
            foreach $line (@output_lines) {
                my @fields = split /\s+/, $line;
                $output_hash{$fields[$args{keyidx}]} = &{$args{outfn}}($line);
            }
            $res = \%output_hash;
        }
        else {
            # Array result
            my @output_lines = read_file($output_file);
            foreach $line (@output_lines) {
                push @output, &{$args{outfn}}($line);
            }
            $res = \@output_lines;
        }
    };

    die $@ if $@;

    unlink $tmp_STDOUT if -e $tmp_STDOUT;
    unlink $tmp_STDERR if -e $tmp_STDERR;
    unlink $input_file if -e $input_file;
    unlink $output_file if -e $output_file;

    return $res;
}


sub xfnSSMShapeMag {
    # XFN representation of shape magnitude calculation for
    # SSM objects.  Input is a ARRAYREF of SSM objectNames,
    # output is a HASHREF with objectName keys; vals are
    # computed shape magnitude.
    use PS::MOPS::DC::SSM;
    use PS::MOPS::DC::Shape;

    my %args = validate(@_, {
            instance => 1,      # MOPS instance
            epoch => 1,         # epoch to calc shape mags for
            objectNames => 1,   # SSM object names
            host => 0,          # pass-through remote host
        }
    );

    my $res;
    $res = PS::MOPS::XFN::xfnCallFunction(
        prog => 'shape.x',
        host => $args{host},
        keyidx => 0,                    # objectName is in 8th col
        input => $args{objectNames},
        infn => sub {
            # Given an input key, convert to a line of text for our program
            my $objectName = shift;
            my $sh = modcsh_retrieve($args{instance}, objectName => $objectName);
            my $obj = modcs_retrieve($args{instance}, objectName => $objectName);
            return undef unless $sh;        # shape spec doesn't exist

            return (join ' ',
                (
                    map { sprintf "%.10f", $_ } (
                        $obj->q,
                        $obj->e,
                        $obj->i,
                        $obj->node,
                        $obj->argPeri,
                        $obj->timePeri,
                        $obj->hV,
                        $obj->epoch
                    )
                ),
                $obj->objectName,
                (
                    map { sprintf "%.10f", $_ } (
                        $sh->g,
                        $sh->p,
                        $sh->period_d,
                        $sh->amp_mag,
                        $sh->a_km,
                        $sh->b_km,
                        $sh->c_km,
                        $sh->beta_deg,
                        $sh->lambda_deg,
                        $sh->ph_a,
                        $sh->ph_d,
                        $sh->ph_k,
                        $args{epoch}
                    )
                )
            ) . "\n";
        },
        outfn => sub {
            # Given an output line of text from our program, split it into a hashref
            my $line = shift;
            my @goo = split /\s+/, $line;
            return {
                mag => $goo[-1],    # [-1] => shape mag
            };
        }
    );

    return $res;
}


__END__

=head1 NAME

PS::MOPS::XFN - Perl extension for program-based batch function calls

=head1 SYNOPSIS

  use PS::MOPS::XFN qw(:all);
  xfnCallFunction(
    stuff
  );

=head1 DESCRIPTION

Description here eventually.

=head2 EXPORT

None by default.

=head1 AUTHOR

Larry Denneau, Jr., Institute for Astronomy, University of Hawaii.
denneau@ifa.hawaii.edu.

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2005 by Larry Denneau, Jr., Insitute for Astronomy,
University of Hawaii.

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.8.7 or,
at your option, any later version of Perl 5 you may have available.

=cut
