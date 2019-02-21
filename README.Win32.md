# boxes Win32 port Information

**Date:** 16-Mar-2017

This is the Win32 Version of boxes, built from the official source tarball.
Many thanks to **Ron Aaron** for making this Windows build possible!

General information may be found in the generic README file provided with
this archive and at https://boxes.thomasjensen.com/, the boxes home page.
Note that the generic README file is intended for UNIX users, so some
things don't translate literally (e.g., you would have to type Ctrl-Z
instead of Ctrl-D to signal end of file, etc.).

The system-wide config file is assumed to reside in the same directory as
the boxes executable. (This is the case when you unpack the archive.) The
name of the config file is assumed to be the same as the name of the
executable, only with a `.cfg` extension instead of `.exe`.  As with the UNIX
version, boxes still looks for a file called *boxes.cfg* in the directory
pointed to by the environment variable `HOME`, and for a file pointed to by
the environment variable `BOXES`.

The standard troff format manual page is part of the package (boxes.1),
but knowing that in the Windows world there may be some people who don't
have troff installed on their systems, an HTML version of the page is
provided.


EOT
