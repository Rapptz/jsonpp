#!/usr/bin/env python

import ninja_syntax
import os, sys, glob
import itertools
import argparse

# utilities
def flags(*args):
    return ' '.join(itertools.chain(*args))

def includes(l):
    return ['-I"{}"'.format(x) for x in l]

def dependencies(l):
    return ['-isystem "{}"'.format(x) for x in l]

# command line stuff
parser = argparse.ArgumentParser()
parser.add_argument('--debug', action='store_true', help='compile with debug flags')
parser.add_argument('--cxx', metavar='<compiler>', help='compiler name to use (default: g++)', default='g++')
args = parser.parse_args()

# general variables
include = [ '.', 'jsonpp' ]
depends = ['tests', os.path.join('Catch', 'single_include')]
cxxflags = [ '-Wall', '-Werror', '-Wextra', '-pedantic', '-std=c++11' ]

if args.debug:
    cxxflags.extend(['-g', '-O0', '-DDEBUG'])
else:
    cxxflags.extend(['-DNDEBUG', '-O3'])

builddir = 'bin'
objdir = 'obj'
tests = os.path.join(builddir, 'tests')

def object_file(f):
    (root, ext) = os.path.splitext(f)
    return os.path.join(objdir, root + '.o')

# ninja file
ninja = ninja_syntax.Writer(open('build.ninja', 'w'))

# variables
ninja.variable('ninja_required_version', '1.3')
ninja.variable('builddir', 'bin')
ninja.variable('cxx', args.cxx)
ninja.variable('cxxflags', flags(cxxflags + includes(include) + dependencies(depends)))

# rules
ninja.rule('bootstrap', command = ' '.join(['python'] + sys.argv), generator = True)
ninja.rule('compile', command = '$cxx -MMD -MF $out.d -c $cxxflags $in -o $out',
                      deps = 'gcc', depfile = '$out.d',
                      description = 'Compiling $in to $out')
ninja.rule('link', command = '$cxx $cxxflags $in -o $out', description = 'Creating $out')
ninja.rule('runner', command = tests)

# builds
ninja.build('build.ninja', 'bootstrap', implicit = sys.argv[0])

object_files = []
for f in glob.glob('tests/*.cpp'):
    obj = object_file(f)
    object_files.append(obj)
    ninja.build(obj, 'compile', inputs = f)

ninja.build(tests, 'link', inputs = object_files)
ninja.build('install', 'phony', inputs = tests)
ninja.build('run', 'runner', implicit = 'install')
ninja.default('run')
