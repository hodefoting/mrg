#!/usr/bin/env luajit

local Math = require('math')
local string = require('string')
local Mrg = require('mrg')
local mrg = Mrg.new(512,512);

local Lyd = require('lyd')
local lyd = Lyd.new()

local instrument = [[

saw (hz * (1.0 + sin(tremolo=5)*tremolo_amount=0.05)) *
     adsr(0.1,0,1.0,0.1)

#pluck (hz=440) * adsr(0.0,0,vol=4,5.0)
# try these other formulas by commenting out the above an uncommenting the other
# square (hz) * adsr(0.1,0,1.0,0.1)
# sin (hz) * adsr(0.1,0,1.0,0.1)
# saw (hz) * adsr(0.1,0,1.0,0.1)
# pluck( hz * (1.0 + sin(21)/100)) + pluck( hz * (1.0 + sin(20)/100)) + pluck( hz * (1.0 + sin(22)/100))
]]

function note_to_hz(no)
  local hz = 55/4
  local twelfth_root_of_two = 1.0594630943592952645
  for step = 1, no do
    hz = hz * twelfth_root_of_two;
  end
  return hz
end

local parameters = {}
local ffi=require 'ffi'

function update_instrument(instrument)
  parameters = {}

  lyd:set_var_handler(function(lyd, var_name, def_val)
    --print ("got var: " .. ffi.string(var_name) .. " def: " .. def_val)
    local param = {}
    param.default = def_val
    param.value = def_val
    param.min_value = 0.0
    if (param.default < 0.1) then
      param.max_value = 0.1
    elseif (param.default < 1.0) then
      param.max_value = 1.1
    elseif (param.default < 10.0) then
      param.max_value = 11.0
    elseif (param.default < 100.0) then
      param.max_value = 110.0
    else
      param.max_value = 1100.0
    end
    parameters[ffi.string(var_name)] = param
  end)
  lyd:set_patch(100, instrument)
end

lyd:set_patch(1, 'saw(hz)*adsr(0.1,0.5,1.0,0.1) * 0.5')

local entered = {}


function draw_piano(mrg, shape)
  local cr = mrg:cr()
  local note = 1;
    --cr:set_source_rgb(0,1,0)
    --cr:fill();

  local radius0 = mrg:height() * 0.5
  local radius1 = mrg:height() * 0.3

  --for note = 24, 60 do
  for note = 0, 128 do
    cr:new_path()
    local sec_start = note-0.5
    local sec_end   = note+0.5

    if shape=='spiral' then
      cr:new_path()
      cr:arc(mrg:width()/2, mrg:height()/2, 
           radius0,
           (sec_start / 12.0) * (2 * 3.14152),
           (sec_end   / 12.0) * (2 * 3.14152));
      cr:arc_negative(mrg:width()/2, mrg:height()/2, 
           radius1,
           (sec_end   / 12.0) * (2 * 3.14152),
           (sec_start / 12.0) * (2 * 3.14152));
      cr:close_path();
    elseif shape=='grid' then
      cr:new_path()
      cr:rectangle((note % 12) * mrg:width()/12,
                   (10-Math.floor(note / 12)) * mrg:height()/14,
                   mrg:width()/12, mrg:height()/10)
      cr:close_path();
    else
      cr:new_path()
      cr:rectangle((note) * mrg:width()/128,
                   mrg:height() - mrg:height()/10,
                   mrg:width()/128, mrg:height()/10)
      cr:close_path();
    end

    mrg:listen(Mrg.ENTER, function(event,a,b)
      entered[note] = true;
      if (event.state > 0) then
        lyd:note(100, note_to_hz(note), 1.0, 0.1)
      end
      mrg:queue_draw(NULL)
      event.stop_propagate = 1
    end)
    mrg:listen(Mrg.PRESS, function(event,a,b)
      lyd:note(100, note_to_hz(note), 1.0, 0.1)
      event.stop_propagate = 1
    end)
    mrg:listen(Mrg.LEAVE, function(event,a,b)
      entered[note] = false;
      mrg:queue_draw(NULL)
      event.stop_propagate = 1
    end)

    cr:set_source_rgba(1,1,1,1)
    cr:fill_preserve ();
    cr:set_source_rgb(1,0,0)
    if (entered[note]) then
      cr:fill ();
    else
      cr:stroke ();
    end

    local foo = 0.980
    radius1 = radius1 * foo
    radius0 = radius0 * foo
  end
end

function draw_circle_cursor (mrg)
  local cr = mrg:cr()
  cr:new_path()
  cr:set_source_rgb(0,0,0)
  cr:arc (mrg:pointer_x(), mrg:pointer_y(), 20, 0, 3.14151*2)
  cr:stroke_preserve ()
  cr:set_source_rgba(1,1,1,0.5)
  cr:fill ()
end

function draw_parameter_uis(mrg)
  local cr = mrg:cr()
  local em = mrg:em()

  local count = 0

  for name,param in pairs(parameters) do
    if (name == "hz") then
      cr:save()
      if (param.default == 440) then
        cr:translate(0, mrg:height()/2)
        cr:scale(0.5,0.5)
        draw_piano(mrg, 'spiral')
      elseif (param.default == 100) then
        cr:translate(0, mrg:height()/2)
        cr:scale(0.5,0.5)
        draw_piano(mrg, 'grid')
      else
        draw_piano(mrg, 'piano')
      end
      cr:restore()
    elseif (string.find(name, "knob") == 1) then
      mrg:print('kNOB :' .. name .. ' ' .. param.default .. '\n')
    else
      mrg:set_xy (2*em, 1*em + count * 3 * em + 1.2 * em)
      mrg:print(name .. ':' .. param.value .. '\n')
      cr:rectangle(1*em, 1*em + count * 3 * em,
                    mrg:width() - 2*em,
                    2*em)
      mrg:listen(Mrg.DRAG, function(event, data1, data2)
        param.value = 
          (event.x - 1*em) / (mrg:width()-2*em) *
          (param.max_value-param.min_value) + param.min_value
        mrg:queue_draw(nil)
      end)
      mrg:listen(Mrg.ENTER + Mrg.LEAVE, function(event, data1, data2)
        mrg:queue_draw(nil)
      end)

      cr:save()
      if (cr:in_fill(mrg:pointer_x(), mrg:pointer_y() )) then
        cr:set_line_width(4)
      else
        cr:set_line_width(2)
      end
      cr:stroke()
      cr:restore()

      cr:save()
      cr:rectangle(1*em, 1*em + count * 3 * em,
                   (mrg:width() - 2*em) * 
                   (param.default - param.min_value) /
                   (param.max_value - param.min_value)
                   ,
                   2*em)
      cr:clip()
      cr:paint_with_alpha(0.08)
      cr:restore()

      cr:save()
      cr:rectangle(1*em, 1*em + count * 3 * em,
                   (mrg:width() - 2*em) * 
                   
                   (param.value - param.min_value) /
                   (param.max_value - param.min_value)
                   ,
                   2*em)
      cr:clip()
      cr:paint_with_alpha(0.2)
      cr:restore()

      count = count + 1
    end
  end
end

mrg:set_ui(
function (mrg, data)
  local cr = mrg:cr()

  cr:rectangle(0,0,mrg:width(),mrg:height())

  --mrg:listen(Mrg.MOTION, function (event, d1, d2) event.mrg:queue_draw(NULL) return 0 end)

  mrg:set_style('color:red;font-size:20')
  local em = mrg:em()
  mrg:set_edge_left (1.0 * em)
  mrg:set_edge_top (1.0 * em) -- setting top causes a cursor reset, so do it after left
  mrg:set_edge_right (mrg:width()-1.0 * em)
  
  draw_parameter_uis(mrg)
  
  mrg:set_style('color:black;font-size:20;font-family:mono')

  mrg:print('\n')
  mrg:edit_start(
  function(new_text,foo)
    instrument = ffi.string(new_text)
    update_instrument (instrument)
  end)
  mrg:set_style('background:transparent;syntax-highlight:C')
  mrg:print(instrument)
  mrg:edit_end() -- or should it just be the next print statement?

  --mrg:print_xml('<h2>Spiral chromatic instrument</h2><p>this ui renders a chromatic set of buttons/tangents in a spiral where the same note in different octaves form a line from the center.</p> ')

  -- draw_circle_cursor(mrg)

  mrg:add_binding("control-q", NULL, NULL, function (foo) mrg:quit() end)
end)

update_instrument(instrument)
mrg:set_title('microraptor synth')
mrg:main()
