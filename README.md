MicroRaptor Gui
===============

Immediate UI framework with cairo.

Features:
  cairos drawing/transformation API
  pointer/keyboard events
  CSS styling/layout
  Single pass direct XML SVG/XHTML layout/renderer (minimal state/no DOM)
  SDL/GTK/framebuffer/terminal backends.


MicroRaptor Gui is a C API for creating user interfaces, it can be used as an
application writing environment or as an interactive canvas for part of a
larger interface.

Micro raptor builds on/integrates with cairos immediate mode vector rendering
API
      
Callbacks registered during the painting of the frame - the cairo
transformations in place during registration applies when called. Also works
for dragging. Keybindings are also registered in the same callback for
building the ui. Thus the mental model is that one - in one call -
repaint/recreate the UI drawing and callbacks from data-state and view-state
in one call, avoiding much caching and intermediate objects.

Embeddable and stand-alone - as a "canvas" mrg can be used as widgets
in Gtk+ applications or full-blown applications.

CSS styled rendering (using a context stack; where the pushing of contexts
specifies the "element.class#id" of each now element rendering entered.

Backends / dependencies
=======================

MicroRaptor Gui has three independent backends it can use, SDL-1.2, GTK+-3.0
and ANSI/vt100 terminals, the GTK backend provides both full application
hosting, and a widget for use as a canvas in GTK+applications. The terminal
backends has mouse support, is utf8 native and for simple text-centric
applications you can even build static libraries skipping even the cairo
support.

Build/run
=========

Clone both mrg and mmm, and have SDL-1.2 and GTK+-3.0 development packages
installed. A 

$ make && sudo make install

in first mmm and then mrg shouler permit running:

$ mmm.sdl mrg /home/foo/..path/to/sources/of/mrg/docs/31-hello.c

To start a slide-show of source code that is live compiled and run as
interactive demos within the viewer, navigate forward/back with F12 and F11.
Prefixing mrg with mmm.sdl uses the mmm SDL host instead of the autodetected
GTK+ backend for microraptor, using GTK there is timing/update issues likely
related to mainloop integration issues.

$ mrg host

Launches a window manager/compositor host for mmm clients, which mrg
applications are.

$ mrg browser mrg:index.html

Launches the mrg XHTML+CSS renderer on a static HTML/CSS data set contained in
the mrg binary.

$ mrg edit <textfile>

Launches the editor on the text file.
      
An immediate mode UI compromise
===============================
     
In terms of programming paradigm, microraptor contains aspects of 
both traditional retained widget-tree/scene-graph ui's and 
purist http://iki.fi/sol/imgui/  immediate mode uis
Since microraptor doesn't have a retained scenegraph, and some of
the freedoms gained with imgui, like being able to paint
incrementally directly from data-structures and free form logic to
structure things.
