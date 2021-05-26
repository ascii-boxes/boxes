---
title: Editor Integration - Emacs
permalink: editors-emacs.html
created_at: 1999-04-06
last_modified_at: 2014-12-23 22:50:00 +0100
---

<a href="{{ site.baseurl }}/editors.html"><button type="button" class="btn btn-outline-primary">Back to All Editors</button></a>


# Integration with Emacs

<span class="atmention">[@zerotao](https://github.com/zerotao)</span> kindly provided the following information on
integrating *boxes* with Emacs:

The simple interface (only a single box style, but easy):

```lisp
(defun boxes-create ()
    (interactive)
    (shell-command-on-region (region-beginning) (region-end) "boxes -d c-cmt2" nil 1 nil))

(defun boxes-remove ()
    (interactive)
    (shell-command-on-region (region-beginning) (region-end) "boxes -r -d c-cmt2" nil 1 nil))
```

<span class="atmention">[@zerotao](https://github.com/zerotao)</span> also wrote a
[*boxes* mode for Emacs](https://github.com/{{ site.github }}/blob/master/doc/boxes.el). Remember to update its design
list when you add new designs to your config file.
