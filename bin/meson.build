
mmm_linux = executable('mrg',
      [
'acoustics.c',
'browser.c',
'dir.c',
'edit.c',
'host.c',
'main.c',
'session.c',
'terminal.c',
],
      include_directories: [ rootInclude, mrgInclude ],
      link_with : [ mrg_lib ],
      dependencies : [ cairo, mmm ],
      install: true
)
