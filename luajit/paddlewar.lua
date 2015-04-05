#!/usr/bin/env luajit

--local S = require('syscall')
--S.setenv('MRG_RESTARTER','yes')
--S.setenv('MRG_BACKEND','mmm')

local Mrg  = require('mrg')
local math = require('math')
local os   = require('os')
local mrg  = Mrg.new(500, 500)

local in_menu
local game_started = false
local game_over = false

local vel_x, vel_y
local ball;
local walls;
local goals;
local top_score, bottom_score
local serve_dir = 1

function reset_game()
  in_menu = true
  game_started = false
  game_over = false
  mrg:queue_draw(nil)
  vel_x = 0
  vel_y = 0

  serve_dir = 1
  top_score    = 0
  bottom_score = 0
  
  ball = {x = 0.5, y = 0.5, w = 0.02, h= 0.02}

  paddles = {
    {x = 0.42, y = 1.0 - 0.08, w = 0.15,  h= 0.04},
    {x = 0.42, y = 0.04,       w = 0.15,  h= 0.04},
  }
  goals = {
    {x = 0.0, y= 0.99, w= 1.0, h = 0.1},
    {x = 0.0, y= -0.095, w= 1.0, h = 0.1}
  }
  walls = {
    {x = 0.0-0.15, y = 0.0, w = 0.15,  h= 1.00},
    {x = 1.0, y = 0.0,      w = 0.15,  h= 1.00}
  }
end

local ball = {x = 0.5, y = 0.5, w = 0.02, h= 0.02};

function serve_ball()
  vel_x  = 0.0 + (math.random() - 0.5) / 2
  vel_y  = (1.0 - (math.random()/2)) * serve_dir
  ball.x = 0.5
  ball.y = 0.5
end

function start_game()
  reset_game ()
  serve_ball ()
  in_menu = false
  game_started = true
end

function collides(a,b)
  if a.x+a.w<b.x or b.x+b.w<a.x or a.y+a.h<b.y or b.y+b.h<a.y then
    return false
  end
  return true
end

reset_game()

function toggle_fullscreen(event)
  event.mrg:set_fullscreen(not event.mrg:is_fullscreen()) 
end

function menu(mrg, dim)
  local cr = mrg:cr()
  cr:set_source_rgba(1,1,1,0.8)
  cr:rectangle(-mrg:width(),-mrg:height(),mrg:width()*2,mrg:height()*2)

  -- XXX: try wit Mrg.ALL here instead?
  
  mrg:listen(Mrg.DRAG_PRESS + Mrg.MOTION + Mrg.RELEASE+ Mrg.PRESS , function(event)
    event:stop_propagate()
  end)
  cr:fill()

  mrg:set_style("background:transparent;color:black;font-size: "  .. dim / 15.0 ..  "px;")

  mrg:set_edge_left(1 * mrg:em())
  mrg:set_edge_top(1 * mrg:em())

  if not game_started then

    mrg:text_listen(Mrg.TAP, function()
      in_menu = false
      start_game()
      mrg:queue_draw(nil)
    end)
    mrg:print ('start\n\n')
    mrg:text_listen_done()

  else

    mrg:text_listen(Mrg.TAP, function() in_menu = false mrg:queue_draw(nil) end)
    mrg:print('resume game\n\n')
    mrg:text_listen_done()

    mrg:text_listen(Mrg.TAP, function() start_game() end)
    mrg:print('new game\n\n')
    mrg:text_listen_done()

  end

  if mrg:is_fullscreen() == false then
    mrg:text_listen(Mrg.TAP, function() mrg:set_fullscreen(true) end)
    mrg:print('fullscreen\n\n')
    mrg:text_listen_done()
  else
    mrg:text_listen(Mrg.TAP, function() mrg:set_fullscreen(false) end)
    mrg:print('unfullscreen\n\n')
    mrg:text_listen_done()
  end

  mrg:text_listen(Mrg.TAP, function() mrg:quit() end)
  mrg:print('quit\n\n')
  mrg:text_listen_done()

  cr:set_source_rgba(1,1,0,0.2)
  cr:rectangle(-mrg:width(),-mrg:height(),mrg:width()*2,mrg:height()*2)
  cr:fill()
end

----------------------------------------------------------------
--

local prev_time = mrg:ms()

mrg:set_ui(function()
  local cr = mrg:cr()
  local dim = mrg:width()
  local time = mrg:ms()
  local delta_ms = time - prev_time
  prev_time = time

  if dim > mrg:height() then dim = mrg:height() end

  cr:set_source_rgb(0,0,0)
  cr:paint()

  mrg:set_style("background:transparent;color:rgba(255,255,255,0.5);font-size: "  .. dim / 5.0 ..  "px;")

  cr:translate((mrg:width()-dim)/2, (mrg:height()-dim)/2)
  mrg:set_edge_left(1 * mrg:em())
  mrg:set_edge_top(-4.7 * mrg:em())

  cr:save()
  cr:rotate(3.141512/2)

  mrg:print('' .. top_score .. '   ' .. bottom_score )
  cr:restore()


  cr:save()
  cr:scale(dim, dim)

  cr:rectangle(0,0,1,1)
    mrg:listen(Mrg.TAP_AND_HOLD, function()
      in_menu = true
      mrg:queue_draw(nil)
    end)
  cr:new_path()

  cr:set_source_rgb(1,1,1)

  cr:rectangle(ball.x,ball.y,ball.w,ball.h)
  cr:fill()

  for k, v in ipairs(paddles) do
     cr:rectangle(v.x,v.y,v.w,v.h)
     cr:fill()
  end

  --cr:set_source_rgba(1,1,1,0.1)

  cr:rectangle(0.0, 0.7, 1.0, 0.3)
  mrg:listen(Mrg.DRAG, function(event)
    paddles[1].x = event.x - paddles[1].w/2;
  end)
  cr:new_path()

  cr:rectangle(0.0, 0.0, 1.0, 0.3)
  mrg:listen(Mrg.DRAG, function(event)
    paddles[2].x = event.x - paddles[2].w/2;
  end)
  cr:new_path()

  cr:restore()

  mrg:set_style("background:transparent;color:yellow;font-size: "  .. dim / 15.0 ..  "px;")
  mrg:set_edge_left(1 * mrg:em())
  mrg:set_edge_top(1.5 * mrg:em())

  if game_over then
      cr:rectangle(0,0, mrg:width(), mrg:height())
      mrg:listen(Mrg.TAP, function() reset_game() end)
      cr:set_source_rgba(0,0,0,0.75)
      cr:new_path()
      cr:paint()

      mrg:print ("game over\n\n")
      mrg:set_style("background:transparent;color:white;font-size: "  .. dim / 5.0 ..  "px;")
      mrg:print ("\n    ↻")
  end

  if in_menu then
    menu (mrg, dim)
  end

  -- some keybindings 
  mrg:add_binding("control-q", NULL, NULL, function () mrg:quit() end)
  mrg:add_binding("q", NULL, NULL,         function () mrg:quit() end)
  mrg:add_binding("escape", NULL, NULL,    function () mrg:quit() end)
  mrg:add_binding("F2", NULL, NULL,        function () reset_game() end)
  mrg:add_binding("r", NULL, NULL,         function () reset_game() end)
  mrg:add_binding("f", NULL, NULL,   toggle_fullscreen)
  mrg:add_binding("F11", NULL, NULL, toggle_fullscreen)

  ball.y = ball.y + vel_y * delta_ms / 1000.0
  ball.x = ball.x + vel_x * delta_ms / 1000.0

  if (collides (ball,paddles[1])  or
      collides (ball,paddles[2])
      ) then
  --  vel_x = - vel_x
    vel_y = - vel_y

    ball.y = ball.y + vel_y * delta_ms / 1000.0
    ball.x = ball.x + vel_x * delta_ms / 1000.0
  end

  if (collides (ball,walls[1]) or
      collides (ball,walls[2])) then
    vel_x = - vel_x

    ball.y = ball.y + vel_y * delta_ms / 1000.0
    ball.x = ball.x + vel_x * delta_ms / 1000.0
  end

  if (collides (ball, goals[1])) then
    top_score = top_score + 1
    serve_dir = -1
    serve_ball ()
  end

  if (collides (ball, goals[2])) then
    bottom_score = bottom_score + 1
    serve_dir = 1
    serve_ball ()
  end


  -- we must queue this, since painting itself clear the needs-redraw state,
  -- maybe lift this restriction since it is an unneded gotcha..
  mrg:add_timeout(0, function()
    mrg:queue_draw(nil)
    return 0
  end)

end)

math.randomseed( os.time() )

mrg:set_title('paddle game')
mrg:main()
