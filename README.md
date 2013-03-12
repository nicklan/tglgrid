tglgrid
=======

A tglgrid is a grid of cells that can each be toggled on and off.  It
can be instantiated with different dimensions depending on your
project's needs.

You can bang the left input to output a list of the active cells in
the currently active column and send a column number to the right input
to select the active column.

Right now this has only been tested on linux (hence the Makefile
targeting linux specifically), but it shouldn't be difficult to port.
Any help in this regard is much appreciated, since I don't have access
to other environments to test.


It's probably easier to just look at this screenshot:
![tglgrid screenshot](https://github.com/nicklan/tglgrid/raw/master/screenshot/tglgrid.png)

The included help patch should explain things thoroughly as well.

Installation
------------
1. Edit the Makefile to have the correct include dir for m_pd.h and
g_canvas.h
2. Type make
3. Either copy tglgrid.pd_linux and tglgrid-help.pd to a directory in
your pd search path, or add the tglgrid directory to your search
path.