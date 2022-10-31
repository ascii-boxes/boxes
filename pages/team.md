---
title: Team
permalink: /team.html
created_at: 2021-05-04 21:50:00 +0200
last_modified_at: 2021-05-04 21:50:00 +0200
---

# Team

*Boxes* was originally written in 1999 by Thomas Jensen &lt;boxes(at)thomasjensen.com&gt;.
He still maintains the software and this website today.

During its lifetime of over two decades, *boxes* has received help and support from a lot of people. This page
tries to mention as many of them as possible and to say **thank you** - you make *boxes* come alive!


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3
   text="Package Maintainers" %}

*Boxes* is available for many platforms, and we greatly appreciate the work put in by the maintainers of every
*boxes* package. Without their special knowledge of distributions, processes, and, of course, *boxes*, it would
be much harder to enjoy running *boxes* everywhere.

<table class="table table-striped table-responsive-sm">
  <thead>
    <tr>
      <th scope="col">Platform</th>
      <th scope="col">Distro</th>
      <th scope="col">Tool</th>
      <th scope="col">Maintainer</th>
      <th scope="col">Past Maintainers</th>
    </tr>
  </thead>
  <tbody>
    <tr><td>Linux</td>   <td>CRUX</td>    <td>pkgadd</td>      <td><span class="atmention"><a href="https://github.com/TimB87">@TimB87</a></span></td>     <td>&nbsp;</td></tr>
    <tr><td>Linux</td>   <td>Debian</td>  <td>apt</td>         <td><span class="atmention"><a href="https://github.com/carnil">@carnil</a></span></td>     <td>&nbsp;</td></tr>
    <tr><td>Linux</td>   <td>Fedora</td>  <td>rpm</td>         <td><span class="atmention"><a href="https://github.com/tim77">@tim77</a></span></td>       <td><span class="atmention"><a href="https://github.com/jhrozek">@jhrozek</a></span></td></tr>
    <tr><td>MacOS</td>   <td>MacOS</td>   <td>brew</td>        <td><span class="atmention"><a href="https://github.com/mathomp4">@mathomp4</a></span></td> <td><span class="atmention"><a href="https://github.com/fxcoudert">@fxcoudert</a></span></td></tr>
    <tr><td>Windows</td> <td>Windows</td> <td>choco</td>       <td><span class="atmention"><a href="https://github.com/tsjensen">@tsjensen</a></span></td> <td>&nbsp;</td></tr>
    <tr><td>Windows</td> <td>Windows</td> <td class="text-nowrap">stand-alone</td> <td><span class="atmention"><a href="https://github.com/tsjensen">@tsjensen</a></span></td> <td>Ron Aaron</td></tr>
    <tr><td>Linux</td> <td>Gentoo</td> <td>portage</td> <td><span class="atmention"><a href="https://github.com/Schievel1">@Schievel1</a></span></td><td>&nbsp;</td></tr>
  </tbody>
</table>

The above table is probably incomplete. If you know someone who maintains a *boxes* package, or even do so yourself,
send me an email, or just update the table via a pull request. Thank you!


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=2
   text="Contributors" %}

Since *boxes* is on GitHub, all contributors are automatically
[listed on GitHub](https://github.com/{{ site.github }}/graphs/contributors){:target="_blank"}.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3 slug="contributors2"
   text="Pre-2014 Contributors" %}

In the years before GitHub, *boxes* received support from additional people:

  - Ron Aaron - win32 port
  - Peter van den Berkmortel - HPUX 10 binaries of v1.0.1
  - [Salvatore Bonaccorso](https://github.com/carnil) - enable compilation on GNU/kFreeBSD and GNU/Hurd;
    fixes to the man page; maintainer of the Debian packages
  - Charles E. Campbell - vim syntax file fixes
  - Christoph Dreyer - tab unexpansion
  - [Andreas Heiduk](https://github.com/asheiduk) - bug fixes and compatibility improvements; Jed integration
  - [Jakub Hrozek](https://github.com/jhrozek) - patch to eliminate a compilation warning on Fedora; then maintainer
    of the Fedora packages
  - Yosuke Kimura - DEC alpha/OSF binaries of v1.0.1
  - Jeff Lanzarotta - contribution to FAQ
  - Elmar Loos - 'mend' option
  - Zdenek Sekera - SGI/Irix6 binaries of v1.0.1
  - [Jason L. Shiffer](https://github.com/zerotao) - Emacs integration
  - Henry Spencer - kindly permitted the use of his regular expression library for *boxes*
  - Lu Weifeng - OpenWRT port - *boxes* now runs on routers, too
  - Tommy Williams - i386/FreeBSD port
  - Joe Zbiciak - compatibility improvements and Solaris binaries
  - [François-Xavier Coudert](https://github.com/fxcoudert) - 64bit enablement for MacOS

  - AlpT - the *c-cmt3* design
  - Ted Berg - the *javadoc* design
  - Neil Bird - the *ada-cmt* and *ada-box* designs
  - Bas van Gils - the *cc* design
  - Karl E. Jorgensen - the *ian_jones* design
  - Vijay Lakshminarayanan - the *lisp-cmt* design
  - Elmar Loos - the *ccel* and *underline* designs
  - Christian Molls - the *boxquote* design
  - Fredrik Steen - the *stone* design
  - Michael Tiernan - the *caml* design

Many thanks to these awesome folks!
