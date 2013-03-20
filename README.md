tglgrid
=======

A tglgrid is a grid of cells that can each be toggled on and off.  It
can be instantiated with different dimensions depending on your
project's needs.

You can bang the left input to output a list of the active cells in
the currently active column and send a column number to the right input
to select the active column.

Right now this has only been tested on linux.  I would appreciate any
reports about success and/or failure of running this in other
environments.

It's probably easier to just look at this screenshot:
![tglgrid screenshot](https://github.com/nicklan/tglgrid/raw/master/screenshot/tglgrid.png)

The included help patch should explain things thoroughly as well.

Installation
------------
1. [Possibly] Edit the Makefile to have the correct include dir for
m_pd.h
2. Type make
3. Either copy tglgrid.pd_[arch] and tglgrid-help.pd to a directory in
your pd search path, or add the tglgrid directory to your search
path.

Coming (Hopefully) Soon(ish)
----------------------------
- Properties dialog to set behavior and some apperance options (color,
spacing, cell size...)
- Support messages on first inlet to turn on/off particular cells, and
query cell state
- Fix bug where a cell stays red when the mouse leaves the object too
quickly
- Toggle -> hold down shift -> click should fill intervening cells