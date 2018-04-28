/*
 * pygments.h
 *
 * This file is a part of php-pygments.
 *
 * Copyright (C) 2018 Roger P. Gee
 */

#ifndef PHP_PYGMENTS_H
#define PHP_PYGMENTS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <php.h>
#include <ext/standard/info.h>
#include "highlight.h"

#ifdef ZTS
#include "TSRM.h"
#endif

/* Extension definitions */

#define PHP_PYGMENTS_EXTNAME "pygments"
#define PHP_PYGMENTS_EXTVER "0.0.0"

/* Module globals */

ZEND_BEGIN_MODULE_GLOBALS(pygments)
  struct pygments_context highlighter;
ZEND_END_MODULE_GLOBALS(pygments)
extern ZEND_DECLARE_MODULE_GLOBALS(pygments);

#ifdef ZTS
#define PYGMENTS_G(v)                                   \
    TSRMG(pygments_globals_id,zend_pygments_globals*,v)
#else
#define PYGMENTS_G(v)                           \
    (pygments_globals.v)
#endif

#endif
