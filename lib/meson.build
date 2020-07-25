
mrgInclude = include_directories('.')

gen_data_inc = find_program('gen-data.sh')

data_inc = custom_target('data.inc',
    input : [ 'data/css.html', 'data/index.html'  ],
    output : [ 'data.inc' ],
    command : [ gen_data_inc, join_paths(meson.current_source_dir(), 'data')],
    capture : true,
    build_by_default : true,
)


mrg_sources = [
'mrg-audio.c',
'mrg-backend-gtk.c',
'mrg-backend-mem.c',
'mrg-backend-mmm.c',
'mrg-backend-terminal.c',
'mrg-binding.c',
'mrg.c',
'mrg-events.c',
'mrg-focus.c',
'mrg-host.c',
'mrg-http.c',
'mrg-image.c',
'mrg-list.c',
'mrg-restarter.c',
'mrg-sha256.c',
'mrg-string.c',
'mrg-style-properties.c',
'mrg-stylesheet.c',
'mrg-text.c',
'mrg-uri-fetcher.c',
'mrg-utf8.c',
'mrg-util.c',
'mrg-vt.c',
'mrg-xml.c',
'mrg-xml-render.c',
'nchanterm.c',
 data_inc
]

mrg_headers = [ 
'mrg-audio.h',
'mrg-config.h',
'mrg-events.h',
'mrg.h',
'mrg-host.h',
'mrg-list.h',
'mrg-string.h',
'mrg-style.h',
'mrg-text.h',
'mrg-utf8.h',
'mrg-util.h'
]

util = cc.find_library('util',  required: true)
math = cc.find_library('m',   required: false)
thread = dependency('threads')
alsa   = dependency('alsa')
x11    = dependency('x11')

install_headers( mrg_headers, subdir: 'mrg-0.1/' )
mrg_lib = library('mrg', mrg_sources,
                  include_directories: [ rootInclude, mrgInclude, ],
                  dependencies: [mmm, cairo, gtk, math, thread, alsa, util, x11] 
                  ,install:true
)
