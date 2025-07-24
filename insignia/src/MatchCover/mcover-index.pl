#!/usr/bin/perl -w

use strict;

if ( scalar (@ARGV) != 1 ) {
    die "Usage: $0 MCOVER\n";
}

open (COVER, "<$ARGV[0]");

my ($id1, $id2, $len);
my $pos = tell (COVER);
my $spos = $pos;

while (<COVER>) {
    chomp;

    if ( /^>(\S+)\s+(\S+)$/ ) {
        if ( defined $id1 && defined $id2 ) {
            print "$id1\t$id2\t$len\t$spos\n";
        }

        $len = 0;
        $spos = $pos;
        $id1 = $1;
        $id2 = $2;
    } elsif ( /^\d+\s+\d+$/ ) {
        $len ++;
    } else {
        die "ERROR: Unrecognized line in mcover\n$_\n";
    }

    $pos = tell (COVER);
}

if ( defined $id1 && defined $id2 ) {
    print "$id1\t$id2\t$len\t$spos\n";
}

close (COVER);
