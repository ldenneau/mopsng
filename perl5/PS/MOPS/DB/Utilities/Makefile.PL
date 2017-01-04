use 5.008;
use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.

my $MOPS_HOME = "$ENV{MOPS_HOME}";

WriteMakefile(
    'NAME'		=> 'PS::MOPS::DB::Utilities',
    'VERSION_FROM'	=> 'Utilities.pm', # finds $VERSION
    'PREREQ_PM'		=> {}, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?    ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM => 'Utilities.pm', # retrieve abstract from module
       AUTHOR     => 'Larry Denneau <denneau@ifa.hawaii.edu>') : ()),
    'LIB' => "$MOPS_HOME/lib/perl5",
    'OBJECT'    => 'Utilities_wrap.o',
    'CCFLAGS'   => "-I$MOPS_HOME/include -D_GNU_SOURCE",
    'LDDLFLAGS'   => "-shared -lmodc -locci -lclntsh -L/usr/lib/oracle/10.1.0.2/client/lib -L$MOPS_HOME/lib"
);
