#!/usr/bin/env python

import sen as senpai
import os, sys
import argparse

# command line stuff
parser = argparse.ArgumentParser(usage='%(prog)s [options...]')
parser.add_argument('--debug', action='store_true', help='compile with debug flags')
parser.add_argument('--cxx', metavar='<compiler>', help='compiler name to use (default: g++)', default='g++')
parser.add_argument('--quiet', '-q', action='store_true', help='suppress warning output')
parser.add_argument('--use-libcxx', action='store_true', help='compile with libc++')
parser.add_argument('--libcxx-include-dir', help='the include directory for libc++', default='/usr/include/c++/v1/')
parser.add_argument('--libcxx-lib-dir', help='the lib directory for libc++', default='/usr/lib/')
args = parser.parse_args()

project = senpai.Project(name='jsonpp', compiler=senpai.compiler(args.cxx), builddir='bin', objdir='obj')
project.includes = ['.', 'jsonpp']
project.dependencies = [os.path.join('Catch', 'single_include')]
S = senpai.Executable(name='tests', target='build', run='run')
S.files = senpai.files_from('tests', '*.cpp')

def warning(string):
    if not args.quiet:
        print('warning: {}'.format(string))

# configuration
if 'g++' not in args.cxx:
    warning('compiler not explicitly supported: {}'.format(args.cxx))

cxxflags = ['-Wall', '-Wextra', '-pedantic', '-std=c++11', '-Wno-switch']

if args.debug:
    cxxflags.extend(['-g', '-O0', '-DDEBUG'])
else:
    cxxflags.extend(['-DNDEBUG', '-O3'])

if args.cxx == 'clang++':
    cxxflags.extend(['-Wno-constexpr-not-const', '-Wno-unused-value', '-Wno-mismatched-tags'])

if args.use_libcxx:
    project.libraries = ['c++']
    cxxflags.append('-stdlib=libc++')
    project.dependencies.append(args.libcxx_include_dir)
    project.library_paths = [args.libcxx_lib_dir]

project.flags = cxxflags
project.add_executable(S)
project.dump(open('build.ninja', 'w'))
