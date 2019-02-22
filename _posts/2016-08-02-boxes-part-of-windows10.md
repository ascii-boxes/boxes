---
layout: post
title: Boxes becomes part of Windows 10
date: 2016-08-02 13:24:00 +0100
comments: false
---

<i>Boxes</i> is now part of Windows 10! This sounds amazing, and it is - albeit, to be honest,
in an indirect way. Since this year's "anniversary update", Windows 10 features an optional
<a href="https://msdn.microsoft.com/commandline/wsl/about" target="_blank">Ubuntu Subsystem</a>, which supports
<i>boxes</i> natively. This is mostly thanks to
<span class="atmention"><a href="https://github.com/carnil">@carnil</a></span>, who maintains the
Debian packages; and Debian being the foundation of Ubuntu. So, like I said, it is an indirect way, but still, we can't
help being a little proud of the fact! <!--break-->

<p>The Ubuntu Subsystem is optional and thus needs to be activated manually. Microsoft provides a reasonably simple
<a href="https://docs.microsoft.com/en-us/windows/wsl/install-win10"
target="_blank">guide</a> on how to do that. After starting a bash in the Ubuntu Subsystem,
you can activate <i>boxes</i> by typing:</p>

<p><pre style="background-color:white; border:0;">
<b>tsjensen@FRACTAL:~$ sudo su -</b>
[sudo] password for tsjensen:
<b>root@FRACTAL:~# apt-get install boxes</b>
Reading package lists... Done
Building dependency tree
Reading state information... Done
The following NEW packages will be installed:
  boxes
Selecting previously unselected package boxes.
Preparing to unpack .../boxes_1.1.1-3_amd64.deb ...
Unpacking boxes (1.1.1-3) ...
Processing triggers for man-db (2.6.7.1-1ubuntu1) ...
Setting up boxes (1.1.1-3) ...
<b>root@FRACTAL:~#</b></pre></p>

<p>Now, all boxes features are present:</p>
<p><img src="/images/windows10-ubuntu-1.png"/></p>

<p>Even the manual page is available:</p>
<p><img src="/images/windows10-ubuntu-2.png"/></p>
