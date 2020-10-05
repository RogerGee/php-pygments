PHP_ARG_ENABLE(pygments,[Whether to enable the "pygments" extension],
    [  --enable-pygments      Enable "pygments" extension support])

if test $PHP_PYGMENTS != "no"; then
    CFLAGS="$CFLAGS "`pkg-config --cflags --libs-only-L python2`

    AC_SEARCH_LIBS([Py_Initialize],[python2.7],[],[AC_MSG_ERROR([Aborting since libpython2.7 not found],[1])])
    AC_CHECK_HEADERS([Python.h],[],[AC_MSG_ERROR([Aborting since Python.h not found],[1])])

    PHP_ADD_LIBRARY(python2.7,1,PYGMENTS_SHARED_LIBADD)
    PHP_SUBST(PYGMENTS_SHARED_LIBADD)
    PHP_NEW_EXTENSION(pygments,pygments.c highlight.c,$ext_shared)
fi
