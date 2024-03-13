---
layout: post
title: "Windows Unicode Configuration"
date: 2024-03-13 21:35:00 +0100
---

Here's a little hint on how to configure your Windows 10/11 for boxes.<!--break-->

Out of the box, some Windows installations are not properly configured to support Unicode in their terminals.
Full UTF-8 support must sometime be explicitly enabled.

1. Press <kbd>Windows</kbd>+<kbd>R</kbd>, type `control`, and press <kbd>Enter</kbd> in order to call up the control
   panel.
2. "Clock and Region" → "Region" → *Administrative* Tab → **Change System Locale**
3. Enable the checkbox "Beta: Use Unicode UTF-8 for worldwide language support". The "Current system locale" does not
   affect this.  
   ![System Locale Dialog]({{ site.baseurl}}/images/windows10-unicode.png){: .img-shad .img-fluid}

The above is written for Windows 10, but it should be roughly the same on Windows 11.

After that, enjoy fine Unicode display even in a regular CMD shell:

![CMD shell with unicode]({{ site.baseurl}}/images/windows10-unicode-example.png){: .img-shad .img-fluid}
