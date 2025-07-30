#!/usr/bin/perl -w

use strict;
my $LINE = 60;

if ( scalar (@ARGV) != 0 ) {
    die "Usage: $0 < FASTA\n";
}

while (<>) {

    if ( /^>/ ) {
        s/^>\s*/>/;
        print;
        next;
    }

    s/[^[:graph:]]//g;
    
    my $rec = $_;
    my $end = length $rec;

    if ( $end > 0 ) {

       my $l = $LINE;
       for ( my $i = 0; $i < $end; $i += $LINE ) {
           if ( $end - $i < $LINE ) { $l = $end - $i; }
           print substr ($rec, $i, $l), "\n";
       }

    }
}
