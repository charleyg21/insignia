##-- INSIGNIA CONFIGURE --##

# initialize
AC_PREREQ(2.59)
AC_INIT([Insignia],
        [1.1],
        [amp@umiacs.umd.edu],
        [insignia])
AC_PREFIX_DEFAULT([`pwd`])
AC_CONFIG_SRCDIR([src/MatchCover/mcover.hh])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE(1.8)

# declare some precious variables
AC_ARG_VAR(PERL, [perl command])
AC_ARG_VAR(MUMMER, [mummer command])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB
AC_PATH_PROG(PERL, [perl], [:])
AC_PATH_PROG(MUMMER, [mummer], [:])

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(
   [ \
   stdlib.h \
   string.h \
   unistd.h \
   ],
   [],
   AC_MSG_ERROR([required header not found please use a supported compiler])
)

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_SYS_LARGEFILE
AC_FUNC_FSEEKO

# Checks for library functions.
AC_CHECK_FUNCS(
   [ \
   strdup \
   ],
   [],
   AC_MSG_ERROR([required function not found please use a supported compiler])
)

# check the platform
AC_CANONICAL_HOST

# makefiles to configure
AC_CONFIG_FILES(
   [ \
   Makefile \
   src/Makefile \
   src/FastaTools/Makefile \
   src/MatchCover/Makefile \
   src/Signature/Makefile
   ]
)

# make it happen
AC_OUTPUT

# dump some configuration confirmations
echo \
"
-- ${PACKAGE_STRING} Configuration Results --
  C compiler:          ${CC} ${CFLAGS}
  C++ compiler:        ${CXX} ${CXXFLAGS}"

if test x"${GCC}" = x"yes" ; then
   gcc_version=`${CC} --version | head -n 1`
   echo "  GCC version:         ${gcc_version}"
else
   gcc_version=''
fi

echo \
"  Host System type:    ${host}
  Install prefix:      ${prefix}
  Install eprefix:     ${exec_prefix}

  See config.h for further configuration information.
  Email <${PACKAGE_BUGREPORT}> with questions and bug reports.
"


if test x"${PERL}" = x":"  ||  ! test -x "${PERL}"; then
  echo "WARNING! perl was not found but is required to run Insignia"
  echo "   install perl or point configure to the correct location"
fi
 
if test x"${MUMMER}" = x":"  ||  ! test -x "${MUMMER}"; then
  echo "WARNING! mummer was not found but is required to run Insignia"
  echo "   install mummer or point configure to the correct location"
fi

