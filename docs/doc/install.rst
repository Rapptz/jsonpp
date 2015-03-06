.. default-domain:: gcpp
.. _doc-install:

Installation
==============

There is no immediate installation step when using this library. As mentioned in
the project's README, the entire thing is header-only. However there are some steps
to help make integration easier if needed.

To begin, you can obtain the source by doing a clone: ::

    $ git clone --recursive https://github.com/Rapptz/jsonpp.git

.. _doc_single_header:

Creating A Single Header
---------------------------

The main repository has a file called ``single.py`` which creates a single file for your convenience without having to copy the entire directory source.

Basic invocation is just ::

    $ python single.py

By default, it creates a file called ``jsonpp.hpp`` in the root directory. You can rename the file and location by passing the ``-o`` option like so: ::

    $ python single.py -o single.hpp


The single header contains no dependencies so it's for maximal ease of including.

.. _doc_make_docs:

Building Documentation
------------------------

Creating an offline back-up of the documentation is pretty simple. The only requirement to build the
documentation is to have `Sphinx <http://sphinx-doc.org/>`_ version 1.2.3 or higher.

After obtaining Sphinx, just run the make command to whatever format you want: ::

    $ cd docs
    $ make html

