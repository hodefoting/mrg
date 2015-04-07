#!/usr/bin/env luajit

local document = [[Microraptor editor, a small text editor written with luajit ffi and microraptor gui, not suited for huge files, but works well for normal source-code sized files.

Usage: mrg-edit.lua <file to edit>
Keybindings: ctrl-s save ctrl-q quit
]]

local math   = require('math')
local string = require('string')
local io     = require('io')
local Mrg    = require('mrg')
local mrg    = Mrg.new(-1, -1);

local path = 'microraptor-lua-editor'

if (#arg >= 1) then
  path = arg[1]
  io.input(path)
  document = io.read("*all")
end

-- automatic cursor centeric with pan handling
local vert_pan = -20
local move_y = 0  

mrg:set_ui(
function (mrg, data)
  local cr = mrg:cr()

  if move_y < 0 then
    vert_pan = vert_pan - mrg:height()/8
  elseif move_y > 0 then
    vert_pan = vert_pan + mrg:height()/8
  end

  cr:translate (0, vert_pan)

  mrg:set_style('color:red;font-size:23')
  local em = mrg:em()
  mrg:set_edge_left (1.0 * em)
  mrg:set_edge_right (mrg:width()-1.0 * em)
  mrg:set_edge_top (1.0 * em) -- setting top causes a cursor reset, so do it after left
  
  mrg:set_style('color:black;font-size:20;font-family:mono')

  -- check if cursor is above or below screen
  local x, y = mrg:print_get_xy(document, mrg:get_cursor_pos())
  y = y + vert_pan;
  if y > mrg:height() - mrg:em() then
    move_y = -1
    mrg:queue_draw(nil)
  elseif y < mrg:em() * 2 then
    move_y = 1
    mrg:queue_draw(nil)
  else
    move_y = 0
  end

  mrg:print('\n')
  mrg:edit_start(
    function(new_text)
      document = new_text 
    end)
  mrg:set_style('background:transparent;syntax-highlight:C')
  mrg:print(document)
  mrg:edit_end() -- or should edit_start just apply to the next print statement?

  mrg:add_binding("control-q", NULL, NULL,
    function ()
      mrg:quit()
    end)

  mrg:add_binding("control-s", NULL, NULL, 
    function ()
      io.output(path)
      io.write(document)
      io.close()
    end)

  if move_y ~= 0 then
    mrg:queue_draw(nil)
  end
end)

mrg:set_title(path)
mrg:set_cursor_pos(0)
mrg:main()
