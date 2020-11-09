#
# YaPB - Counter-Strike Bot based on PODBot by Markus Klinge.
# Copyright © 2004-2020 YaPB Project <yapb@jeefo.net>.
#
# SPDX-License-Identifier: MIT
#

project (
    'botsetup',
    'cpp',
    
    version: '2.0',
    license: 'MIT',
    
    default_options: [
        'buildtype=release',
        'b_ndebug=if-release',
        'cpp_std=c++14',
        'warning_level=0',
        'werror=false',
        'backend=ninja',
        'strip=true',
        'optimization=3',
        'default_library=static',
        'cpp_eh=none',
        'strip=true'
    ],
    meson_version: '>=0.48.0')


flagsLinker = []
flagsCompiler = []

flagsCompiler += [
   '-m32',
   '-fno-threadsafe-statics',
   '-fno-exceptions',
   '-fno-rtti',
   '-fdata-sections',
   '-ffunction-sections',
   '-flto',
   '-fvisibility-inlines-hidden'
]

flagsLinker += [
   '-flto',
   '-static-libgcc',
   '-static',
   '-Wl,--gc-sections'
]

flagsLinker += [
   '-luser32',
   '-lws2_32',
   '-lgdiplus',
   '-limagehlp',
   '-lcomctl32'
]

add_global_arguments (flagsCompiler, language: 'cpp')
add_global_link_arguments (flagsLinker, language: 'cpp')

sources = files (
    'src/botsetup.cpp',
    'ext/unzip/unzip.cpp'
)

includes = include_directories ([
    '.', 'res', 'ext', 'ext/crlib'
], is_system: true)


sources += import('windows').compile_resources (
   'res/botsetup.rc', 
   include_directories: includes, 
)

executable (
    meson.project_name (), 
    sources,
    include_directories: includes,
    gnu_symbol_visibility: 'hidden',
    gui_app : true)
