#!/usr/bin/env luajit

local Mrg = require('mrg')
local mrg = Mrg.new(500, 500)

local BLANK = 0
local WHITE = 1
local BLACK = 2

local board = {}
local to_play

function reset_board()
  for x = 1, 8 do 
    board[x] = {}
    for y = 1, 8 do
      board[x][y] = BLANK
    end
  end
  board[4][4] = WHITE
  board[5][5] = WHITE
  board[4][5] = BLACK
  board[5][4] = BLACK

  to_play = BLACK
end

reset_board()

function legal_coords(x,y)
  if x >= 1 and
     x <= 8 and
     y >= 1 and
     y <= 8 then 
     return true
  end
  return false
end

function is_legal_direction(x,y, dx, dy)
  local self = to_play
  local other
  if self == WHITE then other = BLACK
                   else other = WHITE
  end
  if legal_coords(x + dx, y + dy) and board[x+dx][y+dy] == other then
    local i = 2
    while legal_coords(x + dx * i, y + dy * i) and board[x+dx*i][y+dy*i] == other do i = i+1 end
    if legal_coords(x + dx * i, y + dy * i) and board[x+dx*i][y+dy*i] == self then return true end
  end
  return false
end

function flip_direction(x,y, dx, dy)
  local self = to_play
  local other
  if self == WHITE then
    other = BLACK
  else
    other = WHITE
  end
  if legal_coords(x + dx, y + dy) and board[x+dx][y+dy] == other then
    local i = 2
    while legal_coords(x + dx * i, y + dy * i) and board[x+dx*i][y+dy*i] == other do i = i+1 end
    if legal_coords(x + dx * i, y + dy * i) and board[x+dx*i][y+dy*i] == self then 
      for j = 1, i-1 do board[x+ dx * j][y + dy * j] = self end
    end
  end
  return false
end

function is_legal(x,y)
  if board[x][y] ~= BLANK then
    return false
  end
  if is_legal_direction(x,y, 1, 0) or
     is_legal_direction(x,y, 0, 1) or
     is_legal_direction(x,y, 1, 1) or
     is_legal_direction(x,y, -1, -1) or
     is_legal_direction(x,y, -1, 1) or
     is_legal_direction(x,y, 1, -1) or
     is_legal_direction(x,y, -1, 0) or
     is_legal_direction(x,y, 0, -1) then
     return true
  end
  return false
end

function put_piece(x,y)
  local self = to_play
  local other
  if self == WHITE then
    other = BLACK
  else
    other = WHITE
  end
  if board[x][y] ~= BLANK then
    return
  end
  board[x][y] = self;
  flip_direction(x,y, 1, 0)
  flip_direction(x,y, 0, 1)
  flip_direction(x,y, 1, 1)
  flip_direction(x,y, -1, -1)
  flip_direction(x,y, -1, 1)
  flip_direction(x,y, 1, -1)
  flip_direction(x,y, -1, 0)
  flip_direction(x,y, 0, -1)
  to_play = other
end

function toggle_fullscreen(event)
  event.mrg:set_fullscreen(not event.mrg:is_fullscreen()) 
end

local in_option_menu = false

function option_menu(mrg, dim)
  local cr = mrg:cr()
  cr:set_source_rgba(1,1,1,0.8)
  cr:rectangle(-mrg:width(),-mrg:height(),mrg:width()*2,mrg:height()*2)
    mrg:listen(Mrg.PRESS , function(event)
      event.stop_propagate = 1
    end)
  cr:fill()

  mrg:set_style("background:transparent;color:black;font-size: "  .. dim / 15.0 ..  "px;")

  mrg:set_edge_left(1 * mrg:em())
  mrg:set_edge_top(2 * mrg:em())

  mrg:text_listen(Mrg.TAP, function() in_option_menu = false mrg:queue_draw(nil) end)
  mrg:print('resume game\n\n')
  mrg:text_listen_done()

  mrg:text_listen(Mrg.TAP, function() reset_board() in_option_menu = false mrg:queue_draw(nil) end)
  mrg:print('new game\n\n')
  mrg:text_listen_done()

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


mrg:set_ui(function()
  local cr = mrg:cr()

  local legal_moves = 0
  local white = 0
  local black = 0

  local dim = mrg:width()

  if dim > mrg:height() then dim = mrg:height() end


  cr:translate((mrg:width()-dim)/2, (mrg:height()-dim)/2)

  cr:save()
  cr:scale(dim, dim)

  cr:set_source_rgb(0.17,0.17,0.17)
  cr:paint()

  cr:rectangle(0,0,1,1)
  cr:set_source_rgba(0,0.5,0,1)
    mrg:listen(Mrg.TAP_AND_HOLD, function()
      in_option_menu = true
      mrg:queue_draw(nil)
    end)
  cr:fill()

  for x = 1, 8 do
    for y = 1, 8 do
      if board[x][y] == WHITE then
        cr:set_source_rgba (1,1,1,1)
        cr:arc (1.0/8 * (x-0.5), 1.0/8 * (y-0.5), 1.0/9/2, 0, 3.1415*2)
        cr:fill ()
        white = white + 1
      elseif board[x][y] == BLACK then
        cr:set_source_rgba (0,0,0,1)
        cr:arc (1.0/8 * (x-0.5), 1.0/8 * (y-0.5), 1.0/9/2, 0, 3.1415*2)
        cr:fill ()
        black = black + 1
      elseif is_legal(x,y) then
        cr:set_source_rgba (1,1,0,0.2)
        cr:rectangle(1.0/8 * (x-1), 1.0/8 * (y-1), 1.0/8, 1.0/8)
        mrg:listen(Mrg.TAP, function(ev)
          put_piece(x,y)
          mrg:queue_draw(nil)
        
        end)
        cr:fill ()
        legal_moves = legal_moves + 1
      else
      end
    end
  end

  -- draw grid lines
  cr:set_source_rgba(0,0,0, 0.8)
  cr:set_line_width(1.0 / 300)
  for x = 0, 8 do
    cr:move_to(1.0/8 * x, 0)
    cr:line_to(1.0/8 * x, 1)
    cr:stroke()
  end
  for y = 0, 8 do
    cr:move_to(0, 1.0/8 * y)
    cr:line_to(1, 1.0/8 * y)
    cr:stroke()
  end

  cr:restore()

  if legal_moves == 0 then
    cr:rectangle(0,0, mrg:width(), mrg:height())
    mrg:listen(Mrg.TAP, function()
      reset_board()
      mrg:queue_draw(nil)
    end)
    cr:set_source_rgba(0,0,0,0.75)
    cr:new_path()
    cr:paint()
    mrg:set_style("background:transparent;color:white;font-size: "  .. dim / 15.0 ..  "px;")
    mrg:set_edge_left(1 * mrg:em())
    mrg:set_edge_top(1.5 * mrg:em())

    if (white > black) then
      mrg:print ("white wins\n\n")
      mrg:print ("white: " .. white .. "\n")
      mrg:print ("black: " .. black .. "\n")
    elseif (black > white) then
      mrg:print ("black wins\n\n")
      mrg:print ("black: " .. black .. "\n")
      mrg:print ("white: " .. white .. "\n")
    else
      mrg:print ("nobody wins\n\n")
      mrg:print ("white: " .. white .. "\n")
      mrg:print ("black: " .. black .. "\n")
    end
    mrg:set_style("background:transparent;color:white;font-size: "  .. dim / 5.0 ..  "px;")
    mrg:print ("\n    ↻")
  end

  if in_option_menu then
    option_menu (mrg, dim)
  end

  -- some keybindings 
  mrg:add_binding("control-q", NULL, NULL, function () mrg:quit() end)
  mrg:add_binding("q", NULL, NULL, function () mrg:quit() end)
  mrg:add_binding("escape", NULL, NULL, function () mrg:quit() end)
  mrg:add_binding("F2", NULL, NULL, function () reset_board() mrg:queue_draw(nil) end)
  mrg:add_binding("r", NULL, NULL, function () reset_board() mrg:queue_draw(nil) end)
  mrg:add_binding("f", NULL, NULL, toggle_fullscreen)
  mrg:add_binding("F11", NULL, NULL, toggle_fullscreen)
end)

mrg:set_title('flipping game')
mrg:main()
