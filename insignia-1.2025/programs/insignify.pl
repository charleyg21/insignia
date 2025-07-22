#!/usr/bin/perl -w
use strict;
use File::Temp qw(tempfile);
use File::Spec qw(tmpdir);
my $BINDIR=".";

if ( scalar (@ARGV) != 4 ) {
    die "Usage: $0 DBPATH MERLEN TGT BGX\n";
}

#-- Vars
my ($opt_db, $opt_len, $opt_tgt, $opt_bgx) = @ARGV;
my $refOID;
my %tgtOIDs;
my %bgxOIDs;
my $oid;
my @A;

#-- Temporary files
my $tmpdir = File::Spec->tmpdir();
my ($refFh, $refFn) = tempfile("tmpsig.XXXXX",SUFFIX=>".ref",DIR=>$tmpdir);
my ($tgtFh, $tgtFn) = tempfile("tmpsig.XXXXX",SUFFIX=>".tgt",DIR=>$tmpdir);
my ($bgxFh, $bgxFn) = tempfile("tmpsig.XXXXX",SUFFIX=>".bgx",DIR=>$tmpdir);
my ($uniFn, $uniFn) = tempfile("tmpsig.XXXXX",SUFFIX=>".uni",DIR=>$tmpdir);
my ($shrFn, $shrFn) = tempfile("tmpsig.XXXXX",SUFFIX=>".shr",DIR=>$tmpdir);

#-- Read organism ID input
open(TGT,"<$opt_tgt") or die "Error: Could not open $opt_tgt, $!\n";
while(<TGT>) {
    ($oid) = split;
    if ( $oid ) {
        if ( !$refOID ) {
            $refOID = $oid;
        }
        $tgtOIDs{$oid}++;
    }
}
close(TGT);

open(BGX,"<$opt_bgx") or die "Error: Could not open $opt_bgx, $!\n";
while(<BGX>) {
    ($oid) = split;
    if ( $oid ) {
        $bgxOIDs{$oid}++;
    } elsif ( $oid eq "0" ) {
        #-- OID of 0 is code for exclude all of RefSeq
        print $bgxFh "gi|*|\t0\t0\t0\n";
    }
}
close(BGX);

#-- Make sequence ID input
my $idxFn = "${opt_db}/db_seq/t.idx";
open(IDX,"<$idxFn") or die "Error: Could not open $idxFn, $!";
while(<IDX>) {
    @A = split;
next if ( scalar(@A) != 4 );
if ( $refOID == $A[1] ) {
        print $refFh $_;
    }
    if ( exists($tgtOIDs{$A[1]}) ) {
        print $tgtFh $_;
        print $bgxFh $_; # targets need to be added to bgx
    }
    if ( exists($bgxOIDs{$A[1]}) ) {
        print $bgxFh $_;
    }
}
close(IDX);

#-- Close files, it's the shell's turn
close($refFn);
close($tgtFn);
close($bgxFn);
close($uniFn);
close($shrFn);

#-- Generate signatures
my $covFn = "${opt_db}/db_cov/${refOID}.cov";

#-- Do it in parallel
my $pid1 = fork();
if ( !$pid1 ) {
    system("$BINDIR/mcover-union -T $refFn -X $bgxFn $covFn | $BINDIR/unique-mer -k $opt_len > $uniFn");
    if ( $pid1 == 0 ) { exit(0); }
}
my $pid2 = fork();
if ( !$pid2 ) {
    system("$BINDIR/mcover-intersect -T $refFn -B $tgtFn $covFn | $BINDIR/common-mer -k $opt_len > $shrFn");
    if ( $pid2 == 0 ) { exit(0); }
}

#-- Wait for forks
waitpid($pid1,0);
waitpid($pid2,0);

#-- Finish up
system("$BINDIR/kmer-intersect $uniFn $shrFn");

#-- Clean up
unlink($refFn,$tgtFn,$bgxFn,$uniFn,$shrFn);
