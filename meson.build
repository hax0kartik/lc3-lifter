project('lc3', 'cpp',
  version : '0.1',
  default_options : ['warning_level=2', 'cpp_std=c++20'])

llvm = dependency('llvm', required: true)

executable('lc3',
           'src/disass.hh',
           'src/instrs.hh',
           'src/ir.hh',
           'src/disass.cpp',
           'src/main.cpp',
           install : true,
           dependencies : llvm)
