project('chirurgien', 'c',
          version: '0.5',
    meson_version: '>= 0.50.0',
  default_options: [ 'warning_level=2',
                     'c_std=gnu11' ]
)

i18n = import('i18n')

config_h = configuration_data()
config_h.set_quoted('VERSION', meson.project_version())
config_h.set_quoted('GETTEXT_PACKAGE', meson.project_name())
config_h.set_quoted('LOCALEDIR', join_paths(get_option('prefix'), get_option('localedir')))
configure_file(
  output: 'config.h',
  configuration: config_h
)
add_project_arguments([
  '-I' + meson.build_root(),
], language: 'c')

subdir('data')
subdir('chirurgien')
subdir('po')

meson.add_install_script('build-aux/meson/postinstall.py')
