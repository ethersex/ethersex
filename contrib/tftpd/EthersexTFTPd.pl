#!/usr/bin/perl
use strict;
use Ethersex::TFTPd 0.02 qw(%OPCODES);

# change ROOTDIR to your TFTP root directory
my $rootdir = $ARGV[0];

unless(-d $rootdir)
{
	print "\nUsage: EthersexTFTPd.pl <rootdir>\n\n";
	exit 1;
}

# callback sub used to print transfer status
sub callback
{
	my $req = shift;
	if($req->{'_REQUEST_'}{'OPCODE'} eq $OPCODES{'RRQ'})
	{
		# RRQ
		printf "block: %u\/%u\n", $req->{'_REQUEST_'}{'LASTACK'}, $req->{'_REQUEST_'}{'LASTBLK'};
	}
	elsif($req->{'_REQUEST_'}{'OPCODE'} eq $OPCODES{'WRQ'})
	{
		# WRQ
		printf "block: %u\/%u\n", $req->{'_REQUEST_'}{'LASTBLK'}, $req->{'_REQUEST_'}{'LASTACK'};
	}
}

# create the listener
my $listener = Ethersex::TFTPd->new
    ('RootDir' => $rootdir, 
     'Writable' => 0, 
     'Timeout' => 9999, 
     'CallBack' => \&callback, 
     'debug' => 0, 
     'crypto' => "\x23\x23\x42\x42\x55\x55\x23\x23\x42\x42"
    ) or die Ethersex::TFTPd->error;

printf "TFTP listener is bound to %s:%d.\n", 
    $listener->{'LocalAddr'} ? $listener->{'LocalAddr'} : "'any address'",  
    $listener->{'LocalPort'};

# wait for any request (RRQ or WRQ)
while(my $request = $listener->waitRQ())
{
	# received request
	printf "Received a %s for file '%s'\n", $OPCODES{$request->{'_REQUEST_'}{'OPCODE'}}, $request->{'_REQUEST_'}{'FileName'};

	# process the request
	if($request->processRQ())
	{
		print "OK, transfer completed successfully\n";
	}
	else
	{
		print "Transfer failed.\n";
	}
}
