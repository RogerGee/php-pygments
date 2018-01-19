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
static PHP_FUNCTION(pygments_set_options);

/* Function entries */
static zend_function_entry php_pygments_functions[] = {
    PHP_FE(pygments_highlight,NULL)
    PHP_FE(pygments_set_options,NULL)
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
    /* NOTE: Since another module could be using libpython, we check the
     * initialize state of libpython before attempting anything on it. This
     * works so long as each module contractually behaves in this way.
     *
     * Also, we call Py_InitializeEx(0) to avoid registration of signal
     * handlers.
     */
    if (!Py_IsInitialized()) {
        Py_InitializeEx(0);
    }

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

    /* NOTE: Since another module could be using libpython, we check the
     * initialize state of libpython before attempting anything on it. This
     * works so long as each module contractually behaves in this way.
     */
    if (Py_IsInitialized()) {
        Py_Finalize();
    }

    return SUCCESS;
}

PHP_RINIT_FUNCTION(pygments)
{

    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(pygments)
{
    /* Reset the formatter options to their defaults so that each request starts
     * out with that same state.
     */
    pygments_context_set_default_options(&PYGMENTS_G(highlighter));

    return SUCCESS;
}

/* Implementation of userspace functions */

/* {{{ proto string pygments_highlight(string code[, string lexer, string filename])
   Syntax-highlights the specified code, applying any specified options */
PHP_FUNCTION(pygments_highlight)
{
    char* code;
    int code_len;
    char* preferredLexer = NULL;
    int preferredLexer_len = 0;
    char* filename = NULL;
    int filename_len = 0;
    struct lexer_options lxopts;
    struct highlight_result* result;

    if (zend_parse_parameters(ZEND_NUM_ARGS(),"s|s!s!",&code,&code_len,
            &preferredLexer,&preferredLexer_len,&filename,&filename_len) == FAILURE)
    {
        return;
    }

    /* Assign lexer options. May be NULL if not provided by the user. */
    lxopts.preferred_lexer = preferredLexer;
    lxopts.filename = filename;

    result = highlight(&PYGMENTS_G(highlighter),code,&lxopts);

    if (result == NULL) {
        RETURN_FALSE;

        /* Control no longer in function. */
    }

    RETVAL_STRING(result->html,1);
    highlight_result_free(result);
}
/* }}} */

/* {{{ proto void pygments_set_options(array options)
   Sets the formatter options to the global pygments context */
PHP_FUNCTION(pygments_set_options)
{
    zval* zopts;
    struct pygments_context* ctx;
    struct context_options ctxopts;

    if (zend_parse_parameters(ZEND_NUM_ARGS(),"a",&zopts) == FAILURE) {
        return;
    }

    pygments_context_options_parse(&ctxopts,zopts,"pygments_highlight");
    pygments_context_assign_options(&PYGMENTS_G(highlighter),&ctxopts);
}
/* }}} */
