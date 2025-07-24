#!/usr/bin/perl -w

use strict;

if ( scalar (@ARGV) != 3 ) {
    die "Usage: $0 FASTA IDX KMERS\n";
}

my ($fasta, $idx, $kmer) = @ARGV;
my ($id, $tax, $len, $pos, $off, $rec, $k);
my %index;

open (IDX, "<$idx");
while (<IDX>) {
    ($id, $tax, $len, $pos) = split;
    $index{$id} = [$tax, $len, $pos];
}
close (IDX);


open (KMER, "<$kmer");
open (FASTA, "<$fasta");

while ( <KMER> ) {

    if ( /^>(\S+)\s+(\d+)/ ) {
        $id = $1;
        $k = $2;
        if ( !defined ($index{$id}) ) {
            die "ERROR: FastA index does not contain $id\n";
        }

        print;

        seek (FASTA, $index{$id}[2], 0);
        
        $_ = <FASTA>;
        if ( !/^>/ ) {
            die "ERROR: Bad seek in index\n";
        }

        undef $rec;
        while (<FASTA>) {
            chomp;
            if ( /^>/ ) { last; }
            $rec .= $_;
        }
    } elsif ( /^(\d+)\s+(\d+)$/ ) {
        chomp;
        print;

        $off = $1 - 1;
        $len = $2 + $k - 1;

        if ( !defined ($rec) ) {
            die "ERROR: Kmer listed before sequence\n";
        }

        print "\t", substr ($rec, $off, $len), "\n";
    } else {
        die "ERROR: Unrecognized line in k-mer file\n$_\n";
    }
}

close (FASTA);
close (KMER);
