.. default-domain:: gcpp
.. highlight:: cpp
.. namespace:: json
.. _doc_api:

API Reference
================

The API of jsonpp is largely inspired by the ease of use of the Python
`json module <https://docs.python.org/2/library/json.html>`_. I considered the API of that module to be easy
enough to use and understand in many ways but not in others. The goal of this library was to fix some of the
issues with the json module and to allow the same ease of use in a C++ setting with static typing.


.. _doc_api_error:

Error Handling
------------------

The parsing part of the library throws an error called :class:`parser_error`.

.. class:: parser_error : public std::exception

    The exception type used to report parsing errors.

    .. function:: parser_error(const std::string& str, unsigned line, unsigned column)

        Constructs a parser error.

        :param str: The error string.
        :param line: The line the error occurred in.
        :param column: The column the error occurred in.

    .. function:: const char* what() const noexcept

        Returns the string representation of the error.
