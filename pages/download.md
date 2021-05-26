---
title: Downloads
permalink: /download.html
created_at: 1999-04-06
last_modified_at: 2021-02-17 21:37:44 +0100
---

# Download *Boxes*

The current release is **version {{ site.currentVersion }}**. For older revisions, feel free to browse the
[release history]({{ site.baseurl }}/releases.html).


## Binaries

*boxes* binaries are available on multiple platforms:
{% comment %} TODO properly reference known packages {% endcomment %}

**Windows**

- Option 1 - Using [Chocolatey](https://chocolatey.org/): Just type `choco install boxes.portable`
- Option 2 - Install from ZIP:
  [Download ZIP](https://github.com/{{ site.github }}/releases/download/v{{ site.currentVersion }}/boxes-{{
  site.currentVersion }}-intel-win32.zip), extract and put it somewhere on the PATH. Make sure to put boxes.exe and
  boxes.cfg in the same directory.

**MacOS X**

- Just type `brew install boxes`.\\
  This works for both 32bit and 64bit MacOS.

**Linux**

- Just type `apt-get install boxes`.\\
  This works for Debian and debian-based Linuxes, such as Ubuntu, including the Windows 10 Ubuntu Subsystem.


## Miscellaneous

  - The current [list of available box designs]({{ site.baseurl }}/box-designs.html)
  - A [Vim syntax file](https://raw.githubusercontent.com/{{ site.github }}/master/boxes.vim){:target="_blank"} to
    enable the Vim editor to apply syntax highlighting to *boxes* config files


## Sources

The *boxes* sources are now all on GitHub, including previous releases
([build instructions]({{ site.baseurl }}/build.html)):

[https://github.com/{{ site.github }}](https://github.com/{{ site.github }}){:target="_blank"}

