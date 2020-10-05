PHP_ARG_ENABLE(pygments,[Whether to enable the "pygments" extension],
    [  --enable-pygments      Enable "pygments" extension support])

if test $PHP_PYGMENTS != "no"; then
    CFLAGS="$CFLAGS "`pkg-config --cflags --libs-only-L python2`

    AC_SEARCH_LIBS([Py_Initialize],[python2.7],[],[AC_MSG_ERROR([Aborting since libpython2.7 not found],[1])])
    AC_CHECK_HEADERS([Python.h],[],[AC_MSG_ERROR([Aborting since Python.h not found],[1])])

    # Add PHP_RPATHS to extension build via EXTRA_LDFLAGS.
    if test $PHP_RPATHS != ""; then
        PHP_UTILIZE_RPATHS()
        EXTRA_LDFLAGS="$PHP_RPATHS"
        PHP_SUBST([EXTRA_LDFLAGS])
    fi

    PHP_ADD_LIBRARY(python2.7,1,PYGMENTS_SHARED_LIBADD)
    PHP_SUBST(PYGMENTS_SHARED_LIBADD)
    PHP_NEW_EXTENSION(pygments,pygments.c highlight.c,$ext_shared)
fi
