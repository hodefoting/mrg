#!/usr/bin/env luajit
-- todo
--   rename
--   text editing
--   open context menu with keyboard shortcut
--   ask to save changes when trying to switch file
--   use gifplay as composited helper for gifs
--   svg viewing/editing 
--   ffplay video playback

--   image zoom/pan with gestures
--   gegl based helper for images
--   thumbnails

--   cache dir array between draws, with identify info as well

-- stick a .mrg file in each dir
-- containing per filename extra information/cache
-- order for custom order sort (useful for slideshows and playlists)
-- position and size/rotation for 2d-view
-- use hardlinks when copying .jpg or .png or .mp3 files on the same file system,..

-- enter text for rename, first in a global blocking query..

local S = require('syscall')

--S.setenv('MRG_RESTARTER','yes')
--S.setenv('MRG_BACKEND','mmm')

local Mrg = require('mrg')
local mrg = Mrg.new(640, 480);

--local path = '/home/pippin/src/mrg/luajit'
local path = '/home/pippin/images'
local folder_pan = 0;

local in_context = false
local context_x, context_y = 100, 100
local context_choices={}

local dir = {}

function mrg_modal_draw(mrg)
  local cr = mrg:cr()
  -- block out all other events, making clicking outside cancel

  cr:rectangle (0,0,mrg:width(),mrg:height())
  cr:set_source_rgba(0,0,0,0.5)
  mrg:listen(Mrg.COORD, function(event)
    event.stop_propagate = 1
  end)
  mrg:listen(Mrg.TAP, function(event)
    in_context = false
  end)
  cr:fill()
  
  cr:rectangle (context_x - 50, context_y - 50, 100, 100)
  cr:set_source_rgba(0,0,0, 0.5)
  mrg:listen(Mrg.COORD, function(event)
    event.stop_propagate = 1
  end)
  cr:fill()

  mrg:set_edge_left(context_x - 50)
  mrg:set_edge_top(context_y - 50)
  mrg:set_style('background:transparent; color: white; ')

  for i,v in pairs(context_choices) do
    mrg:text_listen(Mrg.TAP, function(event)
      if v.cb and v.type ~= 'edit' then 
        in_context = false
        v.cb() 
      end
      mrg:queue_draw(nil)
    end)

    if v.type == 'edit' then
      mrg:print('[')
      mrg:edit_start(v.cb)
    end
    mrg:print(v.title .. "\n")

    if v.type == 'edit' then
      mrg:edit_end()  -- XXX: maybe this should register the edit
                      --      bindings, so that later registrations
                      --      take precedence

      mrg:add_binding("return", NULL, NULL, 
        function (event)
          print ('return!!')
          event.stop_propagate = 1
        end)

      mrg:print(']')
    end

    mrg:text_listen_done ()
  end
end


function mrg_modal(mrg, x, y, choices)
  if choices then
    context_choices = choices
  else
    context_choices = {
      {title='uh??'}
    }
  end
  local w, h = 100, 100
  if x < w/2 then x = w/2 end
  if y < h/2 then y = h/2 end
  if x > mrg:width()  - w/2 then x = mrg:width ()-w/2 end
  if y > mrg:height() - h/2 then y = mrg:height()-h/2 end
  context_x = x
  context_y = y
  in_context = true
  mrg:queue_draw(nil)
end

-- local os = require('os')

function serialize (o)
  if type(o) == "number" then
    io.write(o)
  elseif type(o) == "string" then
    io.write(string.format("%q", o))
  elseif type(o) == "table" then
    io.write("{\n")
    for k,v in pairs(o) do
      --io.write("  ", k, " = ")
      io.write ("  [")
      serialize(k)
      io.wite ("] = ")
      ---

      serialize(v)
      io.write(",\n")
    end
    io.write("}\n")
  else
    error("cannot serialize a " .. type(o))
  end
end

function store_state()
  -- the state of the browser is small enough to contain in environment
  -- variables, which is good enough for live-coding
  S.setenv("BROWSER_PATH", path, 1)
  S.setenv("FOLDER_PAN", ''.. folder_pan, 1)
end

function restore_state()
  if (S.getenv("BROWSER_PATH")) then
    path = S.getenv("BROWSER_PATH")
  end
  if (S.getenv("FOLDER_PAN")) then
    folder_pan = tonumber (S.getenv("FOLDER_PAN"))
  end
end

if (#arg >= 1) then
  path = arg[1]
end

restore_state()

--local path = '/home/pippin/images'
local io  = require('io')


local css = [[
.pathbar { background: white; }
document {font-size: 20px; }
/* .folder {border: 1px solid red; } */
.dentry {color:blue} 
/* .entry  {border: 1px solid green; } */
#current { background: yellow; }
.content {color: blue ; background: white; }
.size { width: 6em; display: block; float: left; }
.size_unit { color: gray }
.fname { display: block; float: left; width: 80%; }
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

function go_next()
  local cursor = nil
  for i,file in pairs(dir) do
    if cursor and cursor.path == path then
      path = file.path
      mrg:queue_draw(null)
      return
    end
    cursor = file
  end
end

function go_previous()
  local cursor = nil
  for i,file in pairs(dir) do
    if file.path == path then
      if cursor then
        path = cursor.path
        mrg:queue_draw(null)
      end
      return
    end
    cursor = file
  end
end
function set_path(new_path)
  path = new_path
  mrg:queue_draw(null)
  store_state()
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

function collect_path (path)
    --local fd = S.open(path, "directory, rdonly")
    local dir = {}
    local hide_dot = true
    for name in S.util.ls(path) do --fd:getdents() do
       if name ~= '..' and name ~= '.' then
         local combined_path = path .. '/' .. name
         local file = {}
         local stat = S.stat(combined_path)
         if stat then
         file.size = stat.size
         file.isdir = stat.isdir
         else
           file.size = -1
           file.isdir = false
         end
         file.path = combined_path
         file.name = name
         if not hide_dot or string.sub(file.name, 1, 1) ~= '.' then
           table.insert(dir, file) 
         end
       end
    end

    table.sort(dir, function (a,b) 
      if a.isdir == false and b.isdir == true then
        return false
      elseif a.isdir == true and b.isdir == false then
        return true
      end
      return a.name < b.name
    end)

    return dir
end

function human_size(size)
  if size < 1024 then
    return (string.format("%1.0f<span class='size_unit'>b</span>", (size)))
  elseif size < 1024 * 1024 then
    return (string.format("%1.1f<span class='size_unit'>kb</span>", (size/1024)))
  elseif size < 1024 * 1024 * 1024 then
    return (string.format("%1.1f<span class='size_unit'>mb</span>", (size/1024/1024)))
  else
    return (string.format("%1.1f<span class='size_unit'>gb</span>", (size/1024/1024/1024)))
  end
end

function os.capture(cmd, raw)
  local f = assert(io.popen(cmd, 'r'))
  local s = assert(f:read('*a'))
  f:close()
  if raw then return s end
  s = string.gsub(s, '^%s+', '')
  s = string.gsub(s, '%s+$', '')
  s = string.gsub(s, '[\n\r]+', ' ')
  return s
end

function draw_folder(mrg, path, currpath, details)
    local cr = mrg:cr()


    cr:save()
    cr:translate (0, folder_pan)
    mrg:start('div.folder')

    dir = collect_path (path)

    for i,file in pairs(dir) do

      mrg:text_listen(Mrg.TAP + Mrg.TAP_AND_HOLD,
         function(event,d1,d2)
           if event.type == Mrg.TAP then
            -- default to set the path clicked
              set_path (file.path)
           else

             -- per file context menu

             mrg_modal(mrg, event.device_x, event.device_y,
              {{title=file.name, type='edit',
                  cb=function(new_text)
                    print('new_text: ' .. new_text)
                  end},
               {title='open',
                  cb=function()
                    os.execute('xdg-open ' .. file.path .. ' &')
                  end},
               {title='remove', cb=function()
                  mrg_modal(mrg, mrg:pointer_x(), mrg:pointer_y(), 
                    {
                      {title='really?'},
                      {title='yes',
                       cb=function() 
                            os.execute('rm -f ' .. file.path)
                          end},
                      {title='no', cb=function() end},
                    })
                  end},
               }
             )
             mrg:queue_draw(nil)
           end
           return 0;
         end)
      local xml = "<div "
      if file.isdir then
        xml = xml .. "class='dentry' "
      else
        xml = xml .. "class='entry' "
      end
      if ( file.path == currpath) then
        xml = xml .. " id='current' "
      end
      xml = xml .. "><span class='fname'>" .. file.name .. "</span>"
      if details then
          if file.isdir then
          else
            --local id = os.capture('file ' .. file.path .. ' | cut -f 2 -d ":" ')
            --xml = xml .. "<span class='size'>" .. id .. human_size(file.size) .. "</span>"
            xml = xml .. "<span class='size'>" .. human_size(file.size) .. "</span>"
          end
       end
      xml = xml .. "</div>\n "
      mrg:print_xml(xml)
      mrg:text_listen_done()
    end
    mrg:close()
    cr:restore()
end

function draw_image (mrg, x, y)
  local em = mrg:em()
  local w, h = mrg:image_size(path)
  local cr = mrg:cr()

  local scale = 1.0;

  local dw, dh;
    
  scale = (mrg:width () - 8 * em) / w
  
  dw, dh = w * scale, h * scale

  if dw > mrg:width() - 8 * em then
    scale = (mrg:width () - 8 * em) / w
    dw, dh = w * scale, h * scale
  end
  if dh > mrg:height() - y then
    scale = (mrg:height() - y) / h
    dw, dh = w * scale, h * scale
  end

  cr:save()
  cr:translate(8*em + ((mrg:width()-8 * em) - dw)/2, 
              (y + ((mrg:height()-y) - dh)/2))
  cr:rectangle(0, 0, mrg:width() - 8 * em, mrg:height() - y)
  cr:clip()


  mrg:image(0, 0, dw, dh, path)

  cr:restore()
end


mrg:set_ui(
function (mrg, data)
  local cr = mrg:cr()

  local stat = S.stat(path)
  local x, y

  if stat.isdir then
    x, y = mrg:xy()
    draw_folder(mrg, path, undefined, true)

    cr:rectangle(0, y, mrg:width(), mrg:height())
    mrg:listen(Mrg.DRAG, function(ev)
       folder_pan = folder_pan + ev.delta_y
       mrg:queue_draw(null)
    end)
    cr:new_path()
    mrg:set_xy(0,0)
    path_bar(mrg, path)

  elseif stat.isreg then

    x, y = mrg:xy()
    local em = mrg:em()
    draw_folder(mrg, get_parent(path), path, false)

    mrg:set_edge_right(mrg:width() - em)
    mrg:set_edge_left(8 * em)
    mrg:set_edge_top(y)

    if string.find(path, ".png") or 
       string.find(path, ".PNG") or
       string.find(path, ".jpg") or
       string.find(path, ".gif") or
       string.find(path, ".hdr") or
       string.find(path, ".HDR") or
       string.find(path, ".GIF") or
       string.find(path, ".jpeg") or
       string.find(path, ".JPEG") or
       string.find(path, ".JPG") 
      then
      draw_image(mrg,x, y)
    else
      local f = io.open(path, 'r')
      mrg:print_xml ("<pre class='content'>".. f:read("*all") .. "</pre> ")
      f:close()
    end

    cr:rectangle(0, y, 8 * mrg:em(), mrg:height())
    mrg:listen(Mrg.DRAG, function(ev)
       folder_pan = folder_pan + ev.delta_y
       mrg:queue_draw(null)
    end)
    cr:new_path()

    mrg:set_xy(0,0)
    path_bar(mrg, path)

  end


  mrg:add_binding("control-q", NULL, NULL, function (event) mrg:quit() end)
  mrg:add_binding("left",      NULL, NULL, function (event) go_parent() end)
  mrg:add_binding("up",        NULL, NULL, function (event) go_previous() end)
  mrg:add_binding("down",      NULL, NULL, function (event) go_next() end)
  mrg:add_binding("escape",    NULL, NULL, function (event)
     go_parent()
     event.stop_propagate = 1
  end)

  if in_context then
    mrg_modal_draw (mrg)
  end

end)

mrg:css_set(css)

restore_state()
mrg:main()

