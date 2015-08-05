
# 
# These macros are used for the asciival tests.

AT_INIT([asciival])
# AT_COPYRIGHT([])

AT_TESTED([besstandalone])

AT_ARG_OPTION_ARG([generate g],
    [  -g arg, --generate=arg   Build the baseline file for test 'arg'],
    [if besstandalone -c bes.conf  -i $at_arg_generate -f $at_arg_generate.baseline; then
         echo "Built baseline for $at_arg_generate"
     else
         echo "Could not generate baseline for $at_arg_generate"
     fi     
     exit],[])

# Usage: _AT_TEST_*(<bescmd source>, <baseline file>)

m4_define([AT_BESCMD_RESPONSE_TEST],   
[AT_SETUP([BESCMD $1])
AT_KEYWORDS([ascii])
AT_CHECK([besstandalone -c $abs_builddir/bes.conf -i $abs_srcdir/$1 || true], [], [stdout], [stderr])
AT_CHECK([diff -b -B $abs_srcdir/$1.baseline stdout || diff -b -B $abs_srcdir/$1.baseline stderr], [], [ignore],[],[])
AT_XFAIL_IF([test "$2" = "xfail"])
AT_CLEANUP])


