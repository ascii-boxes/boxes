---
title: Docs - Config Intro
permalink: /config-general.html
redirect_from:
  - /docs/config-general.html
  - /docs/config-general.shtml
created_at: 1999-04-06
last_modified_at: 2014-12-23 22:50:09 +0100
---

# Box Designs

*Boxes* can draw and remove the boxes whose designs are defined in its configuration file. This page describes the
concept of a *box design*, clarifies some terms used to describe a box design, and how to define your own box designs.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=2
   text="General Box Layout" %}

A box design consists of up to 16 so-called **shapes** which are labeled in a fashion similar to the points of the
compass:

    +---------+---------+-----------------+---------+---------+
    |         |         |                 |         |         |
    |   NW    |   NNW   |        N        |   NNE   |   NE    |
    |         |         |                 |         |         |
    +---------+---------+-----------------+---------+---------+
    |         |               padding               |         |
    |   WNW   |  +-- - - - - - - - - - - - - - --+  |   ENE   |
    |         |  |                               |  |         |
    +---------+                                     +---------+
    |         |  |                               |  |         |
    |         |               original              |         |
    |    W    |  |            input              |  |    E    |
    |         |               text                  |         |
    |         |  |                               |  |         |
    +---------+                                     +---------+
    |         |  |                               |  |         |
    |   WSW   |  +-- - - - - - - - - - - - - - --+  |   ESE   |
    |         |               padding               |         |
    +---------+---------+-----------------+---------+---------+
    |         |         |                 |         |         |
    |   SW    |   SSW   |        S        |   SSE   |   SE    |
    |         |         |                 |         |         |
    +---------+---------+-----------------+---------+---------+

The sides of the box are sometimes referred to as *top* (instead of north), *right* (instead of east), *bottom*
(south), and *left* (west), in order to avoid confusion with *shapes* of the same name. The group of shapes NW, NE, SE,
and SW is referred to as the *corner shapes*. The groups of shapes between two corner shapes are called the
*side shapes*.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=2
   text="Shape Restrictions" %}

A design definition does not need to include specifications for all 16 shapes. You only need to specify the shapes that
you need. At least one shape must be specified.

  - All shapes on a vertical side (inluding corners) must be of equal width.
  - All shapes on a horizontal side (including corners) must be of equal height.

In order to let the box have a dynamic size, certain side shapes must be drawn repeatedly. Those shapes are called
**elastic** shapes.

  - At least one shape per side must be elastic.
  - Corners may not be elastic.
  - No two neighboring shapes may be elastic, i.e. there must always be at least one static shape in between two
    elastic shapes.

You don't really need to worry about these restrictions. *Boxes* will tell you if you violate any of them. So if
*boxes* does not complain, your design is (at least syntactically) great.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=2
   text="Open Box Sides" %}

Many box designs require open box sides, for instance most regional comments. Thus, if you define a box side to consist
entirely of spaces, that side will be considered **open** and it will not appear in the output.

Of course, you don't need to actually put the empty shapes into the config file, but *boxes* will automagically
generate empty shapes if they are needed. (Internally, boxes always need at least 8 shapes - the corners, and one shape
per side.) When the box is generated, those empty sides will be left out. In this way, boxes can be created which are
open on any side.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=2
   text="Padding Area" %}

The last thing to mention about a box design is the **padding area**, which surrounds the original input text as shown
in the figure above. The padding area is located inside the actual box and consists only of spaces. It is frequently
used to keep the text from coming too close to the box unless absolutely necessary. &quot;Absolutely necessary&quot;
usually means that the user has specified a very small box size.

It is also possible to control the padding from the command line.


Read on in the next part: [Configuration File Syntax]({{ site.baseurl }}/config-syntax.html)
