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
    PyObject* func_highlight;

    /* The pygments.lexers module */
    PyObject* module_lexers;
    PyObject* func_get_lexer_by_name;
    PyObject* func_guess_lexer_for_filename;
    PyObject* func_guess_lexer;

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

/*
 * lexer_options
 *
 * Options to customize the lexer used to produce syntax-highlighted html in a
 * call to highlight().
 */

struct lexer_options
{
    /* The name of the lexer to use. If set, then this will force Pygments to
     * use this lexer.
     */
    const char* preferred_lexer;

    /* The filename to use when guessing the lexer. */
    const char* filename;
};

/*
 * highlight_result
 *
 * Abstracts the PyObject string that represents the result of calling
 * pygments.highlight().
 */

struct highlight_result
{
    const char* html;

    PyObject* _pyobj;
};

/* Creates a new pygments context for use by the PHP extension. */
int pygments_context_init(struct pygments_context* ctx);

/* Frees the context's members. The context cannot be used after this call. */
int pygments_context_close(struct pygments_context* ctx);

/* Assigns the specified options to the context's formatter instance. */
int pygments_context_assign_options(struct pygments_context* ctx,
    const struct context_options* opts);

/* Resets the context's formatter options to defaults. */
int pygments_context_set_default_options(struct pygments_context* ctx);

/* This is the core function that wraps the calls into the Pygments library for
 * syntax highlighting.
 */
struct highlight_result* highlight(const struct pygments_context* ctx,const char* code,
    const struct lexer_options* opts);

void highlight_result_free(struct highlight_result* result);

#endif
