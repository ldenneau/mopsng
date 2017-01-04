#!/usr/bin/env perl

my $verbose = 0;

foreach my $name (@ARGV) {
    next if !($name =~ /1_/);
    my (undef, undef, undef, $new_name) = split "_", $name;
    die "failed to extract new name from $name\n" unless $new_name;
    print "$name $new_name\n" if $verbose;
    rename $name, $new_name or die "failed to rename $name\n";
}

