#!/usr/bin/env luajit

local Mrg = require('mrg')
local mrg = Mrg.new(512,384);
local x = 0.1;
local y = 0.1;

mrg:set_ui(
function (mrg, data)
  local cr = mrg:cr()

  mrg:set_font_size(32)
  mrg:set_xy(0, mrg:em()*1.2);
  mrg:print_xml("<div style='color:red'>luajit + mrg</div> \n")

  mrg:set_xy(0, mrg:em() * 4)
  mrg:text_listen(Mrg.PRESS, function(event,d1,d2)
    mrg:quit();
    return 0;
  end)

  mrg:print("quit")
  mrg:text_listen_done()

  mrg:print(" " .. x .. ", " .. y)

  cr:rectangle(0,0,mrg:width(),mrg:height())
  mrg:listen(Mrg.MOTION, function (event, d1, d2)
       x = event.x; y = event.y;
       event.mrg:queue_draw(NULL)
       return 0
     end)

  mrg:add_binding("control-q", NULL, NULL, function (foo) mrg:quit() return 0 end)

  -- draw a circle for mouse cursor
  cr:new_path()
  cr:set_source_rgb(0,0,0)
  cr:arc (x, y, 20, 0, 3.14151*2)
  cr:stroke_preserve ()
  cr:set_source_rgba(1,1,1,0.5)
  cr:fill ()

end)
mrg:main()

