#!/usr/bin/env luajit

-- a traditional window manager, titlebar-draggable, maximizable and resizable
-- windows


local S      = require 'syscall'
if true then
  S.setenv('MRG_RESTARTER','yes')
  S.setenv('MRG_BACKEND','mmm')
end
local os     = require 'os'
local string = require 'string'
local Mrg    = require 'mrg'
local mrg    = Mrg.new(-1, -1);
local host   = mrg:host_new("/tmp/mrg")


local notifications = {
  {text='welcome to microraptor gui'},
}

function string.has_prefix(String,Start)
     return string.sub(String,1,string.len(Start))==Start
end

local css = [[
document {background-color:#111;} 
notifications {
   background-color: #000d; 
   padding: 0.5em;
   padding-top: 0;
   border: 2px solid white; 
   display: block; 
   width: 20em;
   margin-left: auto;
   margin-right: auto;
};

notification {
   color: white;
   margin-bottom: 0.1em;
   display: block;
};
]];
--[[
local mrg2 = Mrg.new(200, 200, "mem")
mrg2:set_title ("task list")
mrg2:set_ui(function(mrg) 
  mrg:print("fnord")
end)
host:add_client_mrg(mrg2, 40, 40)
host:add_client_mrg(mrg2, 40, 140)
]]
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
    mrg:text_listen(Mrg.PRESS, function(event)
      os.execute('mrg terminal&')
    end)
    mrg:print("terminal")
    mrg:text_listen_done()
    mrg:close()

    mrg:close()

    local old_focused = host:focused()
    -- host:set_focused(nil)  -- (render_sloppy sets focused as part of rendering)
    host:monitor_dir()

    local clients = host:clients()
    for i, client in ipairs(clients) do 
      local x, y = client:xy()
      local w, h = client:size()

      client:render(mrg, x, y)

      if old_focused == client then
      mrg:start_with_style('client.focused', string.format('left:%dpx;top:%dpx;width:%dpx;height:%dpx', x-1, y-1, w, h))
      else
      mrg:start_with_style('client', string.format('left:%dpx;top:%dpx;width:%dpx;height:%dpx', x-1, y-1, w, h))
      end

      em = mrg:em()
      mrg:start_with_style('title',
      string.format('left:%dpx;top:%dpx;width:%dpx;height:%dpx;border-width:1px', x-2, y- 1.5 * em, w-1, em * 1.0))
      cr:rectangle(x-2, y- 1.5 * em, w-1, em * 1.5)

      -- set up a coordevent blocker behind titlebar
      mrg:listen(Mrg.COORD, function(event) event:stop_propagate() end)

      local loc_x, loc_y
      mrg:listen(Mrg.DRAG_MOTION + Mrg.DRAG_PRESS,
        function(event) 
          if event.type == Mrg.DRAG_PRESS then
            host:set_focused(client)
            client:raise_top()
            loc_x, loc_y = client:xy()
          elseif event.type == Mrg.DRAG_MOTION then
            if not loc_x then -- XXX: hacky precaution
              loc_x, loc_y = client:xy()
            end
            loc_x, loc_y = loc_x + event.delta_x, loc_y + event.delta_y
            client:set_xy(loc_x, loc_y)
          end
          mrg:queue_draw(NULL)
          event:stop_propagate()
          return 0
        end)
      mrg:print(client:title())

      mrg:start('close')
      mrg:text_listen(Mrg.PRESS, function(event) client:kill() end)
      mrg:print('X')
      mrg:text_listen_done()
      mrg:close()

      mrg:start('max')
      mrg:text_listen(Mrg.PRESS, function(event) client:maximize() end)
      mrg:print('  ')
      mrg:text_listen_done()
      mrg:close()

      mrg:close()

      cr:rectangle(x + w - 20, y + h - 20, 23, 23)
      local loc_w, loc_h
      mrg:listen(Mrg.DRAG_PRESS + Mrg.DRAG_MOTION, function(event) 
        if event.type == Mrg.DRAG_PRESS then
          loc_w, loc_h = client:size()
        elseif event.type == Mrg.DRAG_MOTION then
          if not loc_w then -- XXX: hacky precation
            loc_w, loc_h = client:size()
          end
          loc_w, loc_h = loc_w + event.delta_x, loc_h + event.delta_y;
          client:set_size(loc_w, loc_h)
        end
        mrg:queue_draw(NULL)
        event:stop_propagate()
      end)
      cr:fill()

      mrg:close()

      if client:has_message() ~= 0 then
        local message = client:get_message()

        -- by having the all target, the compositor acts as a bus
        -- and it is up to the clients to know that they are targetd

        if message:has_prefix('all ') then
           for i2, client2 in ipairs(clients) do 
             client2:send_message( message:sub(5,-1) )
           end
        elseif message:has_prefix('notify ') then
          table.insert(notifications, {text=message:sub(9.-1)})
          mrg:queue_draw(nil)
        end
      end

    end
    host:register_events()
    mrg:add_binding("F10", nil, "quit", function() mrg:quit() end)

    if #notifications > 0 then
    
    mrg:set_xy(0,0)
    mrg:start('notifications')
    mrg:text_listen(Mrg.TAP, function(event)
      notifications={}
      mrg:queue_draw(nil)
    end)
    for i, notification in ipairs(notifications) do 
      mrg:start('notification')
      mrg:print(notification.text)
      mrg:close()
    end
    mrg:text_listen_done()
    mrg:close()
    end
    
    mrg:close()

    Mrg.draw_keyboard(mrg)
  end
)

mrg:css_set(css)

mrg:main()

