#!/usr/bin/env luajit

local Mrg = require('mrg')
local mrg = Mrg.new(512,384)

local shapes={
  {e = Mrg.TAP,          x=60,    y=175, rad=20, stopPropagate=0, r=0,g=1,b=0,a=0.8},
  {e = Mrg.TAP_AND_HOLD, x=180, y=175, rad=20, stopPropagate=1, r=1,g=0,b=0,a=0.8},
  {e = Mrg.PRESS,        x=60, y=300, rad=20, stopPropagate=1, r=1,g=0,b=0,a=0.8},
  {e = Mrg.RELEASE,      x=180, y=300, rad=20, stopPropagate=1, r=1,g=0,b=0,a=0.8},
}

local pan_y = 0
local pan_x = 0

mrg:set_ui(function()
  local cr = mrg:cr()
    mrg:print('tap, tap and hold\n')
    mrg:print('press, release\n')
    mrg:print('\nthe rectangle pans all, but blocks only some')
    mrg:print('\nthe tap and tap_and_hold not getting the events through the rectangle is a bug')

  cr:translate(pan_x, pan_y)
  for k,v in ipairs(shapes) do
    cr:new_path()
    cr:set_source_rgba (v.r,v.g,v.b,v.a)
    cr:arc (v.x, v.y, v.rad, 0, 3.1415*2)
    mrg:listen(v.e, function(event)
      v.rad = v.rad + 3
      mrg:queue_draw(NULL)
      event.stop_propagate = v.stopPropagate
    end)
    cr:fill ()
    --mrg:print ('x=' .. v.x .. ' y=' .. v.y .. ' rad=' .. v.rad .. '\n')
  end

  cr:rectangle(60,175,120,125)
  cr:set_source_rgba(0,0,1,0.2)
  mrg:listen(Mrg.DRAG, function(event)
    pan_y = pan_y + event.delta_y
    pan_x = pan_x + event.delta_x
    mrg:queue_draw(nil)
  end)
  cr:fill()
end)

mrg:main()


