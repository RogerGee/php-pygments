/*
 * highlight.c
 *
 * This file is a part of php-pygments.
 *
 * Copyright (C) 2018 Roger P. Gee
 */

#include "highlight.h"
#include <stdlib.h>

static PyObject* lookup_lexer(const struct pygments_context* ctx,
    PyObject* pycode,const struct lexer_options* opts)
{
    PyObject* lexer;
    PyObject* args;

    if (opts != NULL && opts->preferred_lexer != NULL) {
        PyObject* name = PyString_FromString(opts->preferred_lexer);
        if (name == NULL) {
            return NULL;
        }

        args = PyTuple_Pack(1,name);
        Py_DECREF(name);
        if (args == NULL) {
            return NULL;
        }

        lexer = PyObject_CallObject(ctx->func_get_lexer_by_name,args);
        Py_DECREF(args);
        if (lexer == NULL) {
            return NULL;
        }
    }
    else if (opts != NULL && opts->filename != NULL) {
        args = Py_BuildValue("(sO)",opts->filename,pycode);
        if (args == NULL) {
            return NULL;
        }

        lexer = PyObject_CallObject(ctx->func_guess_lexer_for_filename,args);
        Py_DECREF(args);
        if (lexer == NULL) {
            return NULL;
        }
    }
    else {
        args = Py_BuildValue("(O)",pycode);
        if (args == NULL) {
            return NULL;
        }

        lexer = PyObject_CallObject(ctx->func_guess_lexer,args);
        Py_DECREF(args);
        if (lexer == NULL) {
            return NULL;
        }
    }

    return lexer;
}

int pygments_context_init(struct pygments_context* ctx)
{
    PyObject* name;
    PyObject* formatters_module;
    PyObject* HtmlFormatter_class;

    memset(ctx,0,sizeof(struct pygments_context));

    /* Import modules. */

    name = PyString_FromString("pygments");
    if (name == NULL) {
        return -1;
    }

    ctx->module_pygments = PyImport_Import(name);
    Py_DECREF(name);
    if (ctx->module_pygments == NULL) {
        return -1;
    }

    name = PyString_FromString("pygments.lexers");
    if (name == NULL) {
        pygments_context_close(ctx);
        return -1;
    }

    ctx->module_lexers = PyImport_Import(name);
    Py_DECREF(name);
    if (ctx->module_lexers == NULL) {
        pygments_context_close(ctx);
        return -1;
    }

    name = PyString_FromString("pygments.formatters");
    if (name == NULL) {
        pygments_context_close(ctx);
        return -1;
    }

    formatters_module = PyImport_Import(name);
    Py_DECREF(formatters_module);
    if (formatters_module == NULL) {
        pygments_context_close(ctx);
        return -1;
    }

    /* Create formatter instance. */

    HtmlFormatter_class = PyObject_GetAttrString(formatters_module,"HtmlFormatter");
    if (HtmlFormatter_class == NULL) {
        Py_DECREF(formatters_module);
        pygments_context_close(ctx);
        return -1;
    }

    ctx->formatter = PyObject_CallObject(HtmlFormatter_class,NULL);
    if (ctx->formatter == NULL) {
        Py_DECREF(formatters_module);
        Py_DECREF(HtmlFormatter_class);
        pygments_context_close(ctx);
        return -1;
    }

    ctx->func_highlight = PyObject_GetAttrString(ctx->module_pygments,"highlight");
    if (ctx->func_highlight == NULL) {
        Py_DECREF(formatters_module);
        Py_DECREF(HtmlFormatter_class);
        pygments_context_close(ctx);
        return -1;
    }

    ctx->func_get_lexer_by_name = PyObject_GetAttrString(ctx->module_lexers,"get_lexer_by_name");
    if (ctx->func_get_lexer_by_name == NULL) {
        Py_DECREF(formatters_module);
        Py_DECREF(HtmlFormatter_class);
        pygments_context_close(ctx);
        return -1;
    }

    ctx->func_guess_lexer_for_filename = PyObject_GetAttrString(ctx->module_lexers,"guess_lexer_for_filename");
    if (ctx->func_guess_lexer_for_filename == NULL) {
        Py_DECREF(formatters_module);
        Py_DECREF(HtmlFormatter_class);
        pygments_context_close(ctx);
        return -1;
    }

    ctx->func_guess_lexer = PyObject_GetAttrString(ctx->module_lexers,"guess_lexer");
    if (ctx->func_guess_lexer == NULL) {
        Py_DECREF(formatters_module);
        Py_DECREF(HtmlFormatter_class);
        pygments_context_close(ctx);
        return -1;
    }

    Py_DECREF(formatters_module);
    Py_DECREF(HtmlFormatter_class);

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

int pygments_context_assign_options(struct pygments_context* ctx,
    const struct context_options* opts)
{
    /* TODO */

    return -1;
}

int pygments_context_set_default_options(struct pygments_context* ctx)
{
    /* TODO */

    return -1;
}

struct highlight_result* highlight(const struct pygments_context* ctx,const char* code,
    const struct lexer_options* opts)
{
    PyObject* pycode;
    PyObject* lexer;
    PyObject* args;
    struct highlight_result* result;

    result = malloc(sizeof(struct highlight_result));
    if (result == NULL) {
        return NULL;
    }

    pycode = PyString_FromString(code);
    if (pycode == NULL) {
        free(result);
        return NULL;
    }

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
        free(result);
        return NULL;
    }

    result->html = PyString_AsString(result->_pyobj);

    return result;
}

void highlight_result_free(struct highlight_result* result)
{
    Py_DECREF(result->_pyobj);
    free(result);
}
