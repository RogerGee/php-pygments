# php-pygments

This project provides a PHP extension that wraps a subset of the `pygments` Python library, streamlining it for use in PHP userspace. It uses `libpython` to load an interpreter alongside the PHP process.

## Design

This extension works by embedding a Python interpreter inside PHP. This is accomplished using `libpython2`. Currently only the `pygments.highlight` and `HtmlFormatter` class functionalities are wrapped since this was designed for dynamically generating syntax-highlighted code snippets as HTML.

The extension loads the required `pygments` modules at module initialization time. The extension maintains a global `pygments` context that is reused for each library call. This avoids having to load the `pygments` code for each request. Note, this only really provides any benefit if PHP is long-lived (meaning it doesn't call `MINIT` for each request but once  for some sequence of requests). If ZTS is enabled, the module opens multiple contexts per thread.

The global context maintains the state of several formatting options that can be configured from PHP userspace. The options are reset at the end of each request so that each request starts in the same state.

The `pygments` library must be in the python load path. This is installed externally and not configured by this extension. You can use the `PYTHONPATH` environment variable to target a custom location containing the library.

## Public API

### `void pygments_set_options(array $options)`

Sets formatter options on the global `pygments` context. The effects of this function are preserved until the next call to this function. The options are reset for each request.

* `$options`: An array of `HtmlFormatter` options to set on the formatter. Currently only the following keys are supported, and they align with the `HtmlFormatter` option names from `pygments`:

    - `bool linenos`
    - `int linenostart`
    - `string lineanchors`
    - `bool noclasses`
    - `string classprefix`
    - `string cssclass`     [default=`php-pygments`]
    - `string cssstyles`
    - `string prestyles`

### `string pygments_highlight(string $code[,string $lexer,string $filename])`

Uses the global `pygments` context to highlight the specified code. `pygments` will try to guess which lexer to use based on the contents of the source code unless you specify a lexer or, alternatively, a filename.

* `$code`: the source code to highlight

_(Note: only specify at most one of the following parameters. They are meant to be used exclusively if at all.)_

* `$lexer`: an optional lexer to force; do not use `$filename` if you use this parameter

* `$filename`: the filename associated with the source code; `pygments` will guess the lexer based on this filename; pass `null` for `$lexer` if you use this parameter; if `pygments` doesn't find a lexer via filename, then the extension will attempt to have it guess via the source code content

## Considerations

I have not tested the ZTS code yet. I'm not sure about `libpython`'s thread safety either. Also, it may run inefficiently since PHP may create a context per thread instead of sharing a pool of context's between threads. More work needs to be done with ZTS to understand this.

Use the [python valgrind suppression file](https://svn.python.org/projects/python/trunk/Misc/valgrind-python.supp) when testing for errors/memory leaks with `valgrind`.

