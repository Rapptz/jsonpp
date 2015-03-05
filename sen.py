#!/usr/bin/env python

"""Python module that generates .ninja files.

Some of the stuff here was borrowed from the main ninja project.
However this module aims to extend the bare-bones provided with a rich
python API.

Note: This file is meant to be a single file for easy inclusion for existing
build systems. Hence this file will be rather big.
"""

import textwrap
import os, sys
import datetime
import fnmatch
try:
    import cStringIO as StringIO
except ImportError:
    import io.StringIO as StringIO # python 3k compat

def escape_path(word):
    return word.replace('$ ', '$$ ').replace(' ', '$ ').replace(':', '$:')

def escape(string):
    """Escape a string such that it can be embedded into a Ninja file without
    further interpretation."""
    assert '\n' not in string, 'Ninja syntax does not allow newlines'
    # We only have one special metacharacter: '$'.
    return string.replace('$', '$$')

class NinjaWriter(object):
    def __init__(self, output, width=78):
        self.output = output
        self.width = width

    def newline(self):
        self.output.write('\n')

    def comment(self, text):
        for line in textwrap.wrap(text, self.width - 2):
            self.output.write('# ' + line + '\n')

    def variable(self, key, value, indent=0):
        if value is None:
            return
        if isinstance(value, list):
            value = ' '.join(filter(None, value))  # Filter out empty strings.
        self._line('%s = %s' % (key, value), indent)

    def pool(self, name, depth):
        self._line('pool %s' % name)
        self.variable('depth', depth, indent=1)

    def rule(self, name, command, description=None, depfile=None,
             generator=False, pool=None, restat=False, rspfile=None,
             rspfile_content=None, deps=None):
        self._line('rule %s' % name)
        self.variable('command', command, indent=1)
        if description:
            self.variable('description', description, indent=1)
        if depfile:
            self.variable('depfile', depfile, indent=1)
        if generator:
            self.variable('generator', '1', indent=1)
        if pool:
            self.variable('pool', pool, indent=1)
        if restat:
            self.variable('restat', '1', indent=1)
        if rspfile:
            self.variable('rspfile', rspfile, indent=1)
        if rspfile_content:
            self.variable('rspfile_content', rspfile_content, indent=1)
        if deps:
            self.variable('deps', deps, indent=1)

    def build(self, outputs, rule, inputs=None, implicit=None, order_only=None,
              variables=None):
        outputs = self._as_list(outputs)
        out_outputs = [escape_path(x) for x in outputs]
        all_inputs = [escape_path(x) for x in self._as_list(inputs)]

        if implicit:
            implicit = [escape_path(x) for x in self._as_list(implicit)]
            all_inputs.append('|')
            all_inputs.extend(implicit)
        if order_only:
            order_only = [escape_path(x) for x in self._as_list(order_only)]
            all_inputs.append('||')
            all_inputs.extend(order_only)

        self._line('build %s: %s' % (' '.join(out_outputs),
                                     ' '.join([rule] + all_inputs)))

        if variables:
            if isinstance(variables, dict):
                iterator = iter(variables.items())
            else:
                iterator = iter(variables)

            for key, val in iterator:
                self.variable(key, val, indent=1)

        return outputs

    def include(self, path):
        self._line('include %s' % path)

    def subninja(self, path):
        self._line('subninja %s' % path)

    def default(self, paths):
        self._line('default %s' % ' '.join(self._as_list(paths)))

    def _count_dollars_before_index(self, s, i):
        """Returns the number of '$' characters right in front of s[i]."""
        dollar_count = 0
        dollar_index = i - 1
        while dollar_index > 0 and s[dollar_index] == '$':
            dollar_count += 1
            dollar_index -= 1
        return dollar_count

    def _line(self, text, indent=0):
        """Write 'text' word-wrapped at self.width characters."""
        leading_space = '  ' * indent
        while len(leading_space) + len(text) > self.width:
            # The text is too wide; wrap if possible.

            # Find the rightmost space that would obey our width constraint and
            # that's not an escaped space.
            available_space = self.width - len(leading_space) - len(' $')
            space = available_space
            while True:
                space = text.rfind(' ', 0, space)
                if (space < 0 or
                    self._count_dollars_before_index(text, space) % 2 == 0):
                    break

            if space < 0:
                # No such space; just use the first unescaped space we can find.
                space = available_space - 1
                while True:
                    space = text.find(' ', space + 1)
                    if (space < 0 or
                        self._count_dollars_before_index(text, space) % 2 == 0):
                        break
            if space < 0:
                # Give up on breaking.
                break

            self.output.write(leading_space + text[0:space] + ' $\n')
            text = text[space+1:]

            # Subsequent lines are continuations, so indent them.
            leading_space = '  ' * (indent+2)

        self.output.write(leading_space + text + '\n')

    def _as_list(self, input):
        if input is None:
            return []
        if isinstance(input, list):
            return input
        return [input]

"""Represents the default compiler"""
class Compiler(object):
    def __init__(self, name):
        self.name = name

    def rules(self, ninja):
        ninja.rule('compile', command = '$cxx -MMD -MF $out.d -c $cxxflags $includes $depends $defines $in -o $out',
                              deps = 'gcc', depfile = '$out.d',
                              description = 'Compiling $in to $out')
        ninja.rule('link', command = '$cxx $cxxflags $in -o $out $linkflags $libpath $libraries', description = 'Creating $out')

    def object_file(self, filename):
        (root, _) = os.path.splitext(filename)
        return root + '.o'

    def include(self, path):
        return "-I\"{}\"".format(path)

    def libpath(self, path):
        return "-L\"{}\"".format(path)

    def library(self, lib):
        return "-l" + lib

    def dependency(self, dep):
        return "-isystem \"{}\"".format(dep)

    def define(self, macro):
        return "-D " + macro


def compiler(name):
    if name == 'cl' or 'msvc' in name:
        raise NotImplementedError('MSVC compilers are not supported yet')
    return Compiler(name=name)

"""Represents the base class for all sen.py classes"""
class Options(object):
    def __init__(self, *args, **kwargs):
        self.flags = None
        self.link_flags = None
        self.libraries = None
        self.library_paths = None
        self.dependencies = None
        self.includes = None
        self.defines = None

"""Represents an executable to be made."""
class Executable(Options):
    def __init__(self, name, **kwargs):
        Options.__init__(self, kwargs)
        self.name = name # the executable name
        self.builddir = kwargs.get('builddir', None) # the executable specific output directory
        self.objdir   = kwargs.get('objdir', None)   # the executable specific object directory
        self.target   = kwargs.get('target', None)
        self.files    = None
        self.run      = kwargs.get('run', None) # the name of the target that runs the executable

# monadic functions
def to_flags(flags):
    if flags == None:
        return None
    return ' '.join(flags)

def fmap(function, iterable):
    if iterable == None:
        return None
    return map(function, iterable)

# utility functions
def generate_files_from(directory, glob):
    for root, directories, files in os.walk(directory):
        for f in files:
            if fnmatch.fnmatch(f, glob):
                yield os.path.join(root, f)

def files_from(directory, glob):
    return list(generate_files_from(directory, glob))

"""Represents a project. Usually only one is needed"""
class Project(Options):
    def __init__(self, name, compiler, **kwargs):
        Options.__init__(self, kwargs)
        self.name = name
        self.compiler = compiler
        self.executables = []
        self.sstream = StringIO.StringIO()
        self.ninja = NinjaWriter(self.sstream)
        self.builddir = kwargs.get('builddir', '.')
        self.objdir   = kwargs.get('objdir', '.')

    def __create_variable_map(self, options):
        return {
            'cxxflags': to_flags(options.flags),
            'includes': to_flags(fmap(self.compiler.include, options.includes)),
            'depends': to_flags(fmap(self.compiler.dependency, options.dependencies)),
            'defines': to_flags(fmap(self.compiler.define, options.defines)),
            'linkflags': to_flags(options.link_flags),
            'libpath': to_flags(fmap(self.compiler.libpath, options.library_paths)),
            'libraries': to_flags(fmap(self.compiler.library, options.libraries))
        }

    def variables(self, **kwargs):
        for k, v in kwargs.items():
            if v == None:
                self.ninja.variable(k, ' ')
            else:
                self.ninja.variable(k, v)

    def add_executable(self, exe):
        if not isinstance(exe, Executable):
            raise ValueError("argument passed must be an 'Executable'")
        self.executables.append(exe)

    def dump(self, stream, generator=True):
        # create the global variables needed
        today = datetime.datetime.today()
        self.ninja.comment("This file was generated automatically on {} using"
                           " the sen.py meta build system. Do not modify this.".format(today.isoformat(' ')))
        self.ninja.variable('ninja_required_version', '1.3')
        self.ninja.variable('cxx', self.compiler.name)
        self.ninja.variable('builddir', self.builddir) # the project builddir

        # process the global variables
        project_variables = self.__create_variable_map(self)
        self.variables(**project_variables)

        # register the global rules
        self.compiler.rules(self.ninja)
        if generator == True:
            self.ninja.rule('bootstrap', command=' '.join(['python'] + sys.argv), generator=True)
            self.ninja.build(stream.name, 'bootstrap', implicit=sys.argv[0])

        # register the executables requested
        for exe in self.executables:
            if exe.files == None:
                continue

            exe_variables = self.__create_variable_map(exe)

            # compile the files in the project
            object_files = []
            objdir = exe.objdir if exe.objdir else self.objdir
            for filename in exe.files:
                obj = os.path.join(objdir, self.compiler.object_file(filename))
                self.ninja.build(obj, 'compile', inputs=filename, variables=exe_variables)
                object_files.append(obj)

            # final link step
            builddir = exe.builddir if exe.builddir else self.builddir
            self.ninja.build(os.path.join(builddir, exe.name), 'link', inputs=object_files, variables=exe_variables)
            if exe.target != None:
                self.ninja.build(exe.target, 'phony', inputs=os.path.join(builddir, exe.name))

            # register the runner
            if exe.run != None:
                self.ninja.rule(exe.name + '_runner', command=os.path.join(builddir, exe.name))
                self.ninja.build(exe.run, exe.name + '_runner', implicit=exe.target)

        # finalise the creation
        result = self.sstream.getvalue()
        stream.write(result)
        self.sstream = StringIO.StringIO()
        self.ninja = NinjaWriter(self.sstream)
