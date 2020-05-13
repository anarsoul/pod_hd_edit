qt5 = import('qt5')
qt5_dep = dependency('qt5', modules: ['Core', 'Gui', 'Widgets', 'Svg'])
inc = include_directories('.')

ui_files = files(
	'ui/mainWindow.ui',
)

#qrc_files = files(
#	'rc/pod_hd_edit.qrc',
#)

gui_moc_headers = files(
	'mainWindow.h',
	'ampItem.h',
	'rectTextItem.h',
	'fxItem.h',
	'mixItem.h',
)

gui_src = files(
	'mainWindow.cpp',
	'ampItem.cpp',
	'rectTextItem.cpp',
	'fxItem.cpp',
	'mixItem.cpp',
	'main.cpp'
)

moc_files = qt5.preprocess(moc_headers : gui_moc_headers,
			   ui_files: ui_files,
			   #qresources: qrc_files,
			   include_directories: [ inc, podhdctrl_inc ],
			   dependencies: qt5_dep)

executable(
	'pod_hd_edit',
	[ gui_src, moc_files ],
	include_directories: [ inc, podhdctrl_inc ],
	dependencies : [ podhdctrl_lib, qt5_dep]
)
