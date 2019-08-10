
math = cc.find_library('m',   required: false)


examples = [
 { 'name': 'audio', },
 { 'name': 'client_move', },
 { 'name': 'gtk-embed', },
 { 'name': 'image', },
 { 'name': 'in-process-compositor', },

]

foreach example : examples
  example_name = example.get('name')
  example_srcs = example.get('srcs', example_name + '.c')
  example_deps = example.get('deps', [])

  exe = executable(example_name, example_srcs,
    include_directories : [mrgInclude,],
    link_with: [ mrg_lib ],
    dependencies : [ example_deps, cairo, mmm, gtk, math, thread ],
    install : false,)


endforeach
