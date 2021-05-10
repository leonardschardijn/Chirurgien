chirurgien_sources = [
  'main.c',
  'chirurgien-application.c',
  'chirurgien-window.c',
  'chirurgien-view.c',
  'chirurgien-view-tab.c',
  'chirurgien-editor.c',
  'chirurgien-actions.c',
  'chirurgien-utils.c',
  'chirurgien-preferences-dialog.c',
  'chirurgien-formats-dialog.c'
]

chirurgien_deps = [
  dependency('gtk4')
]

gnome = import('gnome')
subdir('resources')

subdir('formats')

executable('chirurgien', chirurgien_sources,
  dependencies: chirurgien_deps,
  include_directories: [ formats_include_dir ],
  install: true
)
