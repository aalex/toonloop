AC_DEFUN([AC_PROG_HELP2MAN],[
AC_CHECK_PROGS(help2man,[help2man],no)
export help2man;
if test $help2man = "no" ;
then
    AC_MSG_WARN([Unable to find the help2man application]);
fi
AC_SUBST(help2man)
])


