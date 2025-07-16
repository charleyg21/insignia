#!/usr/bin/perl -w

if ( scalar(@ARGV) != 4 ) {
    die "Usage: $0 FASTA IDX KMERS MAXGAP\n";
}

my ($fasta, $idx, $kmer, $maxgap) = @ARGV;
my $CPL = 60;
my %index;

open (IDX, "<$idx");
while(<IDX>)
  {
    ($id, $tax, $len, $pos) = split;
    $index{$id} = [$tax, $len, $pos];
  }
close(IDX);


open(KMER, "<$kmer");
open(FASTA, "<$fasta");

while (<KMER>)
  {
    if ( /^>(\S+)\s+(\d+)/ )
      {
        dumplast();

        $id = $1;
        $k = $2;
        if ( !defined ($index{$id}) )
          {
            die "ERROR: FastA index does not contain $id\n";
          }

        seek (FASTA, $index{$id}[2], 0);

        $_ = <FASTA>;
        if ( !/^>/ )
          {
            die "ERROR: Bad seek in index\n";
          }

        undef $rec;
        while (<FASTA>)
          {
            chomp;
            if ( /^>/ )
              {
                last;
              }
            $rec .= $_;
          }
      }
    elsif ( /^(\d+)\s+(\d+)$/ )
      {
        if ( !defined($rec) )
          {
            die "ERROR: Kmer listed before sequence\n";
          }

        $b = $1;
        $nk = $2;
        $e = $b + $nk - 1;

        if ( defined($Gb) && $b - $Ge - 1 <= $maxgap )
          {
            push @Gk, [$Ge+1-$Gb, $b-1-$Gb];
            $Ge = $e;
          }
        else
          {
            dumplast();
            $Gb = $b;
            $Ge = $e;
          }
      }
    else
      {
        die "ERROR: Unrecognized line in k-mer file\n$_\n";
      }
  }
dumplast();

close (FASTA);
close (KMER);


sub dumplast
  {
    if ( defined($Gb) )
      {
        my $e = $Ge + $k - 1;
        my $len = $e - $Gb + 1;
        my $seq = uc(substr($rec, $Gb-1, $len));
        foreach $gk (@Gk)
          {
            my $bads = $gk->[0];
            my $bade = $gk->[1] + $k - 1;
            my $badl = $bade - $bads + 1;
            substr($seq, $bads, $badl) = lc(substr($seq, $bads, $badl));
          }

        my $ucseq = $seq;
        $ucseq =~ s/[a-z]//g;
        my $ucnt = length($ucseq);

        print "$id\t$Gb\t$e\t$len\t$ucnt\t$seq\n";
      }
    $Gb = undef;
    $Ge = undef;
    @Gk = ();
  }
