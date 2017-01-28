#!/usr/bin/env python

import argparse
import os, sys
import re
import datetime as dt

import io

description = "Converts jsonpp to a single file for convenience."

# command line parser
parser = argparse.ArgumentParser(usage='%(prog)s [options...]', description=description)
parser.add_argument('--output', '-o', help='output directory to place file in', metavar='dir')
parser.add_argument('--quiet', help='suppress all output', action='store_true')
parser.add_argument('--strip', '-s', help='strip all doc comments from output', action='store_true')
args = parser.parse_args()

script_path = os.path.dirname(os.path.realpath(__file__))
working_dir = os.getcwd()
os.chdir(script_path)

intro = """// The MIT License (MIT)

// Copyright (c) 2014-2016 Danny Y.

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// This file was generated with a script.
// Generated {time} UTC
// This header was generated with jsonpp {version} (revision {revision})
// https://github.com/Rapptz/jsonpp

#ifndef {guard}
#define {guard}

"""

module_path = os.path.join(script_path)

includes = set([])
standard_include = re.compile(r'#include <(.*?)>')
local_include = re.compile(r'#include "(.*?)"')
ifndef_cpp = re.compile(r'#ifndef JSONPP_.*?_HPP')
define_cpp = re.compile(r'#define JSONPP_.*?_HPP')
endif_cpp = re.compile(r'#endif // JSONPP_.*?_HPP')

def get_include(line, base_path):
    local_match = local_include.match(line)
    if local_match:
        # local include found
        full_path = os.path.normpath(os.path.join(base_path, local_match.group(1))).replace('\\', '/')
        return full_path

    return None


def is_include_guard(line):
    return ifndef_cpp.match(line) or define_cpp.match(line) or endif_cpp.match(line)

def get_revision():
    return os.popen('git rev-parse --short HEAD').read().strip()

def get_version():
    return os.popen('git describe --tags --abbrev=0').read().strip()

def process_file(filename, out):
    global includes

    if filename in includes:
        return

    includes.add(filename)

    if not args.quiet:
        print('processing {}'.format(filename))

    if not args.strip:
        out.write('// beginning of {}\n\n'.format(filename))

    empty_line_state = True

    with io.open(filename, 'r', encoding='utf-8') as f:
        for line in f:
            # skip comments
            if line.startswith('//'):
                continue

            # skip include guard non-sense
            if is_include_guard(line):
                continue

            # get relative directory
            base_path = os.path.dirname(filename)

            # check if it's a standard file
            std = standard_include.search(line)
            if std:
                std_file = os.path.join('std', std.group(0))
                if std_file in includes:
                    continue
                includes.add(std_file)

            # see if it's an include file
            name = get_include(line, base_path)

            if name:
                process_file(name, out)
                continue

            empty_line = len(line.strip()) == 0

            if empty_line and empty_line_state:
                continue

            empty_line_state = empty_line

            # line is fine
            out.write(line)

    if not args.strip:
        out.write('// end of {}\n\n'.format(filename))


version = get_version()
revision = get_revision()
include_guard = 'JSONPP_SINGLE_INCLUDE_HPP'

if not args.quiet:
    print('Creating single header for jsonpp')
    print('Current version: {version} (revision {revision})\n'.format(version=version, revision=revision))


single_file = 'jsonpp.hpp'
processed_file = os.path.join(script_path, 'jsonpp', 'parser.hpp')
output_path = '.'

if args.output:
    output_path = os.path.normpath(os.path.join(working_dir, args.output))
    if not os.path.exists(output_path):
        os.makedirs(output_path)

output_file = os.path.join(output_path, single_file)

new_intro = intro.format(time=dt.datetime.utcnow(), revision=revision, version=version, guard=include_guard)
outro = '#endif // {}\n'.format(include_guard)

with io.StringIO() as ss:
    process_file(processed_file, ss)
    result = ss.getvalue()

if args.strip:
    result = re.sub(r'\/\*\*.*?\*\/', '', result, flags=re.DOTALL)

result = new_intro + result + outro

with io.open(output_file, 'w', encoding='utf-8') as f:
    f.write(result)

