# Building from Source

Here's how to build your own version of *boxes* using the [boxes sources](https://github.com/{{ site.github }}).

## Building on Linux / UNIX

Just unzip the [source archive](https://github.com/{{ site.github }}/releases/latest) or [clone the GitHub repo](https://help.github.com/articles/cloning-a-repository/). Make sure you have your C&nbsp;compiler installed, including [Flex](http://flex.sourceforge.net/) and [Bison](http://www.gnu.org/software/bison/). In the top level directory, type `make && make test`. Find the resulting binary in the *src* folder. In case of problems, check the [compilation faq]({{ site.baseurl }}/docs/faq.html#q5).

That should be all. *Boxes* is built so that this works almost everywhere.

## Building on Windows

Boxes was written for UNIX, but it can also run on Windows! Boxes is a 32bit application, but it works on 64bit systems, too. Here's how to build on Windows.

> Special thanks go to Ron Aaron, who provided a specially crafted Makefile for win32 and also created the Windows versions of boxes that have been around to this day.

In order to build *boxes* on Windows, the required win32 executable can be created like this:

1. Install [MinGW32](http://www.mingw.org/)
2. Configure MinGW32
3. Open a MinGW shell and run `make win32`

Basically very simple, but there may be a few pitfalls, so we'll go through each step in detail.

### 1. Install MinGW32

1. Download MinGW from [mingw.org](http://www.mingw.org/) by pressing the **Download Installer** button in the top right corner of the page. You'll receive a mingw-get-setup.exe.
2. Right-click mingw-get-setup.exe and choose **Run as Administrator** from the pop-up menu.
   This brings up the Installation Preferences.
3. Choose `C:\MinGW` as installation directory. This is really important. If for some reason you cannot use this highly recommended directory, choose another one that has a short path, does not contain spaces (!), and does not include a Windows "special path". For example, `C:\MinGW_2`.
4. Select "install support for GUI" (yes, even though *boxes* has no GUI) and install "for all users".
5. Press **Continue** and wait for download of the *Installation Manager*.
6. After the download of the Installation Manager is completed, press **Continue** again.
   The Installation Manager opens.
7. Choose **mingw32-base** and **msys-base**. Also, further down, under *MSYS / MinGW Developer Toolkit*, choose **msys-bison** and **msys-flex**.
   **Do not** install <tt>mingw-developer-toolkit</tt>, because if on 64bit, this requires tweaking of several environment variables to get gcc to use 32bit libs all around (which may be hard to get right for laymen).
8. In the menu, select *Installation* &rarr; *Apply Changes*.
   Something like 104 Packages will be installed, which may take a few minutes.
9. Upon success, select *Installation* &rarr; *Quit* from the menu.

### 2. Configure MinGW32

1. Change into C:\MinGW\msys\1.0\etc.
2. Copy the file *fstab.sample* to a new file called simply *fstab*.
3. Open *fstab* and make sure that the only lines which are not comments are these:

       c:/mingw		/mingw
       d:/path/to/boxes	/boxes

   Note that the whitespace in this example are tab characters. The d:/path/to/boxes is the path where you placed your clone of the boxes repo. Avoid spaces in its path, too.
4. Add C:\MinGW\bin to your system PATH.
5. Create a "MinGW Shell" shortcut somewhere (I chose my desktop). The shortcut invokes the C:\MinGW\msys\1.0\msys.bat script, which is installed as a component of MSYS; (if you installed to an alternative directory, you should adjust the C:\MinGW prefix accordingly).
   An icon file is provided in the same directory, in case you want to set it on the new shortcut.

### 3. Open a MinGW shell and trigger the Windows build

1. Double click your new **MinGW Shell** shortcut icon.
   This opens a command window with the correct environment set up for you, including the correct path references, allowing you to run any of the MinGW or MSYS applications within that command window.
2. Within the MinGW shell:

       cd /boxes
       make clean && make win32

   If you want to create an executable with debug information, call `make clean && make debug.win32` instead.
3. boxes.exe is created in /boxes/src.
4. Optionally, run `make test` to check that your executable is working OK.

### 4. Deployment

In order to run boxes on any Windows machine, two files are required:

- boxes.exe
- boxes.cfg

*boxes.cfg* is obtained by simply renaming the *boxes-config* file from the root of the boxes repo into *boxes.cfg*.

Both files should be placed together somewhere on your PATH.
