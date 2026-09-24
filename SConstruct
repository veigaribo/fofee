import os

options = Variables('.env')
options.Add(EnumVariable(
  'DEBUG',
  help='Build with `-g3`.',
  allowed_values=['yes', 'no'],
  default='yes',
))
options.Add(EnumVariable(
  'ASAN',
  help='Build with `fsanitize=address`.',
  allowed_values=['yes', 'no'],
  default='yes',
))
options.Add(EnumVariable(
  'UDEV',
  help='Build with `udev`. Used to fetch device information.',
  allowed_values=['yes', 'no', 'detect'],
  default='detect',
))

include = ['#.']

env = Environment(variables=options, CPPPATH=include)
Export('env')

Help(options.GenerateHelpText(env))

home = os.environ.get('HOME')
if home is not None:
    include.append(f'{home}/.local/include')

env.PrependENVPath('PATH', os.getenv('PATH'))
env.PrependENVPath('HOME', os.getenv('HOME'))

conf = Configure(env)

if env['UDEV'] == 'detect':
  if conf.CheckLib('systemd'):
    env['UDEV'] = 'yes'
  else:
    env['UDEV'] = 'no'

env.Tool('compilation_db')
env.CompilationDatabase()

env.VariantDir('build/debug', '.')
env.VariantDir('build/release', '.')
env.VariantDir('build/test', '.')

c_flags = []
link_flags = []
libs = ['m']

if env['DEBUG'] == 'yes':
  c_flags.append('-g3')
  c_flags.append('-fno-omit-frame-pointer')
  main_variant = 'build/debug'
else:
  c_flags.append('-O3')
  c_flags.append('-flto')
  main_variant = 'build/release'

if env['ASAN'] == 'yes':
  c_flags.append('-fsanitize=address')
  link_flags.append('-fsanitize=address')

if env['UDEV'] == 'yes':
  c_flags.append('-DUDEV')
  libs.append('systemd')

env.Program(f'{main_variant}/fofee',
            Glob(f'{main_variant}/*.c', exclude=f'{main_variant}/test_*'),
            LIBS=libs,
            CFLAGS=c_flags,
            LINKFLAGS=link_flags)


env.Program('build/test/units',
            ['build/test/test_units.c', 'build/test/units.c'],
            LIBS=['m'],
            CFLAGS=c_flags,
            LINKFLAGS=link_flags)
