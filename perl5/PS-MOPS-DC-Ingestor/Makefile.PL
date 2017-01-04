use 5.008008;
use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    NAME              => 'PS::MOPS::DC::Ingestor',
    VERSION_FROM      => 'lib/PS/MOPS/DC/Ingestor.pm', # finds $VERSION
    PREREQ_PM         => {}, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?     ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM  => 'lib/PS/MOPS/DC/Ingestor.pm', # retrieve abstract from module
       AUTHOR         => 'Larry Denneau, Jr. <denneau@ifa.hawaii.edu>') : ()),
);

# Hackery to prevent test errors because PERL_DL_NONLAZY=1.  This option requires all
# dynamically-loaded symbols to be verified at startup, but CFITSIO fails this.
package MY;
sub test {
    my $self = shift;
    my $result = $self->SUPER::test(@_);
    $result =~ s/PERL_DL_NONLAZY=1//g;
    return $result;
}
