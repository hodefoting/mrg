#!/usr/bin/env luajit

-- a traditional window manager, titlebar-draggable, maximizable and resizable
-- windows

local os     = require 'os'
local string = require 'string'
local Mrg    = require 'mrg'
local mrg    = Mrg.new(640, 480);
local host   = mrg:host_new("/tmp/mrg")

local css = "document {background-color:#111; }";

local mrg2 = Mrg.new(200, 200, "mem")
mrg2:set_title ("task list")
mrg2:set_ui(function(mrg) 
  mrg:print("fnord")
end)
host:add_client_mrg(mrg2, 40, 40)
host:add_client_mrg(mrg2, 40, 140)

mrg:set_ui(
  function()
    local cr = mrg:cr()
    local em = mrg:em()

    mrg:start("host")
    mrg:set_xy(0,0)
    mrg:start("header")
    mrg:start("time")
    mrg:print(os.date("%a %d %b %H:%M"))
    mrg:close()

    mrg:start("applications")
    mrg:print(" $ ")
    mrg:print(" @ ")
    mrg:close()

    mrg:close()

    host:monitor_dir()
    local old_focused = host:focused()
    host:set_focused(nil)  -- (render_sloppy sets focused as part of rendering)


    local clients = host:clients()
    for i, client in ipairs(clients) do 
      local x, y = client:xy()
      local w, h = client:size()

      client:render_sloppy(x, y)

      if old_focused == client then
      mrg:start_with_style('client.focused', string.format('left:%dpx;top:%dpx;width:%dpx;height:%dpx', x-1, y-1, w, h))
      else
      mrg:start_with_style('client', string.format('left:%dpx;top:%dpx;width:%dpx;height:%dpx', x-1, y-1, w, h))
      end

      em = mrg:em()
      mrg:start_with_style('title',
      string.format('left:%dpx;top:%dpx;width:%dpx;height:%dpx;border-width:1px', x-2, y- 1.5 * em, w-1, em * 1.0))
      cr:rectangle(x-2, y- 1.5 * em, w-1, em * 1.5)
      mrg:listen(Mrg.DRAG, function(event) 
        local x, y = client:xy()
        x, y = x + event.delta_x, y + event.delta_y;
        client:set_xy(x,y)
        mrg:queue_draw(NULL)
        return 0
      end)
      mrg:print(client:title())

      mrg:start('close')
      mrg:text_listen(Mrg.PRESS, function(event)
        client:kill()
        return 0
      end)
      mrg:print('X')
      mrg:text_listen_done()
      mrg:close()

      mrg:start('max')
      mrg:text_listen(Mrg.PRESS, function(event)
        client:maximize()
        return 0
      end)
      mrg:print('  ')
      mrg:text_listen_done()
      mrg:close()

      mrg:close()

      cr:rectangle(x + w - 20, y + h - 20, 23, 23)
      mrg:listen(Mrg.DRAG, function(event) 
        local w, h = client:size()
        w, h = w + event.delta_x, h + event.delta_y;
        client:set_size(w, h)
        mrg:queue_draw(NULL)
        return 0
      end)
      cr:fill()

      mrg:close()
    end
    host:register_events()
    mrg:add_binding("F10", nil, "quit", function()
      mrg:quit()
      return 0
    end)
    mrg:close()
  end
)

mrg:css_set(css)

mrg:main()

