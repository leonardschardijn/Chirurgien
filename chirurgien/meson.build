chirurgien_sources = [
  'main.c',
  'chirurgien-application.c',
  'chirurgien-window.c',
  'chirurgien-analyzer-view.c',
  'chirurgien-actions.c',
  'chirurgien-navigation-button.c',
  'chirurgien-preferences-dialog.c',
  'chirurgien-analyzer-info-dialog.c'
]

chirurgien_deps = [
  dependency('gio-2.0', version: '>= 2.50'),
  dependency('gtk+-3.0', version: '>= 3.22'),
  dependency('amtk-5')
]

gnome = import('gnome')
subdir('resources')

subdir('analyzers')

executable('chirurgien', chirurgien_sources,
  dependencies: chirurgien_deps,
  include_directories: [ analyzers_include_dir ],
  install: true
)
