boxes 1.0.1 Win32 port Information                   Date: 18-Mar-2000


This is the Win32 Version of boxes 1.0.1, built from the updated snapshot
of 18-Mar-2000, currently the most stable source.  It was built using
gcc-2.95.2 with mingw32, plus a C source file containing the GNU getopt()
implementation.
Many thanks to Ron Aaron <ron@mossbayeng.com> for helping me with this
port!

General information may be found in the generic README file provided with
this archive and at http://boxes.thomasjensen.com/, the boxes home page.
Note that the generic README file is intended for UNIX users, so some
things don't translate literally (e.g., you would have to type Ctrl-Z
instead of Ctrl-D to signal end of file, etc.).

The system-wide config file is assumed to reside in the same directory as
the boxes executable. (This is the case when you unpack the archive.) The
name of the config file is assumed to be the same as the name of the
executable, only with a .cfg extension instead of .exe.  As with the UNIX
version, boxes still looks for a file boxes.cfg in the directory pointed
to by the environment variable HOME, and for a file pointed to by the
environment variable BOXES.

The standard troff format manual page is part of the package (boxes.1),
but knowing that in the Windows world there may be some people who don't
have troff installed on their systems, an HTML version of the page is
provided. It is a little out of date, but not a whole lot.


Enjoy!

Thomas Jensen <boxes@thomasjensen.com>


EOT
