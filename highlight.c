/*
 * highlight.c
 *
 * This file is a part of php-pygments.
 *
 * Copyright (C) 2018 Roger P. Gee
 */

#include "highlight.h"

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

    if (ctx->module_lexers != NULL) {
        Py_DECREF(ctx->module_lexers);
        ctx->module_lexers = NULL;
    }

    if (ctx->formatter != NULL) {
        Py_DECREF(ctx->formatter);
        ctx->formatter = NULL;
    }

    return 0;
}

int pygments_context_assign_options(struct pygments_context* ctx,const struct context_options* opts)
{
    /* TODO */

    return -1;
}

int pygments_context_set_default_options(struct pygments_context* ctx)
{
    /* TODO */

    return -1;
}

const char* highlight(const struct pygments_context* ctx,const char* code)
{
    /* TODO */

    return NULL;
}
