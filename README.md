SoftwareRenderer
================

Software renderer created using native C language.

Folder data contains all needed data for the application (user interface for main window and texture for example scene)
Folder src contains source of whole project (renderer and sample application)
Makefile uses gtk-2.0 development libs and gtkglext.
Main is linux 64-bit binary

Whole application was created under linux OS, it should be portable on Windows (using MinGW).

Usage:
just make + run main to see software renderer. Arrow keys rotate the cube, page up/down zoom it in and out.

Note. the application can be really slow (depends on the CPU), even through that it actually uses SSE. It is not optimized for speed, but more likely designed to explain how GPU and renderers generally work.
