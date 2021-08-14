---
title: Editor Integration - Geany
permalink: editors-geany.html
created_at: 2021-08-15 00:10:51 +0800
last_modified_at: 2021-08-15 00:10:51 +0800
---

<a href="{{ site.baseurl }}/editors.html"><button type="button" class="btn btn-outline-primary">Back to All Editors</button></a>


# Integration with Geany

From the top menubar, go to Edit -> Format -> Send selection to -> Set custom commands and you'll get a window that looks like this:

<img src="{{ site.baseurl}}/images/editor-geany-filter1.png" class="img-fluid" alt="Command dialog" />

Click add to create a new command slot and type "boxes" under command. This will assign boxes to CTRL+ID number, in the above case, `CTRL+3`.
Now you can invoke *boxes* by selecting the text you want to modify:

<img src="{{ site.baseurl}}/images/editor-geany-filter2.png" class="img-fluid" alt="text before filtering" />

And pressing the hotkey, (`Ctrl+3` in the example) which gets you the box:

<img src="{{ site.baseurl}}/images/editor-geany-filter3.png" class="img-fluid" alt="text after filtering" />
