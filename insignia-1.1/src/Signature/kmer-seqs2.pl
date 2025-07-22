#!/usr/bin/perl -w

use strict;

if ( scalar (@ARGV) != 3 ) {
    die "Usage: $0 FASTA IDX KMERS\n";
}

my ($fasta, $idx, $kmer) = @ARGV;
my ($id, $tax, $len, $e, $pos, $off, $rec, $k, $r);
my $CPL = 60;
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
        if ( !defined ($rec) ) {
            die "ERROR: Kmer listed before sequence\n";
        }

        $off = $1 - 1;
        $len = $2 + $k - 1;
        $e = $off + $len;

        print "${id}[$1,$e]\t$len\t";

        $r = $CPL;
        for ( my $i = $off; $i < $e; $i += $CPL ) {
            if ( $e - $i < $CPL ) { $r = $e - $i; }
            print substr ($rec, $i, $r), "\n";
        }

    } else {
        die "ERROR: Unrecognized line in k-mer file\n$_\n";
    }
}

close (FASTA);
close (KMER);
