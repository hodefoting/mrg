#!/usr/bin/env luajit

local Mrg  = require 'mrg'
local mrg  = Mrg.new(640, 480);
local host = mrg:host_new("/tmp/mrg")

mrg:set_ui(
  function()
    local cr = mrg:cr()
    host:monitor_dir()
    host:set_focused(nil)
    local clients = host:clients()
    for i, client in ipairs(clients) do 
      local x, y = client:x(), client:y()
      local w, h = client:size()
      cr:rectangle(x-0.5,y-0.5,w+1,h+1)
      cr:set_source_rgb(1,0,0)
      cr:set_line_width(3)
      cr:stroke()
      mrg:start()
      mrg:set_xy(x - 2, y)
      mrg:set_style('background:red; color: white;')
      mrg:text_listen(Mrg.DRAG, function(event) 
        local x, y = client:xy()
        x = x + event.delta_x;
        y = y + event.delta_y;
        client:set_xy(x,y)
        mrg:queue_draw(NULL)
        return 0
      end)
      mrg:print(client:title())
      mrg:text_listen_done()
      mrg:close()
      client:render_sloppy(x, y)
      cr:rectangle(x + w - 20, y + h - 20, 23, 23)
      mrg:listen(Mrg.DRAG, function(event) 
        local x, y = client:size()
        x = x + event.delta_x;
        y = y + event.delta_y;
        client:set_size(x,y)
        mrg:queue_draw(NULL)
        return 0
      end)
      cr:fill()


    end
    host:register_events()
  end
)
mrg:main()

