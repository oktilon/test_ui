project('ui', 'c',
    version : '1.0.0',
    meson_version : '>= 0.54.0'
)

# Parse version string
version_arr = meson.project_version().split('.')

# Project configuration
prj_name = meson.project_name()
conf_data = configuration_data()
conf_data.set('version_major', version_arr[0])
conf_data.set('version_minor', version_arr[1])
conf_data.set('version_rev',   version_arr[2])
conf_data.set('dbus_base_path',   '/com/getdefigo/')
conf_data.set('dbus_base_name',   'com.getdefigo.')
conf_data.set('dbus_this',        prj_name)
conf_data.set('dbus_gateway',     'gateway')

# Main header file config
configure_file(
    input         : 'src/app.h.in',
    output        : 'app.h',
    configuration : conf_data
)

# Dependencies
deps = [
    dependency('libsystemd'),
    dependency('threads'),
    dependency('sqlite3'),
    dependency('freetype2'),
    dependency('gtk+-3.0'),
    dependency('glib-2.0'),
    dependency('gio-2.0'),
]

# Resources
gnome = import('gnome')
asresources = gnome.compile_resources(
    'as-resources', 'src/resources.xml',
    source_dir: ['src/ui', 'src/resources'],
    c_name: 'as'
)

# Includes
inc = include_directories([
    'include',
])

# Sources
src = [
    'src/app.c',
    'src/dbus.c',
    'src/keypad.c',
    'src/models/mdl_unit.c',
    'src/models/mdl_doorbell.c',
    'src/view.c',
    'src/view_history.c',
    'src/views/view_main.c',
    'src/views/view_connecting.c',
    'src/view_manager.c',
    asresources,
]

# Build executable
executable(
    prj_name,
    src,
    include_directories : inc,
    dependencies        : deps,
)