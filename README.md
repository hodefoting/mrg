microraptor gui
===============

Explorations of alternative means of interactive user interfaces.
Incrementally augment the cairo API with interactivity and CSS powered
text-layout. This permits interesting ability to experiment with the
alternate mental model to create user interafaces in, most other
frameworks are retained mode - like motif, gtk, qt, clutter, svg/html DOM -

environment built using the framework, including: a shell/host for client
programs, a terminal emulator, file browser and text editor written using the
library.

Features:
* SDL/GTK/framebuffer/terminal backends.
* PNG/JPG image caching

microraptor gui is a C API for creating user interfaces, it can be used as an
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
-----------------------

MicroRaptor Gui has three independent backends it can use: SDL-1.2, GTK+-3.0
and ANSI/vt100 terminals.

The GTK backend provides both full application hosting,
and a widget for use as a canvas in GTK+applications. The terminal
backends has mouse support, is utf8 native and for simple text-centric
applications you can even build static libraries skipping even the cairo
support.

Build/run
---------

Clone both mrg and mmm (from neighboring git locations), and have alsa, SDL-1.2
and GTK+-3.0 development packages installed.`

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

Luajit ffi
----------

All of the mrg C API maps easily to lua when using luajit ffi, binds easily
with luajit ffi. See the `./luajit` dir for an example, that also bundles the
needed cairo bindings.

Microraptor and GTK
-------------------

Microraptor can use GTK as a backend or microraptor can be used in GTK+
applications, when used in gtk applications in widgets it is better to rely on
native mainloop integration with `g_timeout` and `g_idle` instead of similar library functionality provided by microraptor gui.

An immediate mode UI compromise
-------------------------------
     
In terms of programming paradigm, microraptor contains aspects of 
both traditional retained widget-tree/scene-graph ui's and 
purist [immediate mode uis](http://iki.fi/sol/imgui/).
Since microraptor doesn't have a retained scenegraph, and some of
the freedoms gained with imgui, like being able to paint
incrementally directly from data-structures and free form logic to
structure things.
