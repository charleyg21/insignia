include $(top_srcdir)/config/insignia.mk

bin_PROGRAMS = \
	common-mer \
	kmer-intersect \
	mcover-intersect \
	mcover-union \
	unique-mer

dist_bin_SCRIPTS = \
	signify.sh \
	insignify.pl \
	kmer-seqs.pl \
	kmer-seqs2.pl \
	kmer-seqs3.pl \
	kmer2fasta.pl


##-- GLOBAL INCLUDE
AM_CPPFLAGS = \
        -I$(top_srcdir)/src/MatchCover


#-- common-mer
common_mer_SOURCES = \
	common-mer.c

#-- kmer-intersect
kmer_intersect_LDADD = \
	$(top_builddir)/src/MatchCover/libmcover.a
kmer_intersect_SOURCES = \
	kmer-intersect.cc

#-- mcover-intersect
mcover_intersect_LDADD = \
	$(top_builddir)/src/MatchCover/libmcover.a
mcover_intersect_SOURCES = \
	mcover-intersect.cc

#-- mcover-union
mcover_union_LDADD = \
	$(top_builddir)/src/MatchCover/libmcover.a
mcover_union_SOURCES = \
	mcover-union.cc

#-- unique-mer
unique_mer_SOURCES = \
	unique-mer.c

