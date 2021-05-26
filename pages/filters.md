---
title: What are filters?
permalink: /filters.html
redirect_from:
  - /docs/filters.html
  - /docs/filters.shtml
created_at: 1999-04-06
last_modified_at: 1999-04-06
---

# What are filters?

[Filters](http://en.wikipedia.org/wiki/Filter_%28software%29){:target="_blank"} are generally programs which read some
input text from standard input, perform some modifications on it, and write the modified text to standard output.
While a single filter can be used individually, they are frequently strung together to form a pipeline. *Boxes* is
such a filter program.

Some text editors *support filters*, which is to say they offer a way to let filter programs make changes to the text
being edited. By pressing a key, the user tells the editor to start the filter program (e.g. *boxes*) and feed it the
lines of text selected in the editor. Those lines are subsequently replaced with the output from the filter program.

##### Here's an example using *boxes*:

Imagine your editor (in this case, it's [Vim](http://www.vim.org/){:target="_blank"}) shows the following C code
(it could really be any text):

<img src="{{ site.baseurl}}/images/filter1.gif" class="img-fluid" width="619" height="272"
     alt="text before filtering" />

Now you mark some of the lines ...

<img src="{{ site.baseurl}}/images/filter2.gif" class="img-fluid" width="619" height="272"
     alt="text to be filtered is marked" />

... and press the key which makes vim call the filter program *boxes*:

<img src="{{ site.baseurl}}/images/filter3.gif" class="img-fluid" width="619" height="272"
     alt="after the filter was applied" />

The marked lines have been fed to *boxes* and replaced by the output of *boxes*.
The same mechanism works for box removal - or anything else you can find a filter for.

Please see the [editor integration section]({{ site.baseurl }}/editors.html) in the *boxes* docs for more info on how
to make your editor work with *boxes*.
