/*
 * highlight.c
 *
 * php-pygments
 *
 * Copyright (C) Roger P. Gee
 */

#include "highlight.h"
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define NULL2EMPTY(val) (val == NULL ? "" : val)

static void make_default_options(struct context_options* opts)
{
    memset(opts,0,sizeof(struct context_options));
    opts->linenostart = 1;
    opts->lineanchors = "";
    opts->classprefix = "";
    opts->cssclass = PHP_PYGMENTS_DEFAULT_CSSCLASS;
    opts->cssstyles = "";
    opts->prestyles = "";
}

static PyObject* lookup_lexer(const struct pygments_context* ctx,
    PyObject* pycode,const struct lexer_options* opts)
{
    PyObject* lexer;
    PyObject* args;

    if (opts != NULL && opts->preferred_lexer != NULL) {
        PyObject* name = PyUnicode_FromString(opts->preferred_lexer);
        if (name == NULL) {
            PyErr_Clear();
            return NULL;
        }

        args = PyTuple_Pack(1,name);
        Py_DECREF(name);
        if (args == NULL) {
            PyErr_Clear();
            return NULL;
        }

        lexer = PyObject_CallObject(ctx->func_get_lexer_by_name,args);
        Py_DECREF(args);
        if (lexer == NULL) {
            PyErr_Clear();
            return NULL;
        }
    }
    else if (opts != NULL && opts->filename != NULL) {
        args = Py_BuildValue("(sO)",opts->filename,pycode);
        if (args == NULL) {
            PyErr_Clear();
            return NULL;
        }

        lexer = PyObject_CallObject(ctx->func_guess_lexer_for_filename,args);
        Py_DECREF(args);
        if (lexer == NULL) {
            PyErr_Clear();

            /* Fall back on guess_lexer if not found by filename. */
            args = Py_BuildValue("(O)",pycode);
            if (args == NULL) {
                PyErr_Clear();
                return NULL;
            }

            lexer = PyObject_CallObject(ctx->func_guess_lexer,args);
            Py_DECREF(args);
            if (lexer == NULL) {
                PyErr_Print();
                PyErr_Clear();
                return NULL;
            }
        }
    }
    else {
        args = Py_BuildValue("(O)",pycode);
        if (args == NULL) {
            PyErr_Clear();
            return NULL;
        }

        lexer = PyObject_CallObject(ctx->func_guess_lexer,args);
        Py_DECREF(args);
        if (lexer == NULL) {
            PyErr_Clear();
            return NULL;
        }
    }

    return lexer;
}

static int zval_check_bool(int* result,zval* zv,const char* errctx,const char* optname)
{
    if (Z_TYPE_P(zv) == IS_TRUE || Z_TYPE_P(zv) == IS_FALSE) {
        *result = Z_TYPE_P(zv) == IS_TRUE;
        return SUCCESS;
    }

    if (Z_TYPE_P(zv) == IS_LONG) {
        *result = Z_LVAL_P(zv) ? 1 : 0;
        return SUCCESS;
    }

    if (Z_TYPE_P(zv) == IS_STRING) {
        const char* str = Z_STRVAL_P(zv);
        if (strcasecmp(str,"false") == 0) {
            *result = 0;
            return SUCCESS;
        }
        if (strcasecmp(str,"true") == 0) {
            *result = 1;
            return SUCCESS;
        }
    }

    zend_throw_error(NULL,"%s: option '%s' must be a boolean",errctx,optname);
    return FAILURE;
}

static int zval_check_int(int* result,zval* zv,const char* errctx,const char* optname)
{
    if (Z_TYPE_P(zv) == IS_LONG) {
        *result = Z_LVAL_P(zv);
        return SUCCESS;
    }

    if (Z_TYPE_P(zv) == IS_TRUE || Z_TYPE_P(zv) == IS_FALSE) {
        *result = Z_TYPE_P(zv) == IS_TRUE ? 1 : 0;
        return SUCCESS;
    }

    if (Z_TYPE_P(zv) != IS_STRING) {
        zend_throw_error(NULL,"%s: option '%s' must be an integer",errctx,optname);
        return FAILURE;
    }

    return (int)zval_get_long(zv);
}

static int zval_check_string(
    const char** result,
    zval* zv,
    const char* errctx,
    const char* optname)
{
    if (Z_TYPE_P(zv) == IS_NULL) {
        *result = "";
        return SUCCESS;
    }

    if (Z_TYPE_P(zv) == IS_STRING) {
        *result = Z_STRVAL_P(zv);
        return SUCCESS;
    }

    zend_throw_error(NULL,"%s: option '%s' must be a string",errctx,optname);
    return FAILURE;
}

static inline int set_python_attribute_bool(PyObject* inst,const char* attr,int value)
{
    return PyObject_SetAttrString(inst,attr,value ? Py_True : Py_False);
}

static int set_python_attribute_int(PyObject* inst,const char* attr,int value)
{
    int result;
    PyObject* num;

    num = PyLong_FromLong((long)value);
    if (num == NULL) {
        PyErr_Clear();
        return -1;
    }

    result = PyObject_SetAttrString(inst,attr,num);
    Py_DECREF(num);

    return result;
}

static int set_python_attribute_string(PyObject* inst,const char* attr,const char* value)
{
    int result;
    PyObject* str;

    str = PyUnicode_FromString(value);
    if (str == NULL) {
        PyErr_Clear();
        return -1;
    }

    result = PyObject_SetAttrString(inst,attr,str);
    Py_DECREF(str);

    if (result == -1) {
        PyErr_Clear();
        return -1;
    }

    return result;
}

static inline int set_python_attribute_none(PyObject* inst,const char* attr,int delattr)
{
    int result = delattr ? PyObject_DelAttrString(inst,attr)
        : PyObject_SetAttrString(inst,attr,Py_None);

    if (result == -1) {
        PyErr_Clear();
        return -1;
    }

    return result;
}

int pygments_context_init(struct pygments_context* ctx)
{
    PyObject* name;
    PyObject* formatters_module;
    PyObject* HtmlFormatter_class;

    memset(ctx,0,sizeof(struct pygments_context));

    /* Import modules. */

    name = PyUnicode_FromString("pygments");
    if (name == NULL) {
        PyErr_Clear();
        return -1;
    }

    ctx->module_pygments = PyImport_Import(name);
    Py_DECREF(name);
    if (ctx->module_pygments == NULL) {
        PyErr_Clear();
        return -1;
    }

    name = PyUnicode_FromString("pygments.lexers");
    if (name == NULL) {
        PyErr_Clear();
        pygments_context_close(ctx);
        return -1;
    }

    ctx->module_lexers = PyImport_Import(name);
    Py_DECREF(name);
    if (ctx->module_lexers == NULL) {
        PyErr_Clear();
        pygments_context_close(ctx);
        return -1;
    }

    name = PyUnicode_FromString("pygments.formatters");
    if (name == NULL) {
        PyErr_Clear();
        pygments_context_close(ctx);
        return -1;
    }

    formatters_module = PyImport_Import(name);
    Py_DECREF(formatters_module);
    if (formatters_module == NULL) {
        PyErr_Clear();
        pygments_context_close(ctx);
        return -1;
    }

    /* Create formatter instance. */

    HtmlFormatter_class = PyObject_GetAttrString(formatters_module,"HtmlFormatter");
    if (HtmlFormatter_class == NULL) {
        PyErr_Clear();
        Py_DECREF(formatters_module);
        pygments_context_close(ctx);
        return -1;
    }

    ctx->formatter = PyObject_CallObject(HtmlFormatter_class,NULL);
    if (ctx->formatter == NULL) {
        PyErr_Clear();
        Py_DECREF(formatters_module);
        Py_DECREF(HtmlFormatter_class);
        pygments_context_close(ctx);
        return -1;
    }

    ctx->func_highlight = PyObject_GetAttrString(ctx->module_pygments,"highlight");
    if (ctx->func_highlight == NULL) {
        PyErr_Clear();
        Py_DECREF(formatters_module);
        Py_DECREF(HtmlFormatter_class);
        pygments_context_close(ctx);
        return -1;
    }

    ctx->func_get_lexer_by_name = PyObject_GetAttrString(ctx->module_lexers,"get_lexer_by_name");
    if (ctx->func_get_lexer_by_name == NULL) {
        PyErr_Clear();
        Py_DECREF(formatters_module);
        Py_DECREF(HtmlFormatter_class);
        pygments_context_close(ctx);
        return -1;
    }

    ctx->func_guess_lexer_for_filename = PyObject_GetAttrString(ctx->module_lexers,
        "guess_lexer_for_filename");
    if (ctx->func_guess_lexer_for_filename == NULL) {
        PyErr_Clear();
        Py_DECREF(formatters_module);
        Py_DECREF(HtmlFormatter_class);
        pygments_context_close(ctx);
        return -1;
    }

    ctx->func_guess_lexer = PyObject_GetAttrString(ctx->module_lexers,"guess_lexer");
    if (ctx->func_guess_lexer == NULL) {
        PyErr_Clear();
        Py_DECREF(formatters_module);
        Py_DECREF(HtmlFormatter_class);
        pygments_context_close(ctx);
        return -1;
    }

    Py_DECREF(formatters_module);
    Py_DECREF(HtmlFormatter_class);

    pygments_context_set_default_options(ctx);

    return 0;
}

int pygments_context_close(struct pygments_context* ctx)
{
    if (ctx->module_pygments != NULL) {
        Py_DECREF(ctx->module_pygments);
        ctx->module_pygments = NULL;
    }

    if (ctx->func_highlight != NULL) {
        Py_DECREF(ctx->func_highlight);
        ctx->func_highlight = NULL;
    }

    if (ctx->module_lexers != NULL) {
        Py_DECREF(ctx->module_lexers);
        ctx->module_lexers = NULL;
    }

    if (ctx->func_get_lexer_by_name != NULL) {
        Py_DECREF(ctx->func_get_lexer_by_name);
        ctx->func_get_lexer_by_name = NULL;
    }

    if (ctx->func_guess_lexer_for_filename != NULL) {
        Py_DECREF(ctx->func_guess_lexer_for_filename);
        ctx->func_guess_lexer_for_filename = NULL;
    }

    if (ctx->func_guess_lexer != NULL) {
        Py_DECREF(ctx->func_guess_lexer);
        ctx->func_guess_lexer = NULL;
    }

    if (ctx->formatter != NULL) {
        Py_DECREF(ctx->formatter);
        ctx->formatter = NULL;
    }

    return 0;
}

int pygments_context_check(struct pygments_context* ctx)
{
    return ctx->module_pygments != NULL && ctx->func_highlight != NULL;
}

int pygments_context_options_parse(struct context_options* dst,
    zval* zfrom,
    const char* errctx)
{
    zval* zv;
    HashTable* ht;

    ht = Z_ARRVAL_P(zfrom);
    make_default_options(dst);

    zv = zend_hash_str_find(ht,"linenos",sizeof("linenos")-1);
    if (zv != NULL && zval_check_bool(&dst->linenos,zv,errctx,"linenos") == FAILURE) {
        return FAILURE;
    }

    zv = zend_hash_str_find(ht,"noclasses",sizeof("noclasses")-1);
    if (zv != NULL && zval_check_bool(&dst->noclasses,zv,errctx,"noclasses") == FAILURE) {
        return FAILURE;
    }

    zv = zend_hash_str_find(ht,"linenostart",sizeof("linenostart")-1);
    if (zv != NULL && zval_check_int(&dst->linenostart,zv,errctx,"linenostart")) {
        return FAILURE;
    }

    zv = zend_hash_str_find(ht,"lineanchors",sizeof("lineanchors")-1);
    if (zv != NULL
        && zval_check_string(&dst->lineanchors,zv,errctx,"lineanchors") == FAILURE)
    {
        return FAILURE;
    }

    zv = zend_hash_str_find(ht,"classprefix",sizeof("classprefix")-1);
    if (zv != NULL
        && zval_check_string(&dst->classprefix,zv,errctx,"classprefix") == FAILURE)
    {
        return FAILURE;
    }

    zv = zend_hash_str_find(ht,"cssclass",sizeof("cssclass")-1);
    if (zv != NULL
        && zval_check_string(&dst->cssclass,zv,errctx,"cssclass") == FAILURE)
    {
        return FAILURE;
    }

    zv = zend_hash_str_find(ht,"cssstyles",sizeof("cssstyles")-1);
    if (zv != NULL
        && zval_check_string(&dst->cssstyles,zv,errctx,"cssstyles") == FAILURE)
    {
        return FAILURE;
    }

    zv = zend_hash_str_find(ht,"prestyles",sizeof("prestyles")-1);
    if (zv != NULL
        && zval_check_string(&dst->prestyles,zv,errctx,"prestyles") == FAILURE)
    {
        return FAILURE;
    }

    return SUCCESS;
}

int pygments_context_assign_options(struct pygments_context* ctx,
    const struct context_options* opts)
{
    set_python_attribute_bool(ctx->formatter,"linenos",opts->linenos);
    set_python_attribute_int(ctx->formatter,"linenostart",opts->linenostart);
    set_python_attribute_bool(ctx->formatter,"noclasses",opts->noclasses);
    if (opts->lineanchors != NULL) {
        set_python_attribute_string(ctx->formatter,"lineanchors",opts->lineanchors);
    }
    else {
        set_python_attribute_none(ctx->formatter,"lineanchors",1);
    }

    if (opts->classprefix != NULL) {
        set_python_attribute_string(ctx->formatter,"classprefix",opts->classprefix);
    }
    else {
        set_python_attribute_none(ctx->formatter,"classprefix",1);
    }

    if (opts->cssclass != NULL) {
        set_python_attribute_string(ctx->formatter,"cssclass",opts->cssclass);
    }
    else {
        set_python_attribute_none(ctx->formatter,"cssclass",1);
    }

    if (opts->cssstyles != NULL) {
        set_python_attribute_string(ctx->formatter,"cssstyles",opts->cssstyles);
    }
    else {
        set_python_attribute_none(ctx->formatter,"cssstyles",1);
    }

    if (opts->prestyles != NULL) {
        set_python_attribute_string(ctx->formatter,"prestyles",opts->prestyles);
    }
    else {
        set_python_attribute_none(ctx->formatter,"prestyles",1);
    }

    return 0;
}

int pygments_context_set_default_options(struct pygments_context* ctx)
{
    struct context_options opts;
    make_default_options(&opts);
    return pygments_context_assign_options(ctx,&opts);
}

struct highlight_result* highlight(const struct pygments_context* ctx,const char* code,
    const struct lexer_options* opts)
{
    PyObject* pycode;
    PyObject* lexer;
    PyObject* args;
    struct highlight_result* result;

    /* Make sure the context is properly initialized. */
    if (ctx->func_highlight == NULL) {
        return NULL;
    }

    /* Allocate result structure. */
    result = malloc(sizeof(struct highlight_result));
    if (result == NULL) {
        return NULL;
    }
    memset(result,0,sizeof(struct highlight_result));

    /* Convert source code string to Python string. */
    pycode = PyUnicode_FromString(code);
    if (pycode == NULL) {
        PyErr_Clear();
        free(result);
        return NULL;
    }

    /* Get a lexer suitable for the operation. */

    lexer = lookup_lexer(ctx,pycode,opts);
    if (lexer == NULL) {
        free(result);
        Py_DECREF(pycode);
        return NULL;
    }

    /* Call pygments.highlight(). */

    args = Py_BuildValue("(OOO)",pycode,lexer,ctx->formatter);
    result->_pyobj = PyObject_CallObject(ctx->func_highlight,args);
    Py_DECREF(args);
    Py_DECREF(lexer);
    Py_DECREF(pycode);
    if (result->_pyobj == NULL) {
        if (PyErr_Occurred()) {
            PyErr_Clear();
        }

        free(result);
        return NULL;
    }

    result->html = PyUnicode_AsUTF8(result->_pyobj);
    if (result->html == NULL) {
        /* NOTE: PyString_AS_STRING() shouldn't return NULL, but we include this
         * out of paranoia.
         */
        if (PyErr_Occurred()) {
            PyErr_Clear();
        }

        free(result);
        return NULL;
    }

    return result;
}

void highlight_result_free(struct highlight_result* result)
{
    Py_DECREF(result->_pyobj);
    free(result);
}
