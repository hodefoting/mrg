#!/usr/bin/env luajit

local Mrg = require('mrg')
local math = require('math')
local os = require('os')
local mrg = Mrg.new(-1,-1)

local BLANK = 0
local LIGHT = 1
local DARK = 2

local AI = 0
local HUMAN = 1

local DARK_player 
local LIGHT_player
local in_menu

local board = {}

local piece_count
local to_play
local passes = 0

local show_legal = false

function reset_game()
  for x = 1, 8 do 
    board[x] = {}
    for y = 1, 8 do
      board[x][y] = BLANK
    end
  end
  board[4][4] = LIGHT
  board[5][5] = LIGHT
  board[4][5] = DARK
  board[5][4] = DARK

  DARK_player = HUMAN
  LIGHT_player = HUMAN
  in_menu = true
  piece_count = 0
  passes = 0

  to_play = DARK
  mrg:queue_draw(nil)
end

reset_game()

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
  if self == LIGHT then other = DARK
                   else other = LIGHT
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
  if self == LIGHT then
    other = DARK
  else
    other = LIGHT
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
  if self == LIGHT then
    other = DARK
  else
    other = LIGHT
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


function menu(mrg, dim)
  local cr = mrg:cr()
  cr:set_source_rgba(1,1,1,0.8)
  cr:rectangle(-mrg:width(),-mrg:height(),mrg:width()*2,mrg:height()*2)
    mrg:listen(Mrg.PRESS , function(event)
      event:stop_propagate()
    end)
  cr:fill()

  mrg:set_style("background:transparent;color:black;font-size: "  .. dim / 15.0 ..  "px;")

  mrg:set_edge_left(1 * mrg:em())
  mrg:set_edge_top(1 * mrg:em())

  if piece_count == 4 then

    mrg:text_listen(Mrg.TAP, function()
      DARK_player  = HUMAN
      LIGHT_player = HUMAN
      in_menu = false
      mrg:queue_draw(nil)
    end)
    mrg:print ('human vs human\n\n')
    mrg:text_listen_done()

    mrg:text_listen(Mrg.TAP, function()
      DARK_player  = HUMAN
      LIGHT_player = AI
      in_menu = false
      mrg:queue_draw(nil)
    end)
    mrg:print ('human vs AI\n\n')
    mrg:text_listen_done()

    mrg:text_listen(Mrg.TAP, function()
      DARK_player  = AI
      LIGHT_player = AI
      in_menu = false
      mrg:queue_draw(nil)
    end)
    mrg:print ('AI vs AI\n\n')
    mrg:text_listen_done()

  else

    mrg:text_listen(Mrg.TAP, function() in_menu = false mrg:queue_draw(nil) end)
    mrg:print('resume game\n\n')
    mrg:text_listen_done()

    mrg:text_listen(Mrg.TAP, function() reset_game() end)
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

function ai()
  -- this ai is good enough to beat humans not paying attention
  -- and good enough for practice runs before humans playing against
  -- each other

  local coord_score = {
     { 1,51,10,26,27, 9,52, 4},
     {50,58,18,41,42,25,59,49},
     { 5,19,16,33,34,15,24, 8},
     {11,48,35,61,63,40,43,29},
     {32,47,36,62,64,39,44,30},
     {12,13,20,37,38,14,23,17},
     {56,57,21,46,45,22,60,53},
     { 2,55, 6,31,28, 7,54, 3}}

  local best_x = 0
  local best_y = 0
  local best_score = 99
  for x = 1, 8 do
    for y = 1, 8 do
    -- we randomly jitter the score in the table, to make avoid
    -- having the ai be fully predictable
      if is_legal(x, y) and coord_score[x][y] - math.random() * 4.8 < best_score then
        best_x = x
        best_y = y
        best_score = coord_score[x][y]
      end
    end
  end
  return best_x, best_y
end

function ai_random ()
  local best_x = 0
  local best_y = 0
  local best_score = 99
  for x = 1, 8 do
    for y = 1, 8 do
      local score = math.random() * 50

      if is_legal(x, y) and score < best_score then
        best_x = x
        best_y = y
        best_score = score
      end
    end
  end
  return best_x, best_y
end

local ai_id = 0

mrg:set_ui(function()
  local cr = mrg:cr()

  local legal_moves = 0
  local white = 0
  local black = 0
  local dim = mrg:width()
  local is_human = true
  local other
  if to_play == LIGHT then
    other = DARK
  else
    other = LIGHT
  end

  if to_play == LIGHT and LIGHT_player == AI then is_human = false end
  if to_play == DARK  and DARK_player == AI then is_human = false end

  if dim > mrg:height() then dim = mrg:height() end

  cr:translate((mrg:width()-dim)/2, (mrg:height()-dim)/2)

  cr:save()
  cr:scale(dim, dim)

  cr:set_source_rgb(0.17,0.17,0.17)
  cr:paint()

  cr:rectangle(0,0,1,1)
  cr:set_source_rgba(0,0.5,0,1)
    mrg:listen(Mrg.TAP_AND_HOLD, function()
      in_menu = true
      mrg:queue_draw(nil)
    end)
  cr:fill()

  for x = 1, 8 do
    for y = 1, 8 do
      if board[x][y] == LIGHT then
        cr:set_source_rgba (1,1,1,1)
        cr:arc (1.0/8 * (x-0.5), 1.0/8 * (y-0.5), 1.0/9/2, 0, 3.1415*2)
        cr:fill ()
        white = white + 1
      elseif board[x][y] == DARK then
        cr:set_source_rgba (0,0,0,1)
        cr:arc (1.0/8 * (x-0.5), 1.0/8 * (y-0.5), 1.0/9/2, 0, 3.1415*2)
        cr:fill ()
        black = black + 1
      elseif is_human and is_legal(x,y) then
        cr:set_source_rgba (1,1,0,0.2)
        cr:rectangle(1.0/8 * (x-1), 1.0/8 * (y-1), 1.0/8, 1.0/8)
        mrg:listen(Mrg.PRESS, function(ev)
          put_piece(x,y)
          mrg:queue_draw(nil)
        
        end)
        if show_legal then
          cr:fill ()
        else
          cr:new_path()
        end
      else
      end

      if is_legal(x,y) then
        legal_moves = legal_moves + 1
      end
    end
  end
  piece_count = white + black

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

  mrg:set_style("background:transparent;color:yellow;font-size: "  .. dim / 15.0 ..  "px;")
  mrg:set_edge_left(1 * mrg:em())
  mrg:set_edge_top(1.5 * mrg:em())

  if not is_human and legal_moves > 0 then
    mrg:print '…'

    if ai_id == 0 and legal_moves > 0 then
      ai_id = mrg:add_timeout(250, 
         function()
           put_piece(ai()) 
           mrg:queue_draw(nil)
           ai_id = 0
           return 0
         end
      )
    end
  end

  if legal_moves == 0 then

    if passes == 0 then
      mrg:print ("\nforced pass\n\n")
      passes = passes + 1
      to_play = other
      if to_play == WHITE then
        mrg:print ('light to play')
      else
        mrg:print ('dark to play')
      end
      mrg:add_timeout (500, function() mrg:queue_draw (nil) return 0 end)
    else

      cr:rectangle(0,0, mrg:width(), mrg:height())
      mrg:listen(Mrg.TAP, function() reset_game() end)
      cr:set_source_rgba(0,0,0,0.75)
      cr:new_path()
      cr:paint()

      if (white > black) then
        mrg:print ("light wins\n\n")
        mrg:print ("light: " .. white .. "\n")
        mrg:print ("dark: " .. black .. "\n")
      elseif (black > white) then
        mrg:print ("darkness wins\n\n")
        mrg:print ("dark: " .. black .. "\n")
        mrg:print ("light: " .. white .. "\n")
      else
        mrg:print ("nobody wins\n\n")
        mrg:print ("light: " .. white .. "\n")
        mrg:print ("dark: " .. black .. "\n")
      end
      mrg:set_style("background:transparent;color:white;font-size: "  .. dim / 5.0 ..  "px;")
      mrg:print ("\n    ↻")
    end
  else
    passes = 0
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
end)

math.randomseed( os.time() )

mrg:set_title('flipping game')
mrg:main()
