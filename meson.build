project(
	'wayfire',
    'c',
	'cpp',
	version: '0.0',
	license: 'MIT',
	meson_version: '>=0.43.0',
	default_options: [
		'cpp_std=c++11',
        'c_std=c11',
		'warning_level=2',
		'werror=false',
	],
)

so_version = ['0', '0', '0']
wayfire_api_inc  = include_directories('src/api')

wayland_server = dependency('wayland-server')
wayland_client = dependency('wayland-client')
wayland_cursor = dependency('wayland-cursor')
wayland_protos = dependency('wayland-protocols', version: '>=1.12')
cairo          = dependency('cairo')
drm            = dependency('libdrm')
egl            = dependency('egl')
glesv2         = dependency('glesv2')
glm            = dependency('glm')
libevdev       = dependency('libevdev')
freetype2      = dependency('freetype2')
libinput       = dependency('libinput', version: '>=1.7.0')
pixman         = dependency('pixman-1')
threads        = dependency('threads')
xkbcommon      = dependency('xkbcommon')
wlroots        = dependency('wlroots', fallback: ['wlroots', 'wlroots'])
wfconfig       = dependency('wf-config', fallback: ['wf-config', 'wfconfig'])

jpeg = dependency('libjpeg', required: false)
png  = dependency('libpng',  required: false)

conf_data = configuration_data()

conf_data.set('WF_SRC_DIR', meson.source_root())
conf_data.set('INSTALL_PREFIX', get_option('prefix'))

# needed to dlopen() plugins
add_project_arguments(['-DWLR_USE_UNSTABLE'], language: ['cpp', 'c'])
add_project_link_arguments(['-rdynamic'], language:'cpp')

if get_option('buildtype') == 'debug' or get_option('enable_debug_output')
  conf_data.set('WAYFIRE_DEBUG_ENABLED', true)
endif

if get_option('enable_graphics_debug')
  conf_data.set('WAYFIRE_GRAPHICS_DEBUG', true)
endif

if get_option('enable_gles32') and meson.get_compiler('cpp').has_header('GLES3/gl32.h')
  conf_data.set('USE_GLES32', true)
endif

if png.found() and jpeg.found()
  conf_data.set('BUILD_WITH_IMAGEIO', true)
endif

configure_file(input: 'config.h.in',
               output: 'config.h',
               configuration: conf_data)

wayfire_conf_inc = include_directories(['.'])

add_project_arguments(['-Wno-pedantic', '-Wno-unused-parameter'], language: 'cpp')

subdir('proto')
subdir('src')
subdir('plugins')
subdir('shell')

install_subdir('shaders', install_dir: 'share/wayfire')

summary = [
	'',
	'----------------',
	'wayfire @0@'.format(meson.project_version()),
	'',
	'debug output: @0@'.format(conf_data.get('WAYFIRE_DEBUG_ENABLED', false)),
	'     imageio: @0@'.format(conf_data.get('BUILD_WITH_IMAGEIO', false)),
	'      gles32: @0@'.format(conf_data.get('USE_GLES32', false)),
    'graphics dbg: @0@'.format(conf_data.get('WAYFIRE_GRAPHICS_DEBUG', false)),
	'----------------',
	''
]
message('\n'.join(summary))
