# Ethersex::TFTP.pm
#
# Ethersex::TFTP is based on NET::TFTP by Graham Barr and has the crypto 
# layer included, needed by the ethersex device.
# 
# Copyright (c) 2007 Stefan Siegl <stesie@brokenpipe.de>.
# Copyright (c) 1998,2007 Graham Barr <gbarr@pobox.com>. All rights reserved.
# This program is free software; you can redistribute it and/or
# modify it under the same terms as Perl itself.

package Ethersex::TFTP;

use strict;
use vars qw($VERSION);
use IO::File;

$VERSION = "0.17"; # $Id: TFTP.pm 12 2007-07-18 11:32:42Z gbarr $

sub RRQ	  () { 01 } # read request
sub WRQ	  () { 02 } # write request
sub DATA  () { 03 } # data packet
sub ACK	  () { 04 } # acknowledgement
sub ERROR () { 05 } # error code
sub OACK  () { 06 } # option acknowledgement

my @NAME = qw(. RRQ WRQ DATA ACK ERR OACK);

sub new {
    my $pkg = shift;
    my $host = shift;

    	bless {
		Debug 		=> 0,   	# Debug off
		Timeout 	=> 5,   	# resend after 5 seconds
		Retries 	=> 5,   	# resend max 5 times
		Port 		=> 69,  	# tftp port number
		BlockSize 	=> 0,   	# use default blocksize (512)
		IpMode		=> 'v4',	# Operate in IPv6 mode, off by default
		Mode 		=> 'netascii',	# transfer in netascii
		@_,				# user overrides
		Host 		=> $host,	# the hostname
	}, $pkg;
}

sub timeout {
    my $self = shift;
    my $v = $self->{'Timeout'};
    $self->{'Timeout'} = 0 + shift if @_;
    $v
}

sub debug {
    my $self = shift;
    my $v = $self->{'Debug'};
    $self->{'Debug'} = 0 + shift if @_;
    $v
}

sub port {
    my $self = shift;
    my $v = $self->{'Port'};
    $self->{'Port'} = 0 + shift if @_;
    $v
}

sub retries {
    my $self = shift;
    my $v = $self->{'Retries'};
    $self->{'Retries'} = 0 + shift if @_;
    $v
}

sub block_size {
    my $self = shift;
    my $v = $self->{'BlockSize'};
    $self->{'BlockSize'} = 0 + shift if @_;
    $v
}

sub host {
    my $self = shift;
    my $v = $self->{'Host'};
    $self->{'Host'} = shift if @_;
    $v
}

sub ip_mode {
    my $self = shift;
    my $v = $self->{'IpMode'};
    $self->{'IpMode'} = shift if @_;
    $v
}

sub ascii {
    $_[0]->mode('netascii');
}

sub binary {
    $_[0]->mode('octet');
}

BEGIN {
    *netascii = \&ascii;
    *octet    = \&binary;
}

sub mode {
    my $self = shift;
    my $v = $self->{'Mode'};
    $self->{'Mode'} = lc($_[0]) eq "netascii" ? "netascii" : "octet"
	if @_;
    $v
}

sub error {
    my $self = shift;
    exists $self->{'error'}
	? $self->{'error'}
	: undef;
}

sub get {
    my($self,$remote) = splice(@_,0,2);
    my $local = shift if @_ % 2;
    my %arg = ( %$self, @_ );

    delete $self->{'error'};

    my $io  = Ethersex::TFTP::IO->new($self,\%arg,RRQ,$remote);

    return $io
	unless defined($local) && defined($io);

    my $file = $local;
    unless(ref($local)) {
	unlink($file);
	$local = IO::File->new($file,O_WRONLY|O_TRUNC|O_CREAT);
    }

    binmode $local if $self->{'Mode'} eq 'octet';

    my($len,$pkt);
    while($len = sysread($io,$pkt,10240)) {
	if($len < 0) {
	    $self->{'error'} = $io->error;
	    last;
	}
	elsif(syswrite($local,$pkt,length($pkt)) < 0) {
	    $self->{'error'} = "$!";
	    last;
	}
    }

    close($local)
	unless ref($file);

    $self->{'error'} = $io->error
	unless(close($io));

    exists $self->{'error'} ? undef : 1;
}

sub put {
    my($self,$remote) = splice(@_,0,2);
    my $local;
    ($local,$remote) = ($remote,shift) if @_ %2;
    my %arg = (%$self,@_);

    delete $self->{'error'};

    my $file;
    if (defined $local) {
	$file = $local;
	unless(ref($local)) {
	    unless ($local = IO::File->new($file,O_RDONLY)) {
		$self->{'error'} = "$file: $!";
		return undef;
	    }
	}
    }

    my $io  = Ethersex::TFTP::IO->new($self,\%arg,WRQ,$remote);

    return $io
	unless defined($local) && defined($io);

    binmode $local if $self->{'Mode'} eq 'octet';

    my($len,$pkt);
    while($len = sysread($local,$pkt,10240)) {
	if($len < 0) {
	    $self->{'error'} = "$!";
	    last;
	}
	elsif(($len=syswrite($io,$pkt,length($pkt))) < 0) {
	    $self->{'error'} = $io->error;
	    last;
	}
    }

    close($local)
	unless ref($file);

    $self->{'error'} = $io->error
	unless(close($io));

    exists $self->{'error'} ? undef : 1;
}

package Ethersex::TFTP::IO;

use vars qw(@ISA);
use IO::Socket;
use IO::Select;
use Crypt::Skipjack;
use Crypt::Random qw/ makerandom_octet /;
use Data::Hexdumper;

@ISA = qw(IO::Handle);

sub new {
    my($pkg,$tftp,$opts,$op,$remote) = @_;
    my $io = $pkg->SUPER::new;

    $opts->{'Mode'} = lc($opts->{'Mode'});
    $opts->{'IpMode'} = lc($opts->{'IpMode'});
    $opts->{'Mode'} = "netascii"
	unless $opts->{'Mode'} eq "octet";
    $opts->{'ascii'} = lc($opts->{'Mode'}) eq "netascii";

    my $host = $opts->{'Host'};
    ## jjmb - had to make an adjustment here the logic used originally does not work well
    ##        with IPv6.
    my $port = undef;
    if($opts->{'IpMode'} eq "v6") {
	    require Socket6;
	    require IO::Socket::INET6;
    	$port = $opts->{'Port'};
    } else {
    	$port = $host =~ s/:(\d+)$// ? $1 : $opts->{'Port'};
    }
    my $addr = inet_aton($host);

    ## jjmb - added some logic here for the time being to prevent some errors from showing
    if($opts->{'IpMode'} eq "v6") {
            # Skipping validation
     } else {
	    unless($addr) {
		$tftp->{'error'} = "Bad hostname '$host'";
		return undef;
	    }
    }

    ## jjmb - need to construct different objects depending on the IP version used
    my $sock = undef;
    if($opts->{'IpMode'} eq "v6") {
    	$sock = IO::Socket::INET6->new(PeerAddr => $opts->{'Host'}, PeerPort => $opts->{'Port'}, Proto => 'udp');
    } else {
    	$sock = IO::Socket::INET->new(Proto => 'udp');
    }

    my $mode = $opts->{'Mode'};
    my $pkt  = pack("n a* c a* c", $op, $remote, 0, $mode, 0);

    if($opts->{'BlockSize'} > 0) {
	$pkt .= sprintf("blksize\0%d\0",$opts->{'BlockSize'});
    }

    my $read = $op == Ethersex::TFTP::RRQ;

    my $sel = IO::Select->new($sock);

    @{$opts}{'read','sock','sel','pkt','blksize','crypto'}
	= ($read,$sock,$sel,$pkt,512,$opts->{'Crypto'});

    if($read) { # read
	@{$opts}{'ibuf','icr','blk'} = ('',0,1);
    }
    else { # write
	@{$opts}{'obuf','blk','ack'} = ('',0,-1);
    }

    if($tftp->{'IpMode'} eq "v6") {
    	send($sock,$pkt,0,Socket6::sockaddr_in6($port,Socket6::inet_pton(AF_INET6,$host)));
    } else {
    	send($sock,$pkt,0,pack_sockaddr_in($port,inet_aton($host)));
    }
    _dumppkt($sock,1,$pkt) if $opts->{'Debug'};

    tie *$io, "Ethersex::TFTP::IO",$opts;
    $io;
}

sub error {
    my $self = shift;
    my $tied = UNIVERSAL::isa($self,'GLOB') && tied(*$self) || $self;
    exists $tied->{'error'} ? $tied->{'error'} : undef;
}

sub TIEHANDLE {
    my $pkg = shift;
    bless shift , $pkg;
}

sub PRINT {
    my $self = shift;
    # Simulate print
    my $buf = join(defined($,) ? $, : "",@_) . defined($\) ? $\ : "";

    # and with the proposed ?? syntax that would be 
    # $buf = join($, ?? "", @_) . $\ ?? "";

    $self->WRITE($buf,length($buf));
}

sub WRITE {
    # $self, $buf, $len, $offset
    my $self = shift;
    my $buf = substr($_[0],$_[2] || 0,$_[1]);
    my $offset = 0;

    $buf =~ s/([\n\r])/$1 eq "\n" ? "\015\012" : "\015\0"/soge
	if ($self->{'ascii'});

    $self->{'obuf'} .= substr($buf,$offset);

    while(length($self->{'obuf'}) >= $self->{'blksize'}) {
	return -1 if _write($self,1) < 0;
    }

    $_[1];
}

sub READLINE {
    my $self = shift;

    # return undef (ie eof) unless we have an input buffer
    return undef
	if exists $self->{'error'} || !exists $self->{'ibuf'};

    _read($self,0);

    while(1) {
	my $sep;
	# if $/ is undef then we slurp the whole file
	if(defined($sep = $/)) {
	    # if $/ eq "" then we need to do paragraph mode
	    unless(length($sep)) {
		# when doing paragraph mode remove all leading \n's
		$self->{'ibuf'} =~ s/^\n+//s;
		$sep = "\n\n";
	    }
	    my $offset = index($self->{'ibuf'},$sep);
	    if($offset >= 0) {
		my $len = $offset+length($sep);
		# With 5.005 I could use the 4-arg substr
		my $ret = substr($self->{'ibuf'},0,$len);
		substr($self->{'ibuf'},0,$len) = "";

		return $ret;
	    }
	}

	my $res = _read($self,1);

	next if $res > 0; # We have some more, but do we have enough ?

	if ($res < 0) {
	    # We have encountered an error, so 
	    # force subsequent reads to return eof
	    delete $self->{'ibuf'};

	    # And return undef (ie eof)
	    return undef; 
	}

	# $res == 0 so there is no more data to read, just return
	# the buffer contents
	return delete $self->{'ibuf'};
    }

    # NOT REACHED
    return;
}

sub READ {
    # $self, $buf, $len, $offset

    my $self = shift;

    return undef
	if exists $self->{'error'};

    return 0
	unless exists $self->{'ibuf'};

    my $ret = length($self->{'ibuf'});

    unless ($self->{'eof'}) {
        # If there is any data waiting, read it and ask for more
        _read($self,0);

        #  read until we have enough
        while(($ret = length($self->{'ibuf'})) < $_[1]) {
	    last unless _read($self,1) > 0;
        }
    }

    # Did we encounter an error
    return undef
	if exists $self->{'error'};

    # we may have too much
    $ret = $_[1]
	if $_[1] < $ret;

    # We are simulating read() so we may have to insert into $_[0]
    if($ret) {
	if($_[2]) {
	    substr($_[0],$_[2]) = substr($self->{'ibuf'},0,$ret);
	}
	else {
	    $_[0] = substr($self->{'ibuf'},0,$ret);
	}

	# remove what we placed into $_[0]
	substr($self->{'ibuf'},0,$ret) = "";
    }

    # If we are returning less than what was asked for
    # then the next call must return eof
    delete $self->{'ibuf'}
	if $self->{'eof'} && length($self->{'ibuf'}) == 0 ;

    $ret;
}

sub CLOSE {
    my $self = shift;

    if (exists $self->{'sock'} && !exists $self->{'closing'}) {
	$self->{'closing'} = 1;
	if ($self->{'read'} ) {
	    unless ($self->{'eof'}) {
		my $pkt = pack("nna*c",Ethersex::TFTP::ERROR,0,"Premature close",0);
		_dumppkt($self->{'sock'},1,$pkt) if $self->{'Debug'};
		send($self->{'sock'},$pkt,0,$self->{'peer'})
		    if $self->{'peer'};
	    }
	}
	else {
	    # Clear the buffer
	    unless(exists $self->{'error'}) {
        	while(length($self->{'obuf'}) >= $self->{'blksize'}) {
		    last if _write($self) < 0;
        	}

		# Send the last block
        	$self->{'blksize'} = length($self->{'obuf'});
        	_write($self) unless(exists $self->{'error'});

		# buffer is empty so blksize=1 will ensure I do not send
		# another packet, but just wait for the ACK
        	$self->{'blksize'} = 1;
        	_write($self) unless(exists $self->{'error'});
	    }
	}
	close(delete $self->{'sock'});
    }

    exists $self->{'error'} ? 0 : 1;
}

# _natoha($data,$cr) - Convert netascii -> host text
# updates both input args
sub _natoha {
    use vars qw($buf $cr);
    local *buf = \$_[0];
    local *cr  = \$_[1];
    my $last = substr($buf,-1);
    if($cr) {
	my $ch = ord(substr($buf,0,1));
	if($ch == 012) {		# CR.LF => \n
	    substr($buf,0,1) = "\n";
	}
	elsif($ch == 0) {		# CR.NUL => \r
	    substr($buf,0,1) = "\r";
	}
	else {
	    # Hm, badly formed netascii
	    substr($buf,0,0) = "\015";
	}
    }

    if(ord($last) eq 015) {
	substr($buf,-1) = "";
	$cr = 1;
    }
    else {
	$cr = 0;
    }

    $buf =~ s/\015\0/\r/sg;
    $buf =~ s/\015\012/\n/sg;

    1;
}

sub _abort {
    my $self = shift;
    $self->{'error'} ||= 'Protocol error';
    $self->{'eof'} = 1;
    my $pkt = pack("nna*c",Ethersex::TFTP::ERROR,0,$self->{'error'},0);
    send($self->{'sock'},$pkt,0,$self->{'peer'})
	if exists $self->{'peer'};
    CLOSE($self);
    -1;
}

# _read: The guts of the reading
#
# returns
#   >0 size of data read
#    0 eof
#   <0 error

sub _read {
    my($self,$wait) = @_;

    return -1 if exists $self->{'error'};
    return 0 if $self->{'eof'};

    my $sock    = $self->{'sock'} || return -1;
    my $select  = $self->{'sel'};
    my $timeout = $wait ? $self->{'Timeout'} : 0;
    my $retry   = 0;

    while(1) {
	if($select->can_read($timeout)) {
	    my $ipkt = ''; # will be filled by _recv
	    my($peer,$code,$blk) = _recv($self,$ipkt)
		or return _abort($self);

	    redo unless defined($peer); # do not send ACK to real peer

	    if($code == Ethersex::TFTP::DATA) {
		# If we receive a packet we are not expecting
		# then ACK the last packet again

		if($blk == $self->{'blk'}) {
		    $self->{'blk'} = $blk+1;
		    my $data = substr($ipkt,4);

		    if ($self->{'crypto'} && length($data)) {
			my $cipher = new Crypt::Skipjack $self->{'crypto'};
			my $outdata = "";
			for my $blockno(1..(length($data) - 16) / 8) {
			    $outdata .= 
			      $cipher->decrypt(substr($data, $blockno << 3, 8)) 
			      ^ substr($data, ($blockno - 1) << 3, 8);
			}

                        my $blockno = (length($data) - 8) / 8;
			my $cbcmac = 
			  $cipher->decrypt(substr($data, $blockno << 3, 8))
			  ^ substr($data, ($blockno - 1) << 3, 8);

			die "cbc-mac check failed"
			  if($cbcmac ne "\000\000\000\000\000\000\000\000");

			$data = $outdata;
		    }

		    _natoha($data,$self->{'icr'})
			if($self->{'ascii'});

		    $self->{'ibuf'} .= $data;

		    my $opkt = $self->{'pkt'} = pack("nn", Ethersex::TFTP::ACK,$blk);
		    send($sock,$opkt,0,$peer);

		    _dumppkt($sock,1,$opkt)
			if $self->{'Debug'};

		    $self->{'eof'} = 1
			if ( length($ipkt) < ($self->{'blksize'} + 4) );

		    return length($data);
		}
		elsif($blk < $self->{'blk'}) {
		    redo; # already got this data
		}
	    }
	    elsif($code == Ethersex::TFTP::OACK) {
		my $opkt = $self->{'pkt'} = pack("nn", Ethersex::TFTP::ACK,0);
		send($sock,$opkt,0,$peer);

		_dumppkt($sock,1,$opkt)
		    if $self->{'Debug'};

		return _read($self,$wait);
	    }
	    elsif($code == Ethersex::TFTP::ERROR) {
		$self->{'error'} = substr($ipkt,4);
		$self->{'eof'} = 1;
		CLOSE($self);
		return -1;
	    }

	    return _abort($self);
	}
	
	last unless $wait;
	# Resend last packet, this will re ACK the last data packet
	if($retry++ >= $self->{'Retries'}) {
	    $self->{'error'} = "Transfer Timeout";
	    return _abort($self);
	}

	send($sock,$self->{'pkt'},0,$self->{'peer'})
	  if $self->{'peer'};

	if ($self->{'Debug'}) {
	    print STDERR "${sock} << ---- retry=${retry}\n";
	    _dumppkt($sock,1,$self->{'pkt'});
	}
    }

    # NOT REACHED
}

sub _recv {
    my $self = shift;
    my $sock = $self->{'sock'};
    my $bsize = $self->{'blksize'}+4;
    $bsize += 16 if defined($self->{'crypto'});
    $bsize = 516 if $bsize < 516;
    my $peer = recv($sock,$_[0],$bsize,0);

    # There is something on the socket, but not a udp packet. Prob. an icmp.
    return unless ($peer);

    _dumppkt($sock,0,$_[0]) if $self->{'Debug'};

    # The struct in $peer can be bigger than needed for AF_INET
    # so could contain garbage at the end. unpacking and re-packing
    # will ensure it is zero filled (Thanks TomC)
    if($self->{'IpMode'} eq "v6") {
    	$peer = Socket6::pack_sockaddr_in6(Socket6::unpack_sockaddr_in6($peer));
    } else {
    	$peer = pack_sockaddr_in(unpack_sockaddr_in($peer));
    }

    $self->{'peer'} ||= $peer; # Remember first peer

    my($code,$blk) = unpack("nn",$_[0]);

    if($code == Ethersex::TFTP::OACK) {
	my %o = split("\0",substr($_[0],2));
	%$self = (%$self,%o);
    }

    if ($self->{'peer'} ne $peer) {
	# All packets must be from same peer
	# packet from someone else, send them an ERR packet
	my $err = pack("nna*c",Ethersex::TFTP::ERROR, 5, "Unknown transfer ID",0);
	_dumppkt($sock,1,$err)
	    if $self->{'Debug'};
	send($sock,$err,0,$peer);

	$peer = undef;
    }

    ($peer,$code,$blk);
}

sub _send_data {
    my $self = shift;

    if(length($self->{'obuf'}) >= $self->{'blksize'}) {
	my $blk = ++$self->{'blk'};
        my $opkt = substr($self->{'obuf'},0,$self->{'blksize'});

	if($self->{'crypto'}) {
	    my $cipher = new Crypt::Skipjack $self->{'crypto'};

            # pad the block with 0xFF chars (AVR nop instruction)
	    if(my $pad = length($opkt) % 8) {
	        $pad = 8 - $pad;
		$opkt .= "\377" x $pad;
	    }

            # prepend IV to outbound packet
	    $opkt = makerandom_octet(Length => 8, Strength => 0) . $opkt;
	    
	    for my $blockno(1..(length($opkt) - 8) / 8) {
	        # we use $cipher->decrypt so the ethersex can use encrypt
		# to actually decrypt the data; mainly to save ram there.
	        substr($opkt, $blockno << 3, 8) =
		  $cipher->decrypt(substr($opkt, $blockno << 3, 8)
		                   ^ substr($opkt, ($blockno - 1) << 3, 8));
	    }

	    $opkt .= $cipher->decrypt(substr($opkt, length($opkt) - 8, 8));
	}

	$opkt = $self->{'pkt'} = pack("nn", Ethersex::TFTP::DATA,$blk) . $opkt;
	substr($self->{'obuf'},0,$self->{'blksize'}) = '';

	my $sock = $self->{'sock'};
	send($sock,$opkt,0,$self->{'peer'});

	_dumppkt($sock,1,$opkt)
	    if $self->{'Debug'};
    }
    elsif (length($self->{'obuf'}) == 0 and $self->{'blksize'} == 1) {
	# ignore
    }
    elsif($^W) {
	require Carp;
	Carp::carp("Ethersex::TFTP: Buffer underflow");
    }

    1;
}

sub _write {
    my($self) = @_;

    return -1 if exists $self->{'error'};

    my $sock    = $self->{'sock'} || return -1;
    my $select  = $self->{'sel'};
    my $timeout = $self->{'Timeout'};
    my $retry   = 0;

    return _send_data($self)
	if $self->{'ack'} == $self->{'blk'};

    while(1) {
	if($select->can_read($timeout)) {
	    my $ipkt=''; # will be filled by _recv
	    my($peer,$code,$blk) = _recv($self,$ipkt)
		or return _abort($self);

	    redo unless defined($peer); # do not send ACK to real peer

	    if($code == Ethersex::TFTP::OACK) {
		$code = Ethersex::TFTP::ACK;
		$blk = 0;
	    }

	    if($code == Ethersex::TFTP::ACK) {
		if ($self->{'blk'} == $blk) {
		    $self->{'ack'} = $blk;
		    return _send_data($self);
		}
		elsif ($self->{'blk'} > $blk) {
		    redo; # duplicate ACK
		}
	    }

	    if($code == Ethersex::TFTP::ERROR) {
		$self->{'error'} = substr($ipkt,4);
		CLOSE($self);
		return -1;
	    }

	    return _abort($self);
	}

	# Resend last packet, this will resend the last DATA packet
	if($retry++ >= $self->{'Retries'}) {
	    $self->{'error'} = "Transfer Timeout";
	    return _abort($self);
	}
	send($sock,$self->{'pkt'},0,$self->{'peer'});

	if ($self->{'Debug'}) {
	    print STDERR "${sock} << ---- retry=${retry}\n";
	    _dumppkt($sock,1,$self->{'pkt'});
	}
    }
    # NOT REACHED
}

sub _dumppkt {
    my($sock,$send) = @_; 
    my($code,$blk) = unpack("nn",$_[2]);
    $send = $send ? "$sock <<" : "$sock >>";
    my $str = sprintf "%s %-4s",$send,$NAME[$code];
    $str .= sprintf " %s=%d",$code == Ethersex::TFTP::ERROR ? "code" : "blk",$blk
	if $code == Ethersex::TFTP::DATA
	   || $code == Ethersex::TFTP::ACK
	   || $code == Ethersex::TFTP::ERROR;

    printf STDERR "%s length=%d\n",$str,length($_[2]);
    if($code == Ethersex::TFTP::RRQ || $code == Ethersex::TFTP::WRQ || $code == Ethersex::TFTP::OACK) {
	my @a = split("\0",substr($_[2],2));
	printf STDERR "%s      filename=%s mode=%s\n",$send,splice(@a,0,2)
		unless $code == Ethersex::TFTP::OACK;
	my %a = @a;
	my($k,$v);
	while(($k,$v) = each %a) {
	    printf STDERR "%s      %s=%s\n",$send,$k,$v;
	}

    }
    printf STDERR "%s      %s\n",$send,substr($_[2],4)
	if $code == Ethersex::TFTP::ERROR;
}

1;

__END__

=head1 NAME

Ethersex::TFTP - TFTP Client class

=head1 SYNOPSIS

    use Ethersex::TFTP;

    $tftp = Ethersex::TFTP->new("some.host.name", BlockSize => 1024);

    $tftp->ascii;

    $tftp->get("remotefile", "localfile");

    $tftp->get("remotefile", \*STDOUT);

    $fh = $tftp->get("remotefile");

    $tftp->binary;

    $tftp->put("localfile", "remotefile");

    $tftp->put(\*STDOUT, "remotefile");

    $fh = $tftp->put("remotefile");

    $err = $tftp->error

=head1 DESCRIPTION

C<Ethersex::TFTP> is a class implementing a simple I<Trivial File Transfer Protocol>
client in Perl as described in RFC1350. C<Ethersex::TFTP> also supports the
TFTP Option Extension (as described in RFC2347), with the following options

 RFC2348 Blocksize Option

=head1 CONSTRUCTOR

=over 4

=item new ( [ HOST ] [, OPTIONS ])

Create a new Ethersex::TFTP object where HOST is the default host to connect
to and OPTIONS are the default transfer options. Valid options are

 Option     Description                                           Default
 ------     -----------                                           -------
 Timeout    Timeout in seconds before retry	                     5
 Retries    Maximum number of retries				     5
 Port	    Port to send data to                                    69
 Mode	    Mode to transfer data in, "octet" or "netascii"     "netascii"
 BlockSize  Negotiate size of blocks to use in the transfer        512
 IpMode	    Indicates whether to operate in IPv6 mode		   "v4"

=back

=head1 METHODS

=over 4

=item get ( REMOTE_FILE [, LOCAL ] [, OPTIONS ])

Get REMOTE_FILE from the server. OPTIONS can be any that are accepted by
C<new> plus the following

  Host    Override default host

If the LOCAL option is missing the get will return a filehandle. This
filehandle must be read ASAP as the server will otherwise timeout.

If the LOCAL option is given then it can be a file name or a reference.
If it is a reference it is assumed to be a reference that is valid as a
filehandle. C<get> will return I<true> if the transfer is successful and
I<undef> otherwise.

Valid filehandles are

=over 4

=item *

A sub-class of IO::Handle

=item *

A tied filehandle

=item *

A GLOB reference (eg C<\*STDOUT>)

=back

=item put ( [ LOCAL, ] REMOTE_FILE [, OPTIONS])

Put a file to the server as REMOTE_FILE. OPTIONS can be any that are
accepted by C<new> plus the following

  Host    Override default host

If the LOCAL option is missing the put will return a filehandle. This
filehandle must be written to ASAP as the server will otherwise timeout.

If the LOCAL option is given then it can be a file name or a reference.
If it is a reference it is assumed to be a valid filehandle as described above.
C<put> will return I<true> if the transfer is successful and I<undef> otherwise.

=item error

If there was an error then this method will return an error string.

=item host ( [ HOST ] )

=item timeout ( [ TIMEOUT ] )

=item port ( [ PORT ] )

=item mode ( [ MODE ] )

=item retries ( [ VALUE ] )

=item block_size ( [ VALUE ] )

=item debug ( [ VALUE ] )

Set or get the values for the various options. If an argument is passed
then a new value is set for that option and the previous value returned.
If no value is passed then the current value is returned.

=item ip_mode ( [ VALUE ] )

Set or get which verion of IP to use ("v4" or "v6")

=item ascii

=item netascii

Set the transfer mode to C<"netascii">

=item binary

=item octet

Set the transfer mode to C<"octet">

=back

=head1 AUTHOR

Graham Barr <gbarr@pobox.com>

=head1 COPYRIGHT

Copyright (c) 1998,2007 Graham Barr. All rights reserved.
This program is free software; you can redistribute it and/or modify
it under the same terms as Perl itself.

=cut
