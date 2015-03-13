#!/usr/bin/env luajit

-- todo
--   scrolling of file view
--   more info in dir mode
--   persistance of state/design to crash
--   scrolling of edited text (from editor)
--   keybindings
--   image scaling
--   ask to save changes when trying to switch file
--   text editing
--   svg viewing/editing 
--   pdf viewing
--   video playback
--   next/prev item


local S = require('syscall')

--local path = '/home/pippin/src/mrg/luajit'
local path = '/home/'

if (#arg >= 1) then
  path = arg[1]
end



--local path = '/home/pippin/images'
local io  = require('io')
local Mrg = require('mrg')
local mrg = Mrg.new(512,384);

local folder_pan = 0;


local sting = require('string')
local css = [[

document {font-size: 30px; }
.folder {border: 1px solid red; }
.dentry {color:blue} 
.entry  {border: 1px solid green; }
#current { background: yellow; }
.content {color: blue ; background: white; }
.size { width: 4em; display: block; float: left; }
.fname { display: block; float: left; width: 50%; }
]]

function get_parent(path)
  local t = {}
  local newp = '';
  for str in string.gmatch(path, "([^/]+)") do
    t[#t+1] = str
  end
  for i = 1,#t-1 do
    newp = newp .. '/' .. t[i]
  end
  path = newp
  if newp == '' then newp = '/' end
  return newp
end

function go_parent()
  path = get_parent (path)
  mrg:queue_draw(null)
end

function set_path(new_path)
  path = new_path
  mrg:queue_draw(null)
end

function path_bar(mrg)
  local cr = mrg:cr()
  
  mrg:start('div.pathbar')
  local t = {}
  local newp = '';

  mrg:text_listen(Mrg.PRESS,
    function(event,d1,d2)
      set_path '/'
      return 0;
    end)
  mrg:print_xml("<span class='pathentry'> /</span> \n")
  mrg:text_listen_done()

  local sep = '';

  for str in string.gmatch(path, "([^/]+)") do
    local foo = newp .. '/' .. str
    newp = foo
    mrg:text_listen(Mrg.PRESS,
      function(event,d1,d2)
        set_path(foo)
        mrg:queue_draw(null);
        return 0;
      end)
    mrg:print_xml("<span class='pathentry'>" .. sep .. str .. "</span> \n")
    sep='/'
    mrg:text_listen_done()
  end
  mrg:close()
end

function draw_folder(mrg, path, currpath, details)
    local cr = mrg:cr()
    cr:save()
    cr:translate (0, folder_pan)
    local fd = S.open(path, "directory, rdonly")
    mrg:start('div.folder')
    for d in fd:getdents() do
       if d.name ~= '..' and d.name ~= '.' then
         if details then
           local size = S.stat(path .. '/' .. d.name).size

            mrg:text_listen(Mrg.PRESS,
              function(event,d1,d2)
                set_path (path .. '/' .. d.name);
                return 0;
              end)
           if ( path .. '/' .. d.name == currpath) then
             mrg:print_xml("<div class='entry' id='current'><span class='fname'>" .. d.name .. 
               "</span><span class='size'>" .. size .. "</span></div> \n")
           else
             mrg:print_xml("<div class='entry'><span class='fname'>" .. d.name .. 
               "</span><span class='size'>" .. size .. "</span></div> \n")
           end
           mrg:text_listen_done()

           --print(d.name)
         else
            mrg:text_listen(Mrg.PRESS,
              function(event,d1,d2)
                set_path (path .. '/' .. d.name);
                return 0;
              end)
           if ( path .. '/' .. d.name == currpath) then
             mrg:print_xml("<div class='entry' id='current'>" .. d.name .. 
               "</div> \n")
           else
             mrg:print_xml("<div class='entry'>" .. d.name .. "</div> \n")
           end
           mrg:text_listen_done()
         end
       end
    end
    mrg:close()
    cr:restore()
end

function draw_image (mrg, x, y)
  local em = mrg:em()
  mrg:image(8 * em,y,mrg:width() - 8 * em, mrg:height() - y, path)
end

mrg:set_ui(
function (mrg, data)
  local cr = mrg:cr()

  local stat = S.stat(path)
  local x, y

  if stat.isdir then
    path_bar(mrg, path)
    x, y = mrg:xy()
    draw_folder(mrg, path, undefined, true)
  elseif stat.isreg then
    path_bar(mrg, path)
    x, y = mrg:xy()
    local em = mrg:em()
    draw_folder(mrg, get_parent(path), path, false)

    mrg:set_edge_right(mrg:width() - em)
    mrg:set_edge_left(8 * em)
    mrg:set_edge_top(y)

    if string.find(path, ".png") or 
       string.find(path, ".jpg") or
       string.find(path, ".PNG") or
       string.find(path, ".JPG") 
      then
      draw_image(mrg,x, y)
    else
      local f = io.open(path, 'r')
      mrg:print_xml ("<pre class='content'>".. f:read("*all") .. "</pre> ")
      f:close()
    end
  end

  if true then
  cr:rectangle(0, y, 8 * mrg:em(), mrg:height())
  mrg:listen(Mrg.DRAG, function(ev)
     folder_pan = folder_pan + ev.delta_y
     mrg:queue_draw(null)
  end)
  end

  mrg:add_binding("control-q", NULL, NULL, function (event) mrg:quit() return 0 end)
  mrg:add_binding("escape", NULL, NULL, function (event)
     go_parent()
     event.stop_propagate = 1
  end)

end)

mrg:css_set(css)
mrg:main()

