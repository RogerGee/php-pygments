/*
 * pygments.c
 *
 * This file is a part of php-pygments.
 *
 * Copyright (C) 2018 Roger P. Gee
 */

#include "pygments.h"

/* Module/request functions */
static PHP_MINIT_FUNCTION(pygments);
static PHP_MINFO_FUNCTION(pygments);
static PHP_MSHUTDOWN_FUNCTION(pygments);
static PHP_RINIT_FUNCTION(pygments);
static PHP_RSHUTDOWN_FUNCTION(pygments);

/* PHP userspace functions */
static PHP_FUNCTION(pygments_highlight);

/* Function entries */
static zend_function_entry php_pygments_functions[] = {
    PHP_FE(pygments_highlight,NULL)
    {NULL, NULL, NULL}
};

/* Module entries */
zend_module_entry pygments_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    PHP_PYGMENTS_EXTNAME,
    php_pygments_functions,
    PHP_MINIT(pygments),
    PHP_MSHUTDOWN(pygments),
    PHP_RINIT(pygments),
    PHP_RSHUTDOWN(pygments),
    PHP_MINFO(pygments),
#if ZEND_MODULE_API_NO >= 20010901
    PHP_PYGMENTS_EXTVER,
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_PYGMENTS
ZEND_GET_MODULE(pygments)
#endif

/* Define module globals. */
ZEND_DECLARE_MODULE_GLOBALS(pygments);

static void php_pygments_globals_ctor(zend_pygments_globals* gbls TSRMLS_DC)
{
    int result = pygments_context_init(&gbls->highlighter);

    if (result == -1) {
        php_error(E_WARNING,"pygments: fail pygments_context_init()");
    }
}

static void php_pygments_globals_dtor(zend_pygments_globals* gbls TSRMLS_DC)
{
    int result = pygments_context_close(&gbls->highlighter);

    if (result == -1) {
        php_error(E_WARNING,"pygments: fail pygments_context_close()");
    }
}

/* Implementation of module/request functions */

PHP_MINIT_FUNCTION(pygments)
{
    /* NOTE: this assumes python hasn't already been initialized somewhere else
     * in PHP. We'll need a more sophisticated mechanism for deciding when to
     * initialize/finalize (using Py_IsInitialized()).
     *
     * We call Py_InitializeEx(0) to avoid registration of signal handlers.
     */
    Py_InitializeEx(0);

#ifdef ZTS
    ts_allocate_id(&pygments_globals_id,
        sizeof(zend_pygments_globals),
        (ts_allocate_ctor)php_pygments_globals_ctor,
        (ts_allocate_dtor)php_pygments_globals_dtor);
#else
    php_pygments_globals_ctor(&pygments_globals TSRMLS_CC);
#endif

    return SUCCESS;
}

PHP_MINFO_FUNCTION(pygments)
{
    php_info_print_table_start();
    php_info_print_table_row(2,PHP_PYGMENTS_EXTNAME,"enabled");
    php_info_print_table_row(2,"extension version",PHP_PYGMENTS_EXTVER);
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}

PHP_MSHUTDOWN_FUNCTION(pygments)
{
    /* Free globals if non-threaded build. Threaded PHP cleans up globals
     * automatically. We must do this before shutting down python.
     */
#ifndef ZTS
    php_pygments_globals_dtor(&pygments_globals TSRMLS_CC);
#endif

    /* NOTE: this assumes python hasn't already been finalized somewhere else in
     * PHP. We'll need a more sophisticated mechanism for deciding when to
     * initialize/finalize (using Py_IsInitialized()).
     */
    Py_Finalize();

    return SUCCESS;
}

PHP_RINIT_FUNCTION(pygments)
{

    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(pygments)
{

    return SUCCESS;
}

/* Implementation of userspace functions */

PHP_FUNCTION(pygments_highlight)
{
    char* code;
    int code_len;
    struct highlight_result* result;

    if (zend_parse_parameters(ZEND_NUM_ARGS(),"s",&code,&code_len) == FAILURE) {
        return;
    }

    result = highlight(&PYGMENTS_G(highlighter),code,NULL);

    if (result == NULL) {
        RETURN_FALSE;

        /* Control no longer in function. */
    }

    RETVAL_STRING(result->html,1);
    highlight_result_free(result);
}
