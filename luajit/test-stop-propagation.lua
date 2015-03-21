#!/usr/bin/env luajit

local Mrg = require('mrg')
local mrg = Mrg.new(512,384)

local shapes={
  {x=240, y=175, rad=40, stopPropagate=0, r=0,g=1,b=0.02,a=0.8},
  {x=182, y=251, rad=80, stopPropagate=1, r=1,g=0,b=0,a=0.8},
  {x=292, y=249, rad=80, stopPropagate=0, r=0,g=0,b=1,a=0.8},
  {x=392, y=249, rad=80, stopPropagate=0, r=0,g=0,b=1,a=0.8},
}

mrg:set_ui(function()
  local cr = mrg:cr()
    mrg:print('the red one stops propagation, the blue doesnt, the green one grows on press\n')
  for k,v in ipairs(shapes) do
    cr:new_path()
    cr:set_source_rgba (v.r,v.g,v.b,v.a)
    cr:arc (v.x, v.y, v.rad, 0, 3.1415*2)
    if k == 1 then
      mrg:listen(Mrg.PRESS, function(event)
        v.rad = v.rad + 2
      end)
      mrg:listen(Mrg.RELEASE, function(event)
        v.rad = v.rad - 2
      end)
      mrg:listen(Mrg.MOTION, function(event)
        if v.r > 1 then
          v.b = -0.02
        end
        if v.r < 0 then
          v.b = 0.02
        end
        v.r = v.r + v.b
      end)
    else
    mrg:listen(Mrg.DRAG, function(event)
      v.x = v.x + event.delta_x
      v.y = v.y + event.delta_y
      mrg:queue_draw(NULL)
      event.stop_propagate = v.stopPropagate
    end)
    end
    cr:fill ()
    mrg:print ('x=' .. v.x .. ' y=' .. v.y .. ' rad=' .. v.rad .. '\n')
  end
end)

mrg:main()


