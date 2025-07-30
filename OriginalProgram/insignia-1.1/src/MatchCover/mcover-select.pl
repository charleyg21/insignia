#!/usr/bin/perl -w

use strict;

if ( scalar (@ARGV) != 3 ) {
    die "Usage: $0 MCOVER MIDX IDFILE\n";
}

my ($cov, $idx, $list) = @ARGV;
my ($id, $id1, $id2, $len, $pos);
my $c;
my %ids;

open (LIST, "<$list");
while (<LIST>) {
    ($id) = split;
    if ( !defined $id ) {
        die "ERROR: Could not read id list\n";
    }

    $ids{$id} = 1;
}
close (LIST);

open (COVER, "<$cov");
open (IDX, "<$idx");

while (<IDX>) {
    ($id1, $id2, $len, $pos) = split;
    if ( !defined $id1 || !defined $id2 || !defined $len || !defined $pos ) {
        die "ERROR: Could not read index file\n";
    }

    if ( $ids{$id1} ) {
        seek (COVER, $pos, 0);

        $_ = <COVER>;
        if ( ! /^>/ ) {
            "ERROR: Bad seek in index\n";
        }

        print;

        for ( $c = 0; $c < $len; ++ $c ) {
            $_ = <COVER>;
            print;
        }
    }
}

close (COVER);
close (IDX);
