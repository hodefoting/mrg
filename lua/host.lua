#!/usr/bin/env luajit

-- a traditional window manager, titlebar-draggable, maximizable and resizable
-- windows

-- redraw of single windows cause loss of events elsewhere

local S = require 'syscall'


if false then
  -- using the restarter on the host is only advisable
  -- when all clients already are launched, since the use
  -- of environment variables by the restarter mechanism
  -- interacts badly with launching clients of an mmm compositor.

  S.setenv('MRG_RESTARTER','yes')
  S.setenv('MRG_BACKEND','mmm')
end

local tiled = true

local os     = require('os')
local string = require('string')
local Mrg    = require('mrg')
--local mrg    = Mrg.new(-1, -1);
local mrg    = Mrg.new(640, 480);
local host   = mrg:host_new("/tmp/mrg")

local notifications = {
  -- {text='welcome to microraptor gui'},
}

local show_apps = false;

local applications = {
  {text='applications',  cb=function() show_apps=false mrg:queue_draw(nil) end },
  {text='terminal',      command='mrg terminal&'},
  {text='flipping game', command='./flipgame.lua &'},
  {text='paddlewar',     command='./paddlewar.lua &'},
  {text='browser',       command='mrg browser &'},
  {text='filsystem',     command='./browse.lua `pwd` &'},
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
}
notification {
   color: white;
   margin-bottom: 0.1em;
   display: block;
}
apps {
   background-color: #000d; 
   padding: 0.5em;
   padding-top: 0;
   border: 2px solid white; 
   width: 10em;
   display: block; 
}
application {
   color: white;
   margin-bottom: 1.4em;
   display: block;
}
]];

function table.copy(t)
  local u = { }
  for k, v in pairs(t) do u[k] = v end
  return setmetatable(u, getmetatable(t))
end

function table.reverse(t)
  local l = table.getn(t) -- table length
  local j = l
  for i = 1, l / 2 do
    t[i], t[j] = t[j], t[i]
    j = j - 1
  end
end

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
      show_apps = true
      mrg:queue_draw(nil)
    end)
    mrg:print("applications")
    mrg:text_listen_done()
    mrg:close()

    mrg:close()

    local old_focused = host:focused()
    -- host:set_focused(nil)  -- (render_sloppy sets focused as part of rendering)
    host:monitor_dir()

    local clients = host:clients()

    local x = 0
    local y = 40

    local w = 160
    local h = 120

    em = mrg:em()

    for i, client in ipairs(clients) do 
      local x, y = client:xy()
      local w, h = client:size()

      client:render(mrg, x, y)

      em = mrg:em()
      if not client:get_value('borderless') then
      if old_focused == client then
      mrg:start_with_style('client.focused', string.format('left:%dpx;top:%dpx;width:%dpx;height:%dpx', x-1, y-1, w, h))
      else
      mrg:start_with_style('client', string.format('left:%dpx;top:%dpx;width:%dpx;height:%dpx', x-1, y-1, w, h))
      end

      mrg:start_with_style('title',
      string.format('left:%dpx;top:%dpx;width:%dpx;height:%dpx;border-width:1px', x-2, y- 1.5 * em, w-1, em * 1.0))
      cr:rectangle(x-2, y- 1.5 * em, w-1, em * 1.5)

      -- set up a coordevent blocker behind titlebar
      mrg:listen(Mrg.COORD, function(event) event:stop_propagate() end)

      local loc_x, loc_y = client:xy()
      mrg:listen(Mrg.DRAG_MOTION + Mrg.DRAG_PRESS,
        function(event) 
          if event.type == Mrg.DRAG_PRESS then
            host:set_focused(client)
            client:raise_top()
          elseif event.type == Mrg.DRAG_MOTION then
            loc_x, loc_y = loc_x + event.delta_x, loc_y + event.delta_y
            if loc_y <= 1.2 * em then loc_y = 1.2 * em end
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
      local loc_w, loc_h = client:size()
      mrg:listen(Mrg.DRAG_PRESS + Mrg.DRAG_MOTION, function(event) 
        if event.type == Mrg.DRAG_MOTION then
          loc_w, loc_h = loc_w + event.delta_x, loc_h + event.delta_y;
          client:set_size(loc_w, loc_h)
        end
        mrg:queue_draw(NULL)
        event:stop_propagate()
      end)
      cr:fill()

      mrg:close()
      end

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

    -- setting the positions with one frame lag,
    -- but permits doing the interaction rects in the same go
    --
    if tiled then
      local rclients = table.copy(clients)
      table.reverse(rclients)
      for i, client in ipairs(rclients) do 
         if i == 1 then
           w = mrg:width()/2
           h = mrg:height() - 120 -y
           x = mrg:width()/2

           client:set_xy(x, y)
           client:set_size(w,h)
         elseif i == 2 then
           w = mrg:width()/2
           h = mrg:height() - 120 -y
           x = 0

           client:set_xy(x, y)
           client:set_size(w,h)

           cr:new_path()
           cr:rectangle(w - 2 * em,  
                   mrg:height() - 120 + 12 + 12 - 4 * em,
                  4 * em, 2.3 *em);
           cr:set_source_rgba(0,0,1, 0.1)
           mrg:listen(Mrg.TAP, function(event)
             client:set_stack_order(0)
             event:stop_propagate()
             mrg:queue_draw(nil)
           end)
           cr:fill()

           w = 160
           h = 120 - 12
           x = 0
           y = mrg:height() - h + 12

         else
           client:set_xy(x, y)
           client:set_size(w,h)

           cr:rectangle(x,y - 18,w,h + 18)
           cr:set_source_rgba(0,0,1,0.1)
           mrg:listen(Mrg.COORD, function(event) 
             event:stop_propagate()
           end)
           mrg:listen(Mrg.TAP, function(event) 
             client:set_stack_order(1)
           end)
           --cr:fill()
           cr:new_path()

           x = x + w
           if (x + w > mrg:width()) then
             x = 0
             y = y + h + 18
           end
         end
      end
    end

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
    
    if show_apps and #applications > 0 then
      mrg:set_xy(0,0)
      mrg:start('apps')
      for i, application in ipairs(applications) do 
        mrg:start('application')

        mrg:text_listen(Mrg.TAP, function(event)
          if application.cb then
            application.cb()
          elseif application.command then
            os.execute (application.command)
            show_apps = false
            S.nanosleep(0.1) -- XXX: eeek, avoiding construction race... needs fixing in mmm
          end
          mrg:queue_draw(nil)
        end)

        mrg:print(application.text)

        mrg:text_listen_done()

        mrg:close()
      end
      mrg:close()
    end
    
    mrg:close()

    Mrg.draw_keyboard(mrg)
  end
)

mrg:css_set(css)
mrg:main()
