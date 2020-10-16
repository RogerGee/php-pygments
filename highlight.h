/*
 * highlight.h
 *
 * This file is a part of php-pygments.
 *
 * Copyright (C) Roger P. Gee
 */

#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H

#include <Python.h>
#include <php.h>

#define PHP_PYGMENTS_DEFAULT_CSSCLASS "php-pygments"

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
     * into the output.
     */
    int linenos;

    /* If line_numbers is non-zero, then this specifies the first line number
     * for the output region.
     */
    int linenostart;

    /* If non-empty, then the HTML formatter will produce anchor attributes for
     * each code line. Each anchor will contain the specified prefix string.
     */
    const char* lineanchors;

    /* If non-zero, then the HTML formatter will produce inline style
     * definitions.
     */
    int noclasses;

    /*
     * If non-empty, then the HTML formatter will include the following prefix
     * string for each CSS class it uses.
     */
    const char* classprefix;

    /*
     * If non-empty, then the HTML formatter will use the specified class name
     * for the outer <div> element. Default is set to PHP_PYGMENTS_DEFAULT_CSSCLASS.
     */
    const char* cssclass;

    /*
     * If non-empty, then the HTML formatter will use the specified style string
     * for the outer div element.
     */
    const char* cssstyles;

    /*
     * If non-empty, then the HTML formatter will use the specified style string
     * for the outer <pre> element.
     */
    const char* prestyles;
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

/* Determines if the context is valid. */
int pygments_context_check(struct pygments_context* ctx);

/* Parse the context options from the specified zval. An E_ERROR will be issued
 * if parsing fails.
 */
void pygments_context_options_parse(struct context_options* dst,zval* zfrom,
    const char* errctx);

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

/* Frees the result of a call to highlight(). */
void highlight_result_free(struct highlight_result* result);

#endif
