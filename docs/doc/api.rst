.. namespace:: json
.. _doc_api:

API Reference
================

.. |value| replace:: :class:`value`

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

.. _doc_api_value:

JSON Value
--------------

.. class:: value

    The class that represents a JSON value according to the official JSON spec, |json|_.

    Throughout the documentation of this class, there are six different types that
    the |value| class can hold:

    - null
    - object
    - array
    - string
    - boolean
    - number

    It can only hold a single one of these types at a time.

    .. function:: value() noexcept
                  value(null) noexcept

        Constructs a |value| with a null value.

        :post condition: Internal type is null.
    .. function:: value(double val) noexcept

        Constructs a |value| with the number value provided.

        :post condition: Internal type is number.
    .. function:: value(bool b) noexcept

        Constructs a |value| with the boolean value provided.

        :post condition: Internal type is boolean.
    .. function:: value(const std::string& str)

        Constructs a |value| with the string value provided.

        :post condition: Internal type is string.
    .. function:: value(const array& arr)
                  value(std::initializer_list<value> l)

        Constructs a |value| with the array value provided.

        This allows you to use initializer lists to construct a |value| directly. e.g. ::

            json::value x = { 1, 2, "hello", nullptr }; // OK


        :post condition: Internal type is array.
    .. function:: value(const object& obj)

        Constructs a |value| with the object value provided.

        :post condition: Internal type is object.
    .. function:: value(const T& t)
                  value& operator=(const T& t)

        Constructs a |value| from a *serialisable* type. The Internal
        type depends on the internal type of the resulting serialised object.
    .. function:: value(const value& other)
                  value& operator=(const value& other)

        Copies one value to another value.

        :post condition: The resulting internal type is the same as the ``other`` parameter.
    .. function:: value(value&& other) noexcept
                  value& operator=(value&& other) noexcept

        Moves one value to another value. The ownership of the contents owned by
        ``other`` are transferred over.

        :post condition: The resulting internal type is the same as the ``other`` parameter.
    .. function:: std::string type_name() const

        Returns a string representation of the internal type.

        +---------------+-----------------+
        | Internal Type | Returned String |
        +===============+=================+
        | array         | "array"         |
        +---------------+-----------------+
        | string        | "string"        |
        +---------------+-----------------+
        | object        | "object"        |
        +---------------+-----------------+
        | number        | "number"        |
        +---------------+-----------------+
        | boolean       | "boolean"       |
        +---------------+-----------------+
        | null          | "null"          |
        +---------------+-----------------+

    .. function:: void clear() noexcept

        Deletes all internal storage being held.

        :post condition: Internal type is null.
    .. function:: bool is<T>() const noexcept

        Checks if the internal type is one of the C++ types provided.

        +---------------+---------------------------------------------------+
        | Internal Type |                  Valid C++ Types                  |
        +---------------+---------------------------------------------------+
        | array         | :type:`json::array`                               |
        +---------------+---------------------------------------------------+
        | object        | :type:`json::object`                              |
        +---------------+---------------------------------------------------+
        | null          | :type:`json::null`                                |
        +---------------+---------------------------------------------------+
        | boolean       | ``bool``                                          |
        +---------------+---------------------------------------------------+
        | number        | All integral types such as ``int`` and ``float``. |
        +---------------+---------------------------------------------------+
        | string        | ``const char*`` or ``std::string``.               |
        +---------------+---------------------------------------------------+

        Anything else would return ``false`` but due to :issue:`10` a compiler
        error occurs instead.
    .. function:: T as<T>() const noexcept

        Returns a copy of the internal value being held. For type equivalences,
        check the table for :func:`is\<T>`.

        Similarly, an invalid type should throw an exception but due to :issue:`10` a
        compiler error is issued instead.

        .. note::

            This function uses the :cpp:`assert <error/assert>` macro to check the preconditions.

        :precondition: :func:`is\<T>` must return ``true``.

    .. function:: T as<T>(T&& default) const noexcept

        Similar to :func:`as\<T>` but if :func:`is\<T>` is false, then the
        default value is returned.
    .. function:: value operator[](const std::string& str) const noexcept

        Accesses the object at a given string key. If the |value| internal type
        is not :type:`json::object` or the key is not found, then a |value| with
        an internal type of :type:`json::null` is returned instead.

        Example: ::

            json::value x = json::object{
                { "key", 10 }, { "name", "bob" }
            };

            std::cout << x["key"].is<int>() << '\n'; // prints 1

    .. function:: value operator[](const Integral& index) const noexcept

        Accesses the array at a given index. If the |value| internal type
        is not :type:`json::array` or the index is out of bounds, then a |value| with
        an internal type of :type:`json::null` is returned instead. Index starts at 0.

        Example: ::

            json::value x = { 1, 2, 3, 4, 5, 6 };
            for(unsigned i = 0; i < 6; ++i) {
                std::cout << x[i].is<int>(); // prints 111111
            }

Along with the :class:`value` class, several type aliases are provided for other JSON types:

.. type:: null

    Represents ``std::nullptr_t``. For example, ``json::value x = nullptr;`` is valid.

.. type:: array

    Represents ``std::vector<json::value>``.

.. type:: object

    Represents ``std::map<std::string, json::value>``.

.. _doc_api_parsing:

Parsing JSON
----------------

The API for parsing is composed of two functions and a class. The class does not have to actually be instantiated in the
usual cases since the two free functions handle the creation of the parser for you.

.. class:: parser

    Represents a JSON parser. This typically doesn't need to be instantiated and you should use :func:`json::parse` instead.

    The parser is not destructive and is implemented as a recursive descent parser.

    .. function:: parser(const char* str) noexcept

        Creates a parser from a string already in memory. The lifetime of the string must be longer than the actual
        :class:`parser` object. The string must be a valid JSON string or an exception will be thrown when parsing.
        The constructor does not parse anything -- it just sets up the parser state.
    .. function:: void parse(value& val)

        Parses the JSON string. If an error occurs during parsing then an error is thrown. Currently there are a couple
        of assumptions on the parsing state:

        - The internal string is UTF-8 encoded.
        - ``nan`` and ``inf`` are allowed.
        - Comments are disallowed currently. See :issue:`3`.

        The rest follows the |json|_ specification.

        :throws parser_error: Thrown if a parsing error has occurred.

.. function:: void parse(const std::string& str, value& val)

    Parses a JSON string. Equivalent to constructing a :class:`parser` and then using the :func:`parser::parse` function.
.. function:: void parse(std::istream& in, value& val)

    Retrieves the :cpp:`rdbuf <io/basic_ios/rdbuf>` of the :cpp:`std::istream <io/basic_istream>` to construct a string
    and parses the resulting string as JSON.

.. _doc_api_dumping:

Dumping JSON
---------------

There's an API in place to dump (i.e. serialise) C++ objects into JSON constructs.

.. class:: format_options

    This class specifies the behaviour used when dumping JSON with the :func:`json::dump` interface.

    .. enum:: flag_type : int

        A regular enum (i.e. not an ``enum class``) that specifies flags for use with the :member:`flags` member.

        .. enumerator:: none

            The default value for :member:`flags`. Specifies that no special formatting will occur.
        .. enumerator:: allow_nan_inf

            Allow ``nan`` and ``inf`` values to be printed in the resulting JSON. If this is not set and
            ``nan`` and ``inf`` are spotted, then "null" is printed instead.
        .. enumerator:: minify

            Minifies the resulting JSON by suppressing newlines from being printed and indentation.
        .. enumerator:: escape_multi_byte

            UTF-8 codepoints that are greater than ``0x7F`` will be escaped into their proper UTF-16
            codepoint with surrogate pairs included.

    .. member:: int flags

        Specifies the special format behaviour for the family of :func:`json::dump` functions. Defaults to
        :enum:`none`.

    .. member:: int indent

        Specifies how many spaces to indent when pretty printing the output. e.g. with an indent value of
        ``4`` then the JSON ``{ "key": 10 }`` would be dumped as: ::

            {
                "key": 10
            }

        Defaults to 4.
    .. member:: int precision

        The default precision when printing number types. The default value is 6.


.. function:: OStream& dump(OStream& out, const T& t, const format_options& options)

    Dumps a C++ object to JSON with the specified options and a valid :cpp:`std::ostream <io/basic_ostream>` instance
    such as ``std::cout`` or ``std::ofstream``.

    - If the type is a string, then it will be printed as a string such as ``"Hello"``.
    - If the type is :type:`null`, then it will print ``null``.
    - If the type is an integral type, then it will print it with the specified precision of :member:`format_options::precision`.
    - If the type is a container with ``begin`` and ``end`` defined then it will be printed as an array. The values of the array
      will be dumped recursively with :func:`json::dump`.
    - If the type is a container with ``begin`` and ``end`` defined and has a pair-like ``value_type`` then it will be printed
      as an object. The key (i.e. ``p.first``) will be dumped in accordance to :func:`json::key` while the value will be
      recursively called with :func:`json::dump`.
    - If the type is :class:`value` then it will print with the above in mind with its internal value.
    - If the type has ``to_json`` then it will call it and then dump the resulting value recursively.

.. function:: void key(OStream& out, const T& t, const format_options& options)

    Dumps the key type of object types. Only defined for integral types and strings.

    - The integral type is turned into a string calling :cpp:`std::to_string <string/basic_string/to_string>`.
    - The string type is just forwarded to :func:`json::dump`.
