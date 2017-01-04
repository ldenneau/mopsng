use ExtUtils::MakeMaker;
$Verbose = 1;
WriteMakefile(
    NAME   => 'PS::MOPS::GCR::gcf',
    SKIP   => [qw(all static static_lib dynamic dynamic_lib)],
    clean  => {'FILES' => 'libgcf$(LIB_EXT)'},
    INC               => '-I. -std=c99', # e.g., '-I. -I/usr/include/other'
);
sub MY::top_targets {
'
all :: static
pure_all :: static
static ::       libgcf$(LIB_EXT)
libgcf$(LIB_EXT): $(O_FILES)
	$(AR) cr libgcf$(LIB_EXT) $(O_FILES)
	$(RANLIB) libgcf$(LIB_EXT)
';
}
