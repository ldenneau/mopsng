use 5.008;
use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    NAME              => 'PS::MOPS::DC::History::Attribution',
    VERSION_FROM      => 'lib/PS/MOPS/DC/History/Attribution.pm', # finds $VERSION
    PREREQ_PM         => {}, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?     ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM  => 'lib/PS/MOPS/DC/History/Attribution.pm', # retrieve abstract from module
       AUTHOR         => 'A. U. Thor <denneau@localdomain>') : ()),
);
