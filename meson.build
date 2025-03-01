project('ajns', 'cpp', default_options : ['cpp_std=c++17', 'buildtype=release', 'optimization=3'])

# Defina os diretórios de include
inc_dirs = include_directories([
  '/home/evellyn/cplex2211/cplex/include',
  '/home/evellyn/cplex2211/concert/include',
  '/home/evellyn/boost_1_83_0'
])

# Adicione os arquivos de origem
src_files = [
  'preprocessing/problem_data.h',
  'utils/const.h',
  'utils/validation.h',
  'base/graph.h',
  'base/instance.h',
  'base/solution.cpp',
  'heuristic/minimal_extension.cpp',
  'optimizer/feasibility_characterization.cpp',
  'optimizer/exponential.cpp',
  'optimizer/optimizer.cpp',
  'optimizer/multiflow.cpp',
  'optimizer/iterative.cpp',
  'optimizer/optimizer_creator.cpp',
  'preprocessing/instance_generator.cpp',
  'preprocessing/reader.cpp',
  'main.cpp'
]

# Obtenha o compilador C++
cpp = meson.get_compiler('cpp')

# Defina as bibliotecas a serem vinculadas
ilocplex = cpp.find_library('ilocplex', dirs: ['/home/evellyn/cplex2211/cplex/lib/x86-64_linux/static_pic'])
concert = cpp.find_library('concert', dirs: ['/home/evellyn/cplex2211/concert/lib/x86-64_linux/static_pic'])
cplex = cpp.find_library('cplex', dirs: ['/home/evellyn/cplex2211/cplex/lib/x86-64_linux/static_pic'])


# Crie o executável
executable('ajns', src_files,
  include_directories: inc_dirs,
  dependencies: [ilocplex, concert, cplex],
  link_args: ['-lm', '-lpthread', '-ldl']
)




# project('ajns', 'cpp',
#   version : '0.1',
#   default_options : ['warning_level=3', 'cpp_std=c++14'])

# boost_inc = include_directories('/home/evellyn/boost_1_83_0')

# boost_dep = declare_dependency(
#     include_directories: boost_inc)

# thread_dep = dependency('threads')
# cpp = meson.get_compiler('cpp')
# libdl = cpp.find_library('dl')
# libm = cpp.find_library('m')




# executable('ajns',
#            'main.cpp',
#            install : false,
#            include_directories: ['/home/evellyn/boost_1_83_0','/home/evellyn/cplex_22/cplex/include', 
#            '/home/evellyn/cplex_22/concert/include'],
#            dependencies : [boost_dep, libdl, thread_dep, libm],
#            link_args : ['-L/home/evellyn/cplex_22/cplex/lib/x86-64_linux/static_pic', 
#            '-L/home/evellyn/cplex_22/concert/lib/x86-64_linux/static_pic', '-lilocplex',
#            '-lconcert', '-lcplex', '-m64']
#            )
# boost = cpp.find_library('boost', dirs: ['/home/evellyn/boost_1_83_0'])