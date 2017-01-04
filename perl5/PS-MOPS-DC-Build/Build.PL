use strict;
use warnings;
use Module::Build;

my $builder = Module::Build->new(
    module_name         => 'PS::MOPS::DC::Build',
    license             => 'perl',
    dist_author         => 'Larry Denneau, Jr. <denneau@ifa.hawaii.edu>',
    dist_version_from   => 'lib/PS/MOPS/DC/Build.pm',
    requires => {
        'Test::More' => 0,
    },
    add_to_cleanup      => [ 'PS-MOPS-DC-Build-*' ],
);

$builder->create_build_script();
