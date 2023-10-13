PHP_ARG_ENABLE(pygments,[Whether to enable the "pygments" extension],
    [  --enable-pygments      Enable "pygments" extension support])

if test $PHP_PYGMENTS != "no"; then
    CFLAGS="$CFLAGS "`pkg-config --cflags --libs-only-L python3-embed`
    MODVERSION="`pkg-config --modversion python3-embed`"

    AC_SEARCH_LIBS([Py_Initialize],[python$MODVERSION],[],[AC_MSG_ERROR([Aborting since libpython$MODVERSION not found],[1])])
    AC_CHECK_HEADERS([Python.h],[],[AC_MSG_ERROR([Aborting since Python.h not found],[1])])

    # Add PHP_RPATHS to extension build via EXTRA_LDFLAGS.
    if test $PHP_RPATHS != ""; then
        PHP_UTILIZE_RPATHS()
        EXTRA_LDFLAGS="$PHP_RPATHS"
        PHP_SUBST([EXTRA_LDFLAGS])
    fi

    PHP_ADD_LIBRARY(python$MODVERSION,1,PYGMENTS_SHARED_LIBADD)
    PHP_SUBST(PYGMENTS_SHARED_LIBADD)
    PHP_NEW_EXTENSION(pygments,pygments.c highlight.c,$ext_shared)
fi
