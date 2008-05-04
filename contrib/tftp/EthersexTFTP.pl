#! /usr/bin/perl -w
use strict;
use Ethersex::TFTP;

my $tftp = Ethersex::TFTP->new("2001:4b88:10e4:0:aede:48ff:fef3:ef3e",
                               IpMode => 'v6', Debug => 1, 
			       #Crypto => "ABCDEF2342"
			      );
$tftp->binary;

if (scalar(@ARGV) != 3) {
 usage:
  print "Usage:
    EthersexTFTP.pl get <remote> <local>
    EthersexTFTP.pl put <local> <remote>

    The remote IP-address as well as the crypto-key currently have to be
    defined in the source code (right at the top).

";
  exit 0;
}

my ($action, $fn1, $fn2) = @ARGV;

if ($action eq "get") {
  $tftp->get($fn1, $fn2);
} 
elsif ($action eq "put") {
  $tftp->put($fn1, $fn2);
}
else {
  goto usage;
}
