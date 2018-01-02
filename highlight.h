/*
 * highlight.h
 *
 * This file is a part of php-pygments.
 *
 * Copyright (C) 2018 Roger P. Gee
 */

#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H

#include <Python.h>

/*
 * pygments_context
 *
 * Defines a global context for using the Pygments library. It is meant to be
 * reused between one or more calls. Option fields may be modified to customize
 * a particular usage.
 */

struct pygments_context
{
    /* The pygments module */
    PyObject* module_pygments;

    /* The pygments.lexers module */
    PyObject* module_lexers;

    /* A pygments.formatters.HtmlFormatter instance */
    PyObject* formatter;
};

/* context_options
 *
 * The following struct defines an interface for customizing the behavior of a
 * pygments_context. The exposed options correspond directly to a subset of the
 * HtmlFormatter options from the pygments library.
 */

struct context_options
{
    /* If non-zero, then the HTML formatter will insert line number elements
     * into the output. [HtmlFormatter.linenos]
     */
    int line_numbers;

    /* If line_numbers is non-zero, then this specifies the first line number
     * for the output region. [HtmlFormatter.linenostart]
     */
    int line_start;

    /* If non-NULL, then the HTML formatter will produce anchor attributes for
     * each code line. Each anchor will contain the specified prefix
     * string. [HtmlFormatter.lineanchors]
     */
    const char* line_anchor_prefix;

    /* If non-zero, then the HTML formatter will produce inline style
     * definitions. [HtmlFormatter.noclasses]
     */
    int inline_styles;

    /*
     * If non-NULL, then the HTML formatter will include the following prefix
     * string for each CSS class it uses.
     */
    const char* css_prefix;

    /*
     * If non-NULL, then the HTML formatter will use the specified class name
     * for the outer <div> element. Default is set to php-pygments.
     * [HtmlFormatter.cssclass]
     */
    const char* div_css_class;

    /*
     * If non-NULL, then the HTML formatter will use the specified style string
     * for the outer div element. [HtmlFormatter.cssstyles]
     */
    const char* div_css_styles;

    /*
     * If non-NULL, then the HTML formatter will use the specified style string
     * for the outer <pre> element. [HtmlFormatter.prestyles]
     */
    const char* pre_css_styles;
};

/* Creates a new pygments context for use by the PHP extension. */
int pygments_context_init(struct pygments_context* ctx);

/* Frees the context's members. The context cannot be used after this call. */
int pygments_context_close(struct pygments_context* ctx);

/* Assigns the specified options to the context's formatter instance. */
int pygments_context_assign_options(struct pygments_context* ctx,const struct context_options* opts);

/* Resets the context's formatter options to defaults. */
int pygments_context_set_default_options(struct pygments_context* ctx);

/* This is the core function that wraps the calls into the Pygments library for
 * syntax highlighting.
 */
const char* highlight(const struct pygments_context* ctx,const char* code);

#endif
