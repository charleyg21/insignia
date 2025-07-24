include $(top_srcdir)/config/insignia.mk

bin_PROGRAMS = \
	mcover \
	mcover-gi

dist_bin_SCRIPTS = \
	mcover-build.sh \
	mcover-grid.sh \
	mcover-index.pl \
	mcover-select.pl

noinst_LIBRARIES = \
	libmcover.a

noinst_HEADERS = \
	mcover.hh


#-- mcover
mcover_gi_LDADD = \
	$(top_builddir)/src/MatchCover/libmcover.a
mcover_gi_SOURCES = \
	mcover-gi.cc

#-- mcover
mcover_LDADD = \
	$(top_builddir)/src/MatchCover/libmcover.a
mcover_SOURCES = \
	mcover-main.cc

#-- libmcover.a
libmcover_a_SOURCES = \
	mcover.cc

