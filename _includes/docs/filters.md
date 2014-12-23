# What are filters?

[Filters](http://en.wikipedia.org/wiki/Filter_%28software%29) are generally programs which read some input text from standard input, perform some modifications on it, and write the modified text to standard output. While a single filter can be used individually, they are frequently strung together to form a pipeline. *Boxes* is such a filter program.

Some text editors *support filters*, which is to say they offer a way to let filter programs make changes to the text being edited. By pressing a key, the user tells the editor to start the filter program (e.g. *boxes*) and feed it the lines of text selected in the editor. Those lines are subsequently replaced with the output from the filter program.

**Here's an example using *boxes*:**

Imagine your editor (in this case, it's [Vim](http://www.vim.org/)) shows the following C code (it could be any text, of course):

<IMG SRC="{{ site.baseurl}}/images/filter1.gif" WIDTH="619" HEIGHT="272" ALT="text before filtering" />

Now you mark some of the lines ...

<IMG SRC="{{ site.baseurl}}/images/filter2.gif" WIDTH="619" HEIGHT="272" ALT="text to be filtered is marked" />

... and press the key which makes vim call the filter program *boxes*:

<IMG SRC="{{ site.baseurl}}/images/filter3.gif" WIDTH="619" HEIGHT="272" ALT="after the filter was applied" />

Voil&aacute;! The marked lines have been fed to *boxes* and replaced by the output of *boxes*. You've got yourself a box! Of course the same mechanism works for box removal - or anything else you can find a filter for. :smile:

Most major text editors support filters, among them [Vim](http://www.vim.org/), [Emacs](http://www.emacs.org/), and [Jed](http://www.jedsoft.org/jed/). There are certainly many, many more.

Please see the [installation section](install.html) in the *boxes* docs for more info on how to make your editor work with *boxes*.
