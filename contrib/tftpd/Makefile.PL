use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    'NAME'		=> 'Ethersex::TFTPd',
    'VERSION_FROM'	=> 'TFTPd.pm', # finds $VERSION
    'AUTHOR'            => 'Stefan Siegl <stesie@brokenpipe.de>',
);
