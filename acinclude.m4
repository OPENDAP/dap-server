
# m4 macros from the Unidata netcdf 2.3.2 pl4 distribution. Modified for use
# with GNU Autoconf 2.1. I renamed these from UC_* to DODS_* so that there
# will not be confusion when porting future versions of netcdf into the DODS
# source distribution. Unidata, Inc. wrote the text of these macros and holds
# a copyright on them.
#
# jhrg 3/27/95
#
# Added some of my own macros (don't blame Unidata for them!) starting with
# DODS_PROG_LEX and down in the file. jhrg 2/11/96
#
# I've added a table of contents for this file. jhrg 2/2/98
# 
# 1. Unidata-derived macros. 
# 2. Macros for finding libraries used by the core software.
# 3. Macros used to test things about the compiler
# 4. Macros for locating various systems (Matlab, etc.)
# 5. Macros used to test things about the computer/OS/hardware
#
# $Id: acinclude.m4,v 1.53 1999/07/30 20:01:50 jimg Exp $

# 1. Unidata's macros
#-------------------------------------------------------------------------

# Check for fill value usage.

AC_DEFUN(DODS_FILLVALUES, [dnl
    AC_MSG_CHECKING(for fill value usage)
    if test "${OLD_FILLVALUES-}" = "yes"
    then
        OLD_FILLVALUES=1
	AC_DEFINE(OLD_FILLVALUES)
	AC_MSG_RESULT(using old fill values)
    else
	OLD_FILLVALUES=0
	AC_MSG_RESULT(not using old fill values)
    fi
    AC_SUBST(OLD_FILLVALUES)])

# Check endianness. This program returns true on a little endian machine,
# false otherwise.

ENDIAN_CHECK_PRG="main() {
    long i = 0;
    unsigned char *ip = (unsigned char *)&i;
    *ip	= 0xff;
    /* i == 0xff if little endian or if sizeof(long)==sizeof(char) */
    exit(i == 0xff ? 0: 1);
}"

AC_DEFUN(DODS_SWAP, [dnl
    AC_REQUIRE([AC_CANONICAL_HOST])
    AC_MSG_CHECKING(endianess)
    AC_TRY_RUN([${ENDIAN_CHECK_PRG}], [SWAP=-DSWAP], [SWAP=""], [dnl
	case "$host" in
	    i386* | dec*) SWAP=-DSWAP;;
	    *) SWAP="";;
	esac])

    dnl Look for the endian.h header. If it is present, *don't* define 
    dnl BIG_ENDIAN or LITTLE_ENDIAN since one of those will be defined there.
    dnl 5/13/99 jhrg
    AC_CHECK_HEADER(endian.h, found=1, found=0)
    if test $found -eq 0
    then
	if test -z "$SWAP"
	then
	    AC_DEFINE(BIG_ENDIAN)
	    AC_MSG_RESULT(big endian)
	else
	    AC_DEFINE(LITTLE_ENDIAN)
	    AC_MSG_RESULT(little endian)
	fi
    fi
    AC_SUBST(SWAP)])

# Check type of 32-bit `network long' integer.

NETLONG_CHECK_PGM="main() {exit(sizeof(long) == 4 ? 0: 1);}"

AC_DEFUN(DODS_NETLONG, [dnl
    AC_REQUIRE([AC_CANONICAL_HOST])
    AC_MSG_CHECKING(net long type)
    AC_TRY_RUN([${NETLONG_CHECK_PGM}], [NETLONG=""], 
	       [NETLONG='-DNETLONG=int'], [dnl
	case "$host" in
            *alpha*) NETLONG='-DNETLONG=int';;
	    *) NETLONG="";;
        esac])
   if test -z "$NETLONG"
   then
	AC_MSG_RESULT(long)
   else
	AC_MSG_RESULT(int)
   fi
   AC_SUBST(NETLONG)])

# Set the value of a variable.  Use the environment if possible; otherwise
# set it to a default value.  Call the substitute routine.

AC_DEFUN(DODS_DEFAULT, [$1=${$1-"$2"}; AC_SUBST([$1])])

# 2. Finding libraries
#--------------------------------------------------------------------------

# This macro does things that the other library-finding macros rely on. 
# It must be run before the other library macros. jhrg 2/2/98

# Define the C preprocessor symbol `DODS_ROOT' to be the full path to the
# top of the DODS tree (e.g., /usr/local/DODS-2.15). Also substitute that 
# string for @dods_root@ and set it to the shell variable $dods_root. Thus
# Makefile.in files can use the path as can configure.in files.

AC_DEFUN(DODS_GET_DODS_ROOT, [dnl
    fullpath=`pwd`
    dods_root=`echo $fullpath | sed 's@\(.*DODS[[-.0-9a-z]]*\).*@\1@'`
echo "dods root: $dods_root"
    AC_DEFINE_UNQUOTED(DODS_ROOT, "$dods_root")
    AC_SUBST(dods_root)])

# Check for the existence of the -lsocket and -lnsl libraries.
# The order here is important, so that they end up in the right
# order in the command line generated by make.  Here are some
# special considerations:
# 1. Use "connect" and "accept" to check for -lsocket, and
#    "gethostbyname" to check for -lnsl.
# 2. Use each function name only once:  can't redo a check because
#    autoconf caches the results of the last check and won't redo it.
# 3. Use -lnsl and -lsocket only if they supply procedures that
#    aren't already present in the normal libraries.  This is because
#    IRIX 5.2 has libraries, but they aren't needed and they're
#    bogus:  they goof up name resolution if used.
# 4. On some SVR4 systems, can't use -lsocket without -lnsl too.
#    To get around this problem, check for both libraries together
#    if -lsocket doesn't work by itself.
#
# From Tcl7.6 configure.in. jhrg 11/18/96

AC_DEFUN(DODS_LIBS, [dnl
    tcl_checkBoth=0
    AC_CHECK_FUNC(connect, tcl_checkSocket=0, tcl_checkSocket=1)
    if test "$tcl_checkSocket" = 1; then
	AC_CHECK_LIB(socket, main, LIBS="$LIBS -lsocket", tcl_checkBoth=1)
    fi
    if test "$tcl_checkBoth" = 1; then
	tk_oldLibs=$LIBS
	LIBS="$LIBS -lsocket -lnsl"
	AC_CHECK_FUNC(accept, tcl_checkNsl=0, [LIBS=$tk_oldLibs])
    fi
    AC_CHECK_FUNC(gethostbyname, , AC_CHECK_LIB(nsl, main, 
		  [LIBS="$LIBS -lnsl"]))])

AC_DEFUN(DODS_FIND_PACKAGES_DIR, [dnl
    AC_MSG_CHECKING("for the packages directory")
    # Where does DODS live?
    AC_REQUIRE([DODS_GET_DODS_ROOT])
    DODS_PACKAGES_DIR=`ls -1d $dods_root/packages-*`
    AC_MSG_RESULT("found it at $DODS_PACKAGES_DIR")
    AC_SUBST(DODS_PACKAGES_DIR)])

AC_DEFUN(DODS_PACKAGES_SUPPORT, [dnl
    # Where does DODS live?
    AC_REQUIRE([DODS_GET_DODS_ROOT])
    # Find a good C compiler (hopefully gcc).
    AC_REQUIRE([AC_PROG_CC])
    # Find out about -lns and -lsocket
    AC_REQUIRE([DODS_LIBS])
    # Find the full name of the packages directory
    AC_REQUIRE([DODS_FIND_PACKAGES_DIR])
    # Assume that we always search the packages/lib directory for libraries.
    LDFLAGS="$LDFLAGS -L$DODS_PACKAGES_DIR/lib"
    # Assume that we can always search packages/include directory for include 
    # files. 
    INCS="$INCS -I$DODS_PACKAGES_DIR/include"
    # Initialize $packages to null.
    packages=""
    AC_SUBST(packages)])

AC_DEFUN(DODS_COMPRESSION_LIB, [dnl
    AC_REQUIRE([DODS_PACKAGES_SUPPORT])
    AC_CHECK_LIB(z, zlibVersion,
		 HAVE_Z=1; LIBS="$LIBS -lz",
		 packages="$packages libz"; HAVE_Z=1; LIBS="$LIBS -lz")
    AC_SUBST(packages)])

AC_DEFUN(DODS_RX_LIB, [dnl
    AC_REQUIRE([DODS_PACKAGES_SUPPORT])
    AC_CHECK_LIB(rx, rx_version_string,
		 HAVE_RX=1; LIBS="$LIBS -lrx",
		 packages="$packages libz"; HAVE_RX=1; LIBS="$LIBS -lrx")
    AC_SUBST(packages)])

# Look for the web library. Then look for the include files. If the library
# cannot be found, then build the version in packages. jhrg 2/3/98

AC_DEFUN(DODS_WWW_LIB, [dnl
    AC_REQUIRE([DODS_PACKAGES_SUPPORT])
    DODS_FIND_WWW_ROOT
    AC_CHECK_LIB(www, HTLibInit,
		 HAVE_WWW=1; LIBS="-lwww $LIBS",
		 packages="$packages libwww"; HAVE_WWW=1; LIBS="-lwww $LIBS")
    AC_SUBST(packages)])

# Because the www library is now included in the DODS_ROOT/packages-*/ 
# directory, look there for the include files. Users can specify a 
# different directory using --with-www. jhrg 2/4/98

# This used to be DODS_WWW_ROOT, but it is no longer called directly in
# configure.in files. Instead use DODS_WWW_LIB. jhrg 2/3/98

AC_DEFUN(DODS_FIND_WWW_ROOT, [dnl
    AC_REQUIRE([DODS_PACKAGES_SUPPORT])

    AC_ARG_WITH(www,
	[  --with-www=DIR          Directory containing the W3C header files],
	WWW_ROOT=${withval}, WWW_ROOT=$DODS_PACKAGES_DIR/include/w3c)

    AC_SUBST(WWW_ROOT)
    INCS="$INCS -I\$(WWW_ROOT)"
    AC_SUBST(INCS)
    AC_MSG_RESULT(Set the WWW header directory to $WWW_ROOT)])

# Note that this macro looks for Tcl in addition to Expect since expect 
# requires tcl. 2/3/98 jhrg

AC_DEFUN(DODS_EXPECT_LIB, [dnl
    AC_REQUIRE([DODS_PACKAGES_SUPPORT])

    # Use the path supplied using --with if given.
    AC_ARG_WITH(expect,
        [  --with-expect=ARG       What is the Expect prefix directory],
        EXPECT_PATH=${withval}, EXPECT_PATH="")

    if test -n "$EXPECT_PATH"
    then
      	INCS="$INCS -I${EXCEPT_PATH}/include"
      	LDFLAGS="$LDFLAGS -L${EXCEPT_PATH}/lib"
      	AC_MSG_RESULT("Set the Expect root directory to $EXPECT_PATH")
    fi

    # Look for the tcl library. Note that we have to check for both SYSV and 
    # SunOS 4 style version numbers. 
    AC_CHECK_LIB(tcl7.6, Tcl_SetPanicProc, HAVE_TCL=1; tcl=tcl7.6, HAVE_TCL=0)
    if test $HAVE_TCL -eq 0; then
    	AC_CHECK_LIB(tcl76, Tcl_SetPanicProc, HAVE_TCL=1; tcl=tcl76, 
		     HAVE_TCL=0)
    fi

    # Look for expect
    AC_CHECK_LIB(expect5.21, Expect_Init,
		 HAVE_EXPECT=1; expect=expect5.21,
		 HAVE_EXPECT=0, -l${tcl} -ldl -lm)
    if test $HAVE_EXPECT -eq 0; then
    	AC_CHECK_LIB(expect521, Expect_Init,
		     HAVE_EXPECT=1; expect=expect521,
		     HAVE_EXPECT=0, -l${tcl})
    fi

    # Now set up LIBS
    if test $HAVE_EXPECT -eq 1; then
	LIBS="$LIBS -l${expect}"
    else
	packages="$packages libexpect"
    fi
    if test $HAVE_TCL -eq 1; then
	LIBS="$LIBS -l${tcl}"
    else
	packages="$packages libtcl"
    fi

    # Part two: Once we have found expect (and tcl), locate the tcl include
    # directory. Assume that all the tcl includes live where tclRegexp.h
    # does.
    AC_CHECK_HEADER(tclRegexp.h, found=1, found=0)

    # Look some other places if not in the standard ones.
    if test $found -eq 0
    then
    	tcl_include_paths="/usr/local/src/tcl7.6/generic \
		       $EXCEPT_PATH/src/tcl7.6/generic \
		       $DODS_PACKAGES_DIR/src/tcl7.6/generic"

	AC_MSG_CHECKING(for tclRegex.h in some more places)

	for d in $tcl_include_paths
	do
	    if test -f ${d}/tclRegexp.h
	    then
		INCS="$INCS -I${d}"
		AC_MSG_RESULT($d)
		found=1
		break
	    fi
	done

	if test $found -eq 0 
	then
	    AC_MSG_WARN(not found)
	fi
    fi

    AC_DEFINE_UNQUOTED(HAVE_EXPECT, $HAVE_EXPECT)])

# Electric fence and dbnew are used to debug malloc/new and free/delete.
# I assume that if you use these switches you know enough to build the 
# libraries. 2/3/98 jhrg

AC_DEFUN(DODS_EFENCE, [dnl
    AC_ARG_ENABLE(efence,
		  [  --enable-efence         Runtime memory checks (malloc)],
		  EFENCE=$enableval, EFENCE=no)

    case "$EFENCE" in
    yes)
      AC_MSG_RESULT(Configuring dynamic memory checks on malloc/free calls)
      LIBS="$LIBS -lefence"
      ;;
    *)
      ;;
    esac])

AC_DEFUN(DODS_DBNEW, [dnl
    AC_ARG_ENABLE(dbnew,
	          [  --enable-dbnew          Runtime memory checks (new)],
		  DBNEW=$enableval, DBNEW=no)

    case "$DBNEW" in
    yes)
      AC_MSG_RESULT(Configuring dynamic memory checks on new/delete calls)
      AC_DEFINE(TRACE_NEW)
      LIBS="$LIBS -ldbnew"
      ;;
    *)
      ;;
    esac])

#check for hdf libraries
# cross-compile problem with test option -d
AC_DEFUN(DODS_HDF_LIBRARY, [dnl
    AC_ARG_WITH(hdf,
        [  --with-hdf=ARG          Where is the HDF library (directory)],
        HDF_PATH=${withval}, HDF_PATH="$HDF_PATH")
    if test ! -d "$HDF_PATH"
    then
        HDF_PATH="/usr/local/hdf"
    fi
    if test "$HDF_PATH"
    then
            LDFLAGS="$LDFLAGS -L${HDF_PATH}/lib"
            INCS="$INCS -I${HDF_PATH}/include"
            AC_SUBST(INCS)
    fi

dnl None of this works with HDF 4.1 r1. jhrg 8/2/97

    AC_CHECK_LIB(z, deflate, LIBS="-lz $LIBS", nohdf=1)
    AC_CHECK_LIB(jpeg, jpeg_start_compress, LIBS="-ljpeg $LIBS", nohdf=1)
    AC_CHECK_LIB(df, Hopen, LIBS="-ldf $LIBS" , nohdf=1)
    AC_CHECK_LIB(mfhdf, SDstart, LIBS="-lmfhdf $LIBS" , nohdf=1)])

# 3. Compiler test macros
#--------------------------------------------------------------------------

# Look for Flex version 2.5.2 or greater. 
# NB: on some machines `flex -V' writes to stderr *not* stdout while `|'
# connects only stdout to stdin. Thus for portability, stderr must be
# connected to stdout manually (This is true for IRIX-5.2).

# NB: had to use [[ for [ due to m4's quoting. 11/17/95.

AC_DEFUN(DODS_PROG_LEX, [dnl
    AC_PROG_LEX
    case "$LEX" in
	flex)
	    flex_ver1=`flex -V 2>&1 | sed 's/[[^0-9]]*\(.*\)/\1/'`
	    flex_ver2=`echo $flex_ver1 | sed 's/\.//g'`
	    if test -n "$flex_ver2" && test $flex_ver2 -ge 252
	    then
		AC_MSG_RESULT(Found flex version ${flex_ver1}.)
	    else
		AC_MSG_ERROR(Flex version: found ${flex_venr1} should be at least 2.5.2)
	    fi
	    ;;
	*)
	    AC_MSG_WARN(Flex is required for grammar changes.)
	    ;;
    esac])

# Look for Bison version 1.24 or greater. Define DODS_BISON_VER to be the
# version number without the decimal point.

AC_DEFUN(DODS_PROG_BISON, [dnl
    AC_CHECK_PROG(YACC,bison,bison)
    case "$YACC" in
	bison)
	    bison_ver1=`bison -V 2>&1 | sed 's/[[^0-9]]*\(.*\)/\1/'`
	    bison_ver2=`echo $bison_ver1 | sed 's/\.//g'`
	    AC_DEFINE_UNQUOTED(DODS_BISON_VER, $bison_ver2)
	    if test -n "$bison_ver2" && test $bison_ver2 -ge 125
	    then
		AC_MSG_RESULT(Found bison version ${bison_ver1}.)
	    else
		AC_MSG_ERROR(Bison version: found ${bison_ver1} should be at least 1.25)
	    fi
	    ;;
	*)
	    AC_MSG_WARN(Bison is required for grammar changes.)
	    ;;
    esac])

# Check for support of `-g' by gcc (SGI does not support it unless your using
# gas (and maybe ld).

NULL_PROGRAM="mail() {}"

AC_DEFUN(DODS_CHECK_GCC_DEBUG, [dnl
    AC_MSG_CHECKING(for gcc debugging support)
    msgs=`gcc -g /dev/null 2>&1`
    if echo $msgs | egrep "\`-g' option not supported"
    then		
	CFLAGS=`echo $CFLAGS | sed 's/-g//'`;
	CXXFLAGS=`echo $CXXFLAGS | sed 's/-g//'`;
	LDFLAGS=`echo $LDFLAGS | sed 's/-g//'`;
	AC_MSG_RESULT(not supported)
    else
	AC_MSG_RESULT(supported)
    fi])

# Look for the location of the g++ include directory

AC_DEFUN(DODS_FIND_GPP_INC, [dnl
    AC_MSG_CHECKING(for the g++ include directories)
    AC_REQUIRE([DODS_GCC_VERSION])

    GPP_INC=""
    case $GCC_VER in
	2.8*) specs=`gcc -v 2>&1`;
           dir=`echo $specs | sed 's@Reading specs from \(.*\)lib\/gcc-lib.*@\1@'`;
           GPP_INC="${dir}include/g++";;
	*) specs=`gcc -v 2>&1`;
           dir=`echo $specs | sed 's@Reading specs from \(.*\)gcc-lib.*@\1@'`;
           GPP_INC="${dir}g++include";;
    esac

    if test -z "$GPP_INC"
    then
	AC_MSG_WARN(not found)
    else
        AC_MSG_RESULT($GPP_INC);
        AC_SUBST(GPP_INC)
    fi])

# Find the root directory of the current rev of gcc

AC_DEFUN(DODS_GCC, [dnl
    AC_REQUIRE([DODS_GCC_VERSION])
    AC_MSG_CHECKING(for gcc's libgcc.a)

    GCC_ROOT=`gcc -v 2>&1 | awk '/specs/ {print}'`
    GCC_ROOT=`echo $GCC_ROOT | sed 's@[[^/]]*\(/.*\)/specs@\1@'` 
    
    AC_SUBST(GCC_ROOT)
    AC_MSG_RESULT($GCC_ROOT)])

AC_DEFUN(DODS_GCC_VERSION, [dnl
    AC_MSG_CHECKING(for gcc/g++ 2.8 or greater)

    GCC_VER=`gcc -v 2>&1 | awk '/version/ {print}'`
    dnl We need the gcc version number as a number, without `.'s and limited
    dnl to three digits
    GCC_VER=`echo $GCC_VER | sed 's@[[a-z ]]*\([[0-9.]]\)@\1@'`

    dnl g++ 2.8.0 and greater does not automatically link with -lg++, so we 
    dnl supply it here.
    case $GCC_VER in
        *egcs*) AC_MSG_RESULT(Found egcs version ${GCC_VER}.) ;;
        2.8*)   AC_MSG_RESULT(Found gcc/g++ version ${GCC_VER}) ;;
        2.7*)   AC_MSG_RESULT(Found gcc/g++ version ${GCC_VER}) ;;
        *)      AC_MSG_ERROR(must be at least version 2.7.x) ;;

dnl This old code was replaced witht he above which adds support for egcs and 
dnl removes -lg++ (since the libg++ code is now in the dap directory). It
dnl may be that soon the libg++ code will vanish... 7/28/98 jhrg
dnl
dnl	2.8*) 	AC_MSG_RESULT(Found gcc/g++ version ${GCC_VER} adding -lg++.)
dnl		LIBS="$LIBS -lg++ -lstdc++" ;;
dnl        2.7*)   AC_MSG_RESULT(Found gcc/g++ version ${GCC_VER}) ;;
dnl	*)      AC_MSG_ERROR(must be at least version 2.7.x) ;;
    esac])


dnl Check for exceptions handling support. From Todd.

# Check for exceptions handling support
AC_DEFUN(DODS_CHECK_EXCEPTIONS, [dnl
    AC_LANG_CPLUSPLUS
    AC_MSG_CHECKING("for exception handling support in C++ compiler")
    OLDCXXFLAGS="$CXXFLAGS"
    if test $CXX = "g++"; then
       CXXFLAGS="$OLDCXXFLAGS -fhandle-exceptions"
    fi
    EXCEPTION_CHECK_PRG="int foo(void) {
                              throw int();
                         }
                         main() {
                              try { foo(); }
                              catch(int) { exit(0); }
                              exit(1);
                         }"

    AC_TRY_RUN([${EXCEPTION_CHECK_PRG}],
        AC_MSG_RESULT(yes),
	[dnl
        AC_MSG_RESULT(no)
        AC_MSG_WARN("Compiling without exception handling.  See README.")
        CXXFLAGS=$OLDCXXFLAGS
        CPPFLAGS="$CPPFLAGS -DNO_EXCEPTIONS"
        ],true)])

# 4. Macros to locate various programs/systems used by parts of DODS
#---------------------------------------------------------------------------

# Find the matlab root directory
# cross-compile problem with test option -d

AC_DEFUN(DODS_MATLAB, [dnl
    AC_ARG_WITH(matlab,
        [  --with-matlab=ARG       Where is the Matlab root directory],
        MATLAB_ROOT=${withval}, MATLAB_ROOT="$MATLAB_ROOT")
    if test "$MATLAB_ROOT" = no; then
        MATLAB_ROOT="$MATLAB_ROOT"
    elif test ! -d "$MATLAB_ROOT"
    then
        MATLAB_ROOT=""
    fi
    if test -z "$MATLAB_ROOT"
    then
        AC_MSG_CHECKING(for matlab root)

	MATLAB_ROOT=`cmex -v 2>&1 | awk '/MATLAB *= / {print}'`
	MATLAB_ROOT=`echo $MATLAB_ROOT | sed 's@[[^/]]*\(/.*\)@\1@'`

	if test -z "$MATLAB_ROOT"
	then
	    AC_MSG_ERROR(Matlab not found! Run configure using -with-matlab option)
        else
	    AC_SUBST(MATLAB_ROOT)
	    AC_MSG_RESULT($MATLAB_ROOT)
        fi
    else
        AC_SUBST(MATLAB_ROOT)
        AC_MSG_RESULT("Set Matlab root to $MATLAB_ROOT")
    fi

    dnl Find the lib directory (which is named according to machine type).
    matlab_lib_dir=`find ${MATLAB_ROOT}/extern -name 'libmat*' -print \
		    | sed 's@\(.*\)/libmat.*@\1@'`
    if test "$matlab_lib_dir"
    then
	LDFLAGS="$LDFLAGS -L$matlab_lib_dir"
	dnl This is used by the nph script to set LD_LIBRARY_PATH
	AC_SUBST(matlab_lib_dir)
    fi
    
    dnl sleazy test for version 5; look for the version 4 compat flag

    if grep V4_COMPAT ${MATLAB_ROOT}/extern/include/mat.h > /dev/null 2>&1
    then
       MAT_VERSION_FLAG="-V4"
       MATLIBS="-lmat -lmi -lmx -lut"
    else
       MAT_VERSION_FLAG=""
       MATLIBS="-lmat"
    fi

    AC_CHECK_LIB(ots, _OtsDivide64Unsigned, MATLIBS="$MATLIBS -lots", )
    AC_SUBST(MATLIBS)
    AC_SUBST(MAT_VERSION_FLAG)])

# cross-compile problem with test option -d
AC_DEFUN(DODS_DSP_ROOT, [dnl

    AC_ARG_WITH(dsp,
		[  --with-dsp=DIR          Directory containing DSP software from U of Miami],
		DSP_ROOT=${withval}, DSP_ROOT="$DSP_ROOT")

    if test ! -d "$DSP_ROOT"
    then
        DSP_ROOT=""
    fi
    if test -z "$DSP_ROOT"
    then
	AC_MSG_CHECKING(for the DSP library root directory)

	for p in /usr/local/src/DSP /usr/local/DSP \
		 /usr/local/src/dsp /usr/local/dsp \
		 /usr/contrib/src/dsp /usr/contrib/dsp \
		 $DODS_ROOT/third-party/dsp /usr/dsp /data1/dsp
	do
	    if test -z "$DSP_ROOT"
	    then
	    	for d in `ls -dr ${p}* 2>/dev/null`
		do
		    if test -f ${d}/inc/dsplib.h
		    then
		        DSP_ROOT=${d}
		        break
		    fi
	        done
	    fi
	done
    fi

    if test "$DSP_ROOT"
    then
	AC_SUBST(DSP_ROOT)
	dnl Only add this path to gcc's options... jhrg 11/15/96
	CFLAGS="$CFLAGS -I\$(DSP_ROOT)/inc"
	LDFLAGS="$LDFLAGS -L\$(DSP_ROOT)/lib -L\$(DSP_ROOT)/shlib"
	AC_MSG_RESULT(Set DSP root directory to $DSP_ROOT) 
    else
        AC_MSG_WARN(not found!)
    fi])

# 5. Misc stuff
#---------------------------------------------------------------------------

# Use the version.h file in the current directory to set the version 
# Makefile varible. All Makefiles should have targets that use this variable
# to rename the directory, build source distribution tarfiles, etc.
AC_DEFUN(DODS_DIRECTORY_VERSION, [dnl
    VERSION=`cat version.h`
    AC_MSG_RESULT(Setting Makefile version variable to $VERSION)
    AC_SUBST(VERSION)])

AC_DEFUN(DODS_DEBUG_OPTION, [dnl
    AC_ARG_ENABLE(debug, 
		  [  --enable-debug=ARG      Program instrumentation (1,2)],
		  DEBUG=$enableval, DEBUG=no)

    case "$DEBUG" in
    no) 
      ;;
    1)
      AC_MSG_RESULT(Setting debugging to level 1)
      AC_DEFINE(DODS_DEBUG)
      ;;
    2) 
      AC_MSG_RESULT(Setting debugging to level 2)
      AC_DEFINE(DODS_DEBUG)
      AC_DEFINE(DODS_DEBUG2)
      ;;
    *)
      AC_MSG_ERROR(Bad debug value)
      ;;
    esac])

AC_DEFUN(DODS_SEM, [dnl
    found=0
    AC_CHECK_HEADERS(sys/sem.h, found=1, found=0)
    if test $found -eq 1
    then
        AC_CHECKING(semaphore features in sem.h)
        if grep 'int *semctl.*(' /usr/include/sys/sem.h >/dev/null 2>&1
        then
            AC_DEFINE(HAVE_SEM_PROTO, 1)
        else
            AC_DEFINE(HAVE_SEM_PROTO, 0)
        fi

        if grep 'union *semun *{' /usr/include/sys/sem.h >/dev/null 2>&1
        then
           AC_DEFINE(HAVE_SEM_UNION, 1)
        else
           AC_DEFINE(HAVE_SEM_UNION, 0)
        fi
    fi])

AC_DEFUN(DODS_OS, [dnl
    AC_MSG_CHECKING(type of operating system)
    # I have removed the following test because some systems (e.g., SGI)
    # define OS in a way that breaks this code but that is close enough
    # to also be hard to detect. jhrg 3/23/97
    #  if test -z "$OS"; then
    #  fi 
    OS=`uname -s | tr '[[A-Z]]' '[[a-z]]' | sed 's;/;;g'`
    if test -z "$OS"; then
        AC_MSG_WARN(OS unknown!)
    fi
    case $OS in
        aix)
            ;;
        hp-ux)
            OS=hpux`uname -r | sed 's/[[A-Z.0]]*\([[0-9]]*\).*/\1/'`
            ;;
        irix)
            OS=${OS}`uname -r | sed 's/\..*//'`
            ;;
        # I added the following case because the `tr' command above *seems* 
	# to fail on Irix 5. I can get it to run just fine from the shell, 
	# but not in the configure script built using this macro. jhrg 8/27/97
        IRIX)
            OS=irix`uname -r | sed 's/\..*//'`
	    ;;
        osf*)
            ;;
        sn*)
            OS=unicos
            ;;
        sunos)
            OS_MAJOR=`uname -r | sed 's/\..*//'`
            OS=$OS$OS_MAJOR
            ;;
        ultrix)
            case `uname -m` in
            VAX)
                OS=vax-ultrix
                ;;
            esac
            ;;
        *)
            # On at least one UNICOS system, 'uname -s' returned the
            # hostname (sigh).
            if uname -a | grep CRAY >/dev/null; then
                OS=unicos
            fi
            ;;
    esac

    # Adjust OS for CRAY MPP environment.
    #
    case "$OS" in
    unicos)

        case "$CC$TARGET$CFLAGS" in
        *cray-t3*)
            OS=unicos-mpp
            ;;
        esac
        ;;
    esac

    AC_SUBST(OS)

    AC_MSG_RESULT($OS)])


AC_DEFUN(DODS_MACHINE, [dnl
    AC_MSG_CHECKING(type of machine)

    if test -z "$MACHINE"; then
    MACHINE=`uname -m | tr '[[A-Z]]' '[[a-z]]'`
    case $OS in
        aix*)
            MACHINE=rs6000
            ;;
        hp*)
            MACHINE=hp`echo $MACHINE | sed 's%/.*%%'`
            ;;
        sunos*)
            case $MACHINE in
                sun4*)
                    MACHINE=sun4
                    ;;
            esac
            ;;
        irix*)
            case $MACHINE in
                ip2?)
                    MACHINE=sgi
                    ;;
            esac
            ;;
    esac
    fi

    AC_SUBST(MACHINE)
    AC_MSG_RESULT($MACHINE)])

AC_DEFUN(DODS_CHECK_SIZES, [dnl
    # Ignore the errors about AC_TRY_RUN missing an argument. jhrg 5/2/95

    AC_REQUIRE([AC_PROG_CC])

    if test "$cross_compiling" = "yes"
    then
	    case "$host" in
	    *alpha*) ac_cv_sizeof_long=8
		     AC_DEFINE(SIZEOF_CHAR, 1)
		     AC_DEFINE(SIZEOF_DOUBLE, 8)
		     AC_DEFINE(SIZEOF_FLOAT, 4)
		     AC_DEFINE(SIZEOF_INT, 4)
		     AC_DEFINE(SIZEOF_LONG, 8)
		     ;;
	    *)	AC_MSG_WARN(Assuming that your target is a 32bit machine)
		    ac_cv_sizeof_long=4
		    AC_DEFINE(SIZEOF_CHAR, 1)
		    AC_DEFINE(SIZEOF_DOUBLE, 8)
		    AC_DEFINE(SIZEOF_FLOAT, 4)
		    AC_DEFINE(SIZEOF_INT, 4)
		    AC_DEFINE(SIZEOF_LONG, 4)
		    ;;
	    esac
    else
	    AC_CHECK_SIZEOF(int)
	    AC_CHECK_SIZEOF(long)
	    AC_CHECK_SIZEOF(char)
	    AC_CHECK_SIZEOF(double)
	    AC_CHECK_SIZEOF(float)
    fi

    if test $ac_cv_sizeof_long -eq 4 
    then
	ARCHFLAG=ARCH_32BIT
	AC_SUBST(ARCHFLAG)
	CPPFLAGS="-DARCH_32BIT $CPPFLAGS"
    elif test $ac_cv_sizeof_long -eq 8
    then
	ARCHFLAG=ARCH_64BIT
	AC_SUBST(ARCHFLAG)
	CPPFLAGS="-DARCH_64BIT $CPPFLAGS"
    else
	AC_MSG_ERROR(Could not determine architecture size - 32 or 64 bits)
    fi])

# Added by Ethan, 1999/06/21
# Look for perl.
# 
# I modified the regexp below to removed any text that follows the version
# number. This extra text was hosing the text. 7/15/99 jhrg

AC_DEFUN(DODS_PROG_PERL, [dnl
    AC_CHECK_PROG(PERL,perl,perl)
    case "$PERL" in
	perl)
	    perl_ver=`$PERL -v 2>&1 | awk '/This is perl/ {print}'`
	    perl_ver=`echo $perl_ver | sed 's/This is perl, version \([[0-9._]]*\).*/\1/'`
            perl_ver_main=`echo $perl_ver | sed 's/\([[0-9]]*\).*/\1/'`
	    if test -n "$perl_ver" && test $perl_ver_main -ge 5
	    then
		AC_MSG_RESULT(Found perl version ${perl_ver}.)
	    else
		AC_MSG_ERROR(perl version: found ${perl_ver} should be at least 5.000.)
	    fi
	    ;;
	*)
	    AC_MSG_WARN(perl is required.)
	    ;;
    esac

    AC_SUBST(PERL)])

# Added by Ethan, 1999/06/21
# Look for GNU tar.
# 
# I modified the regexp below but it still does not work exactly correctly; 
# the variable tar_ver should have only the version number in it. However,
# my version (1.12) spits out a multi-line thing. The regexp below gets the
# version number from the first line but does not remove the subsequent lines
# of garbage. 7/15/99 jhrg
# Added awk line to handle multiline output. 1999/07/22 erd

AC_DEFUN(DODS_PROG_GTAR, [dnl
    AC_CHECK_PROGS(TAR,gtar tar,tar)
    case "$TAR" in
	*tar)
	    tar_ver=`$TAR --version 2>&1 | awk '/G[[Nn]][[Uu]] tar/ {print}'`
	    tar_ver=`echo $tar_ver | sed 's/.*GNU tar[[^0-9.]]*\([[0-9._]]*\)/\1/'`
	    if test -n "$tar_ver"
	    then
		AC_MSG_RESULT(Found Gnu tar version ${tar_ver}.)
	    else
		AC_MSG_ERROR(GNU tar is required.)
	    fi
	    ;;
	*)
	    AC_MSG_WARN(GNU tar is required.)
	    ;;
    esac

    AC_SUBST(TAR)])
