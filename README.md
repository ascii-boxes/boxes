[![Build Status](https://travis-ci.org/ascii-boxes/boxes.svg?branch=master)](https://travis-ci.org/ascii-boxes/boxes)


```
     _.
    | |                                                      ____     ____ 
    | |__     ____   ___  ___   ____     ______     ___  __ /_   |   /_   |
    | __ \   /  _ \  \  \/  / _/ __ \   /  ___/     \  \/ /  |   |    |   |
    | \_\ \ (  <_> )  >    <  \  ___/   \___ \       \   /   |   |    |   |
    |___  /  \____/  /__/\_ \  \___  > /____  >       \_/    |___| /\ |___|
        \/                 \/      \/       \/                     \/

           draws all kinds of boxes around your text (in ASCII art)

                            [stable release 1.1.1]
```


###GETTING IT

http://boxes.thomasjensen.com/

Boxes is free software under the GNU General Public License (GNU GPL),
version 2. See LICENSE file or
http://www.gnu.org/licenses/old-licenses/gpl-2.0.html.


###COMPILING IT

Skip this section if you got yourself a binary distribution. Note,
however, that the location of the system-wide config file can only be
changed using the source distribution, because its path is compiled in.

1. Unpack the archive.
2. Edit the top level *Makefile* and change the value of `GLOBALCONF` to
   whereever you want the system-wide config file to reside. Note that
   each user may have his/her own config file in *$HOME/.boxes*.
   Also note that the value of `GLOBALCONF` is a full file name. It does
   **not** specify a directory into which to copy the config file.

3. If you are on DEC/OSF, edit *src/regexp/Makefile*, and add
   `-D_ANSI_C_SOURCE` to the `CFLAGS` definition.

4. Type `make` in the top level directory.<br/>
   This should leave you with the binary in the *src* directory.

The boxes development platform is intel/linux (SLES). If you are on a
different platform and somehow manage to materialize a binary, please
send it to the author (boxes at thomasjensen.com), so it can be posted on
the boxes home page. Boxes is generally very easy to port, because it
uses only a small set of instructions and is comformant to the ANSI C
standard.

For a small collection of frequently asked questions, visit the
[compilation FAQ](https://github.com/ascii-boxes/boxes/wiki/FAQ#q-5-compilation)
on the boxes website.


###INSTALLING IT

1. Basic installation
  - If you are installing a BINARY distribution:
    - Copy *boxes.1* to an appropriate manual page directory.
    - Copy *boxes-config* to */usr/share/boxes*.
      Note that */usr/share/boxes* is the name of the file,
      and **not** the name of a directory into which to copy the file.
      So, the name of the config file is changed from *boxes-config*
      to just *boxes*.
    - Copy the executable to a location which is in your path.<br/>
      *Example* (as root):<pre>cp boxes.1 /usr/share/man/man1
       cp boxes-config /usr/share/boxes
       cp boxes /usr/bin</pre>
  - If you are installing a SOURCE distribution:
    - Copy *doc/boxes.1* to an appropriate manual page directory.
    - Copy *boxes-config* to the location specified in the Makefile
      as being the name of the system-wide boxes config file.
      Note that the value of `GLOBALCONF` is the name of the file,
      and **not** the name of a directory into which to copy the file.
      So, the name of the config file is changed from *boxes-config*
      to just *boxes*.
    - Copy *src/boxes* (the binary) to a location which is in your path.<br/>
      *Example* (as root):<pre>cp doc/boxes.1 /usr/share/man/man1
       cp boxes-config /usr/share/boxes
       cp src/boxes /usr/bin</pre>
2. If you want to make your own changes to the config file, copy the
   system-wide config file into your home as *$HOME/.boxes*, then modify
   it. Boxes will use *$HOME/.boxes* if it exists.

3. Integration into your text editor: Please see the
   [editor integration page](http://boxes.thomasjensen.com/docs/install.html)
   on the website.
   It currently describes integration with vim, emacs, and jed, but
   this list may grow as time goes by and people send how-tos.


###RUNNING IT

Try if boxes works by typing `echo foo | boxes` on the command line.
It should give you something like that:

    /*******/
    /* foo */
    /*******/

Detailed usage information is in the manual page and a lot more can be
found on the website (see above). It is especially interesting to tie
boxes to your editor, so that it can be invoked by a simple keypress.


###DEFINING YOUR OWN BOXES

Any user may define his or her own box designs by creating a file called
*.boxes* in his or her home directory. The syntax of the config file is
explained on the boxes home page, along with several examples:

http://boxes.thomasjensen.com/docs/


###CONTACT

- For questions which the online documentation could not answer,
  praise and general comments please feel free to email the author at
  boxes at thomasjensen.com. Bug reports are especially appreciated,
  but those should, like feature requests, be opened on Github.

- For questions particular to the binary you downloaded contact the
  supplier of the binary listed on the download page.

- I appreciate the creative new box designs that you craft for boxes.
  Please feel free to submit them in the form of pull requests on
  *boxes-config*, and they may make it into the next release!

```
                                          _________________________
                                         /\                        \
                                         \_|  Thank you for        |
                                           |  using Boxes. Enjoy!  |
                                           |   ____________________|_
                                           \_/______________________/
```
10/19/2012

<tt>EOF</tt>
