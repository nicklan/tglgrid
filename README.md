tglgrid
=======

A tglgrid is a grid of cells that can each be toggled on and off.  It
can be instantiated with different dimensions depending on your
project's needs.

You can bang the left input to output a list of the active cells in
the currently active column and send a column number to the right input
to select the active column.

There is also an "rtglgrid" that operates row-wise.

The size of the grid can be dynamically changed by sending a "size"
message, or in the properties dialog.

Cells can also be toggled and/or turned on/off by sending messages:
"tgl col row", "on col row", or "off col row".

You can get particular cells, columns, or rows with the messages:
"say col row", "csay col", or "rsay row"

Grid size, cell size, spacing, and the colors for toggled and
un-toggled cells can be set in a properties dialog.

Right now this has only been tested on linux.  I would appreciate any
reports about success and/or failure of running this in other
environments.

It's probably easier to just look at these screenshots:
![tglgrid screenshot](https://github.com/nicklan/tglgrid/raw/master/screenshot/tglgrid.png)
![tglgrid screenshot](https://github.com/nicklan/tglgrid/raw/master/screenshot/help.png)

The included help patch should explain things thoroughly as well.

Installation
------------
1. [Possibly] Edit the Makefile to have the correct include dir for
m_pd.h
2. Type make
3. Either copy tglgrid.pd_[arch] and tglgrid-help.pd to a directory in
your pd search path, or add the tglgrid directory to your search
path.