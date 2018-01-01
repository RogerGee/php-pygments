/*
 * pygments.c
 *
 * This file is a part of php-pygments.
 *
 * Copyright (C) 2018 Roger P. Gee
 */

#include "pygments.h"
#include <Python.h>

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

/* Implementation of module/request functions */

PHP_MINIT_FUNCTION(pygments)
{
    Py_Initialize();

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
}
