alsa = dependency('alsa')

inc_src = include_directories(['include', 'src'])
podhdctrl_inc = include_directories('include')

files_podhdctrl = files(
	'include/podhdctrl.h',
	'include/podhd_preset.h',

	'src/list.h',
	'src/podhdctrl_int.h',
	'src/parser.c',
	'src/podhdctrl.c'
)

libpodhdctrl = static_library('podhdctrl',
			   files_podhdctrl,
			   include_directories : inc_src,
			   dependencies : alsa)

podhdctrl_lib = declare_dependency(link_with : [ libpodhdctrl ])
