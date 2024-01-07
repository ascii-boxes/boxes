---
title: Downloads
permalink: /download.html
created_at: 1999-04-06
last_modified_at: 2021-02-17 21:37:44 +0100
---

# Download *Boxes*

The current release is **version {{ site.currentVersion }}**. For older revisions, feel free to browse the
[release history]({{ site.baseurl }}/releases.html).


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=2
   text="Binaries" %}

The best way to get *boxes* is via your package manager. Here are some package managers which we know have a
*boxes* package, but there are more - just try it on yours.

| Platform               | Tool  | Command                        |
|:-----------------------|:------|:-------------------------------|
| Linux (Debian, Ubuntu) | apt   | `apt-get install -y boxes`     |
| Linux (Fedora)         | dnf   | `dnf install boxes`            |
| MacOS                  | brew  | `brew install boxes`           |
| Windows                | choco | `choco install boxes.portable` |
{:.table .table-striped .w-auto}


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3 slug="windows"
   text="Windows Portable ZIP" %}

For Windows, we also have a portable ZIP which you can unpack somewhere in your `PATH`. Make sure to put boxes.exe and
boxes.cfg in the same directory. Get the ZIP file
[here](https://github.com/{{ site.github }}/releases/download/v{{ site.currentVersion }}/boxes-{{
  site.currentVersion }}-intel-win32.zip).


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3
   text="Vim Syntax File" %}

We also have a [Vim syntax file](https://raw.githubusercontent.com/{{ site.github }}/master/boxes.vim){:target="_blank"}
which enables the Vim editor to apply syntax highlighting to *boxes* config files.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=2
   text="Sources" %}

The *boxes* sources are on GitHub, including previous releases ([build instructions]({{ site.baseurl }}/build.html)):

[https://github.com/{{ site.github }}](https://github.com/{{ site.github }}){:target="_blank"}
