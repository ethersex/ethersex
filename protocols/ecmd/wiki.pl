#! /usr/bin/perl -w
use strict;
my $default_block = "Miscelleanous";

# block([[Am_Puls_der_Zeit|Clock]])
# ecmd_feature(time, "time",, Display the current time in seconds since January 1st 1970.)

my %data;
my $current_block = $default_block;
while(<>) {
  if(m/block\((.+)\)/) { $current_block = $1; }
  elsif(m/ecmd_feature\([^,]+,\s*"([^"]+)"\s*,([^,]*),(.*)\)/) {
    $data{$current_block}->{$1} = { 'args' => $2, 'help' => $3 };
  }
}

print<<FNORD
<div class="errorbox">
This page is automatically generated from the files in the Ethersex source code
repository.  Do not edit this page but send [[patches]] for those files!
</div>

__NOTOC__
FNORD
 ;

foreach my $key (sort (keys %data)) {
  next if ($key eq $default_block);
  if(scalar(keys %{$data{$key}}) == 1) {
    foreach (keys %{$data{$key}}) {
      $data{$default_block}{$_} = $data{$key}{$_};
    }
    next;
  }
  &write_block($key);
}

&write_block($default_block);
sub write_block($) {
  my $block = shift @_;
  my %entries = %{$data{$block}};

  print "== $block ==\n{| border='1'\n| ''Command syntax''\n| ''Short description''\n|-\n";

  foreach(sort (keys %entries)) {
    printf "| %s %s\n| %s\n|-\n", $_, $entries{$_}->{'args'}, $entries{$_}->{'help'};
  }

  print "|}\n";
}

print<<FNORD

[[Category:Ethersex]]
[[Category:ECMD]]
FNORD
 ;
