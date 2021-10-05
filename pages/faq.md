---
title: FAQ
permalink: /faq.html
redirect_from:
  - /docs/faq.html
  - /docs/faq.shtml
created_at: 1999-04-06
last_modified_at: 2021-10-05 21:44:00 +0200
---

# FAQ

These are the frequently asked questions about the *boxes* program and their answers.

These questions are *actually* "frequently asked". For general information on the *boxes* program, installation
instructions, and information on box design creation please refer to the
[*boxes* documentation]({{ site.baseurl }}/).


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3 slug="q1"
   text="Q. 1. What is a text filter program?" %}

There is a [separate page]({{ site.baseurl }}/filters.html) explaining this. *Boxes* is mostly used as such a
filter program. 


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3 slug="q2"
   text="Q. 2. Error message \"input in flex scanner failed\"" %}

Upgrade to version 1.0.1 or later. Versions prior to 1.0.1 gave this error message when the config file they were
trying to read was in fact a directory. The global config file name is */usr/share/boxes* on most systems. This is the
name of the file, not the name of a directory into which a config file would be placed.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3 slug="q3"
   text="Q. 3. Boxes destroys my tabs!" %}

By default, all tab characters are replaced by spaces. However, you can change this behavior using the `-t` option
(since version 1.1). The `-t` option only affects leading tabs. Tabs which end up inside the box are *always* converted
into spaces.  
Note that you can also set the tab stop distance (== how many spaces per tab) using the `-t` option.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3 slug="q4"
   text="Q. 4. \"Can't read file C:\TEMP\VIO44.TMP\" when calling boxes from vim" %}

On Windows, this error message may appear instead of a box when *boxes* is called from vim. This is not a problem of
*boxes*. In fact, it's a misleading message from the vim editor which is supposed to tell you that *boxes* is not in
your PATH. Solution: Copy *boxes.exe* and *boxes.cfg* to a directory which is in your PATH. (thanks *Jeff Lanzarotta*,
05-Jul-00)


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3 slug="q5"
   text="Q. 5. Compilation" %}

Detailed information on how to build *boxes* from source is collected on the
[build page]({{ site.baseurl }}/build.html).

In addition to that, the following issues have occurred:

- Custom library locations\\
  If you have manually provided some of the libraries that *boxes* uses for building, such as pcre2 or libunistring,
  you may need to tell our build where to find them. For this, use the `CFLAGS_ADDTL` and `LDFLAGS_ADDTL`
  environment variables. They can be used to add options to the compiler and linker calls. For example:

      make CFLAGS_ADDTL=-I/Users/USER/local/include LDFLAGS_ADDTL=-L/Users/USER/local/lib

- `Bad address` on *boxes* execution after compiling on a 64bit system:
  This may happen when the system you are compiling on is 64bit. Boxes is only a 32bit program, so the compiler may
  have to be forced to 32bit by adding the `-m32` option. (Thanks to
  <span class="atmention">[@stefanow](https://github.com/stefanow)</span> for
  [supplying](https://github.com/{{ site.github }}/issues/7){:target="_blank"} this information!)
  In order to do this, use the following command line (works with current sources):

      make CFLAGS_ADDTL=-m32 LDFLAGS_ADDTL=-m32

- Compilation on SLES:\\
  On SLES, you might need to add `-std=c99` as observed by
  <span class="atmention">[@mathomp4](https://github.com/mathomp4)</span> in
  [#74](https://github.com/ascii-boxes/boxes/issues/74#issuecomment-784371446).


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3 slug="q6"
   text="Q. 6. Character Encoding" %}

Since v2.0.0, *boxes* supports different character encodings for input/output text. The config file, however, is
still ASCII (we are working on that). *boxes* normally picks up your system encoding, which on most systems, is simply
UTF-8. You can override this behavior with `-n`. *boxes* shows what it thinks is the system encoding when you call
`boxes -h` - the displayed default value for `-n` is the system encoding.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3 slug="q7"
   text="Q. 7. How to align a box within a terminal window?" %}

The *boxes* options only allow aligning the text within a box, but it is actually possible to use *boxes* to align the
entire box within a terminal window. At least on Unix / Linux ... we don't currently know how to do this on Windows.

This method assumes that `tput cols` yields the width of the terminal window. If it doesn't, you'd have to find another
way to get this information, for example a `$COLUMNS` environment variable.

Centered:

```shell
echo -e foo\\ncenter | boxes -d parchment -p h5 | boxes -s $(tput cols) -c x -a hc -i none | cut -c 2-
```

Right-aligned:

```shell
echo -e foo\\nright-aligned | boxes -d parchment -p h5 | boxes -s $(tput cols) -c x -a hr -i none | sed -e 's/^x/ /'
```

Note that we `-a hc` instead of just `-ac`, because we want to align the whole block of text, not individual lines.  
The result looks like this:

<img src="{{ site.baseurl}}/images/faq-alignment.png" class="img-fluid" width="1400" height="477"
     alt="box aligned in terminal window" />
