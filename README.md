[![Build Status](https://travis-ci.org/ascii-boxes/boxes.svg?branch=master)](https://travis-ci.org/ascii-boxes/boxes)

# boxes

**Command line ASCII boxes unlimited!**

[Boxes](https://boxes.thomasjensen.com/) is a command line
[filter program](https://boxes.thomasjensen.com/docs/filters.html) that draws ASCII art boxes
around your input text ([examples](https://boxes.thomasjensen.com/examples.html)).

For more information, please visit [the website](https://boxes.thomasjensen.com/).

## Status

*Boxes* is **stable**.

It has been around since 1999, and has since been included in several
Linux distributions and, lately, even
[Windows 10](https://boxes.thomasjensen.com/2016/08/boxes-part-of-windows10), via its Ubuntu subsystem.
Its source code is quite portable, and so, *boxes* has been made to run on a wide range of
platforms, from Windows PCs to Netgear routers.

*Boxes* has matured, and is thusly no longer being actively developed. But contributors can be sure
that their pull requests are promptly reviewed and merged into the main code base.

## License

Boxes is free software under the GNU General Public License, version 2
([GPLv2](http://www.gnu.org/licenses/old-licenses/gpl-2.0.html)).

## Development

The website has information on [building from source](https://boxes.thomasjensen.com/build.html),
[testing](https://boxes.thomasjensen.com/testing.html), and
[contributing](https://boxes.thomasjensen.com/contributing.html) to the source code.<br/>
*Boxes* has received support from many [contributors](https://boxes.thomasjensen.com/contributing.html#contributors)
over the years, and continues to benefit greatly from contributions. Many thanks for that!

## Running it

Try if boxes works by typing `echo foo | boxes` on the command line.
It should give you something like that:

    /*******/
    /* foo */
    /*******/

Type `boxes -l` to get a long list of box designs to use. Detailed usage information is in the
manual page and on the website. Consider tying
*boxes* to your editor, so that it can be invoked by a simple keypress.

## Custom box designs

You can define [your own box designs](https://boxes.thomasjensen.com/docs/)!

We appreciate the creative new box designs that you craft for *boxes*.
Please feel free to submit them in the form of pull requests on
[boxes-config](https://github.com/ascii-boxes/boxes/blob/master/boxes-config),
and they may make it into the next release!

```
     _________________________
    /\                        \
    \_|    Thank you for      |
      |    using Boxes!       |
      |  _____________________|_
      \_/______________________/
```
