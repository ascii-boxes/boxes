---
title: Building from Source
permalink: /build.html
longContent: true
created_at: 2015-03-17 15:41:27 +0100
last_modified_at: 2021-04-23 21:02:30 +0200
---

# Building from Source

Here's how to build your own version of *boxes* using the
[boxes sources](https://github.com/{{ site.github }}){:target="_blank"}.

The boxes development platform is currently Debian Linux on WSL2. The author also supports MinGW on win32 and derived
from that, a choco package for Windows.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=2 slug="unix"
   text="Building on Linux / UNIX" %}

In order to build on Linux:

1. Unzip the [source archive](https://github.com/{{ site.github }}/releases/latest){:target="_blank"}
   or [clone the GitHub
   repo](https://docs.github.com/en/github/creating-cloning-and-archiving-repositories/cloning-a-repository){:target="_blank"}.
2. Install Prerequisites:  
   The following command is for Debian (APT), but those packages have the same names across most distros.

       sudo apt-get update
       sudo apt-get install -y build-essential diffutils flex bison libunistring-dev libpcre2-dev git vim-common

   Of those, only [libunistring](https://www.gnu.org/software/libunistring/) and [PCRE2](http://www.pcre.org/) are
   libraries that are used at runtime. Everything else is just for building, including
   [Flex](https://github.com/westes/flex){:target="_blank"} and
   [Bison](http://www.gnu.org/software/bison/){:target="_blank"}. `vim-common` provides the *xxd* tool, which is used
   by some of our test cases.

3. The location of the system-wide config file is compiled in, so if you don't like it's default location, you can edit
   the top level *Makefile* and change the value of `GLOBALCONF` to whereever you want the system-wide config file to
   reside. Note that each user may have his/her own config file in *$HOME/.boxes* or other locations.  
   Also note that the value of `GLOBALCONF` is a full file name. It does **not** specify a directory into which to copy
   the config file.
4. From the top level directory, type `make && make test`. Find the resulting binary in the *out* folder. In case of
   problems, check the [compilation faq]({{ site.baseurl }}/faq.html#q5).

That should be all. *Boxes* is built so that this works almost everywhere.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3 slug="deployment-unix"
   text="Deployment" %}

In order to deploy your newly created binary on Linux/UNIX, these steps are recommended:

- Copy *doc/boxes.1* to an appropriate manual page directory.
- Copy *boxes-config* to the location specified in the Makefile
  as being the name of the system-wide boxes config file.
  Note that the value of `GLOBALCONF` is the name of the file,
  and **not** the name of a directory into which to copy the file.
  So, the name of the config file may be changed from *boxes-config* to just *boxes*.
- Copy *out/boxes* (the binary) to a location which is in your path.

Example (as root):

    cp doc/boxes.1 /usr/share/man/man1
    cp boxes-config /usr/share/boxes
    cp out/boxes /usr/bin

If you want to make your own changes to the config file, copy the system-wide config file
into your home as *$HOME/.boxes*, then modify it. Boxes will use *$HOME/.boxes* if it exists.  
Since *boxes* v2.1.0, you can also start your local config with `parent :global:`, which will inherit everything from
the global file, and you can add or override box designs in your local file.

<p></p>


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=2 slug="windows"
   text="Building on Windows" %}

Boxes was written for UNIX, but it can also run on Windows! Boxes is a 32bit application, but it works on 64bit systems,
too. Here's how to build on Windows.

> Special thanks go to Ron Aaron, who provided a specially crafted Makefile for win32 and also created the Windows
> versions of boxes that have been around to this day.

In order to build *boxes* on Windows, the required win32 executable can be created like this:

1. Install [MinGW](http://mingw-w64.org/){:target="_blank"}
2. Configure MinGW
3. Open a MinGW shell and run `make win32`

Basically very simple, but there may be a few pitfalls, so we'll go through each step in detail.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3 slug="win1"
   text="1. Install MinGW" %}

1. Download MinGW from [SourceForge](https://sourceforge.net/projects/mingw/){:target="_blank"}.
   You'll receive a small mingw-get-setup.exe.
2. Right-click mingw-get-setup.exe and choose **Run as Administrator** from the pop-up menu.
3. Press **Install**.
4. This will download the *Installation Manager*. Some time later, you can make some choices.  
   Make sure that you install to `C:\MinGW`. This is really important. If for some reason you cannot use this highly
   recommended directory, choose another one that has a short path, does not contain spaces (!), and does not include a
   Windows "special path". For example, `C:\MinGW_2`.  
   Select "install support for GUI" (yes, even though *boxes* has no GUI) and install "for all users".
5. In the Installation Manager menu, select *Installation* &rarr; *Update Catalogue*.
6. Choose the following packages:

       mingw32-base
       msys-base
       mingw32-libunistring
       mingw32-libiconv
       msys-diffutils
       msys-dos2unix
       msys-zip
       msys-unzip
   
   PCRE2 is unfortunately not available here, so we must handle this in a later step. For each of the above components,
   choose all the items (bin, doc, lic, etc.). We also don't choose flex and bison.  
   **Do not** install <tt>mingw-developer-toolkit</tt>, because if on 64bit, this requires tweaking of several
   environment variables to get gcc to use 32bit libs all around (which may be hard to get right for laymen).
7. In the menu, select *Installation* &rarr; *Apply Changes*.
   Something like 113 Packages will be installed, which may take a few minutes.
8. Upon success, select *Installation* &rarr; *Quit* from the menu.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3 slug="win2"
   text="2. Configure MinGW" %}

1. Change into C:\MinGW\msys\1.0\etc.
2. Copy the file *fstab.sample* to a new file called simply *fstab*.
3. Open *fstab* and make sure that the only lines which are not comments are these:

       c:/mingw		/mingw
       d:/path/to/boxes	/boxes

   Note that the whitespace in this example are tab characters. The d:/path/to/boxes is the path where you placed your
   clone of the boxes repo. Avoid spaces in its path, too.
4. Add C:\MinGW\bin to your system PATH.
5. Create a "MinGW Shell" shortcut somewhere (I chose my desktop). The shortcut invokes the C:\MinGW\msys\1.0\msys.bat
   script, which is installed as a component of MSYS; (if you installed to an alternative directory, you should adjust
   the C:\MinGW prefix accordingly). An icon file is provided in the same directory, in case you want to set it on the
   new shortcut.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3 slug="win3"
   text="3. Open a MinGW shell and trigger the Windows build" %}

1. Double click your new **MinGW Shell** shortcut icon.
   This opens a command window with the correct environment set up for you, including the correct path references,
   allowing you to run any of the MinGW or MSYS applications within that command window.
2. The remainder of the steps happen within the MinGW shell:

       cd /boxes

3. Make sure curl is on your path. That normally means that you download and install it separately from its
   [website](https://curl.se/windows/). At the end of this step, you should be able to `curl -V` from within the
   MinGW shell.
4. Download and build PCRE2, and download flex and bison. This is summarily handled by

       make win32.prereq

   In case this doesn't work, you must somehow get PCRE2 to compile on your own. The goal is to have
   *pcre2-10.36/.libs/libpcre2-32.a* available.  We only need the static library for UTF-32. The version number of
   PCRE may change in the future.

4. Next,

       make win32

   If you want to create an executable with debug information, call `make clean && make win32.debug` instead.
3. boxes.exe is created in /boxes/out.
4. Run `make test` to check that your executable is working OK.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3 slug="deployment-windows"
   text="4. Deployment" %}

In order to run *boxes* on any Windows machine, two files are required:

- boxes.exe
- boxes.cfg

*boxes.cfg* is obtained by simply renaming the *boxes-config* file from the root of the boxes repo into *boxes.cfg*.

Both files should be placed together somewhere on your PATH.
