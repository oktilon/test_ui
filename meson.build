project('demo', 'cpp')
deps = [
      dependency('gtk+-3.0')
]

gnome = import('gnome')
asresources = gnome.compile_resources(
    'as-resources', 'src/resources.xml',
    source_dir: ['src/ui', 'src/resources'],
    c_name: 'as'
)

executable(
    meson.project_name(),
    [
        'src/app.cpp',
        asresources,
    ],
    dependencies : deps
)