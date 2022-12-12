---
title: Editor Integration - Emacs
permalink: editors-emacs.html
created_at: 1999-04-06
last_modified_at: 2022-12-12 17:34:00 +0000
---

<a href="{{ site.baseurl }}/editors.html"><button type="button" class="btn btn-outline-primary">Back to All Editors</button></a>


# Integration with Emacs

[![MELPA](https://melpa.org/packages/boxes-badge.svg)](https://melpa.org/#/boxes)

The easiest way is to install the `boxes` package from [MELPA](https://melpa.org/#/boxes), however it can also be installed manually if desired:

-   Installation from MELPA
    1.  Make sure [MELPA is in your package archives list](https://melpa.org/#/getting-started).
    
	2.  `M-x package-install boxes`

    3.  Add key bindings for the `boxes` commands to your startup file:

        ```emacs-lisp
         (global-set-key "\C-cb" 'boxes-command-on-region)
         (global-set-key "\C-cq" 'boxes-create)
         (global-set-key "\C-cr" 'boxes-remove)
        ```
		
    4.  Alternatively, if `use-package` is installed you can automatically install the `boxes` binary & elisp packages and
        add the key bindings with the following form, just like magic 😊

        ```emacs-lisp
         (use-package boxes
           :ensure t
           :ensure-system-package boxes
           :bind (("C-c b" . boxes-command-on-region)
                  ("C-c q" . boxes-create)
                  ("C-c r" . boxes-remove)))
        ```
		
-   Manual Installation
    1.  [Download](https://github.com/{{ site.github }}/blob/master/doc/boxes.el) `boxes.el` to a directory and add it
        to your `load-path`:

        ```emacs-lisp
         (add-to-list 'load-path <install directory>)
        ```
		
    2.  Add autoloads to your startup file so `boxes` will be loaded on first use:
	
        ```emacs-lisp
		 (autoload 'boxes-command-on-region "boxes" nil t)
         (autoload 'boxes-remove "boxes" nil t)
         (autoload 'boxes-create "boxes" nil t)
		 ```

    3.  Add key bindings for the `boxes` commands to your startup file:

		```emacs-lisp
         (global-set-key "\C-cb" 'boxes-command-on-region)
         (global-set-key "\C-cq" 'boxes-create)
         (global-set-key "\C-cr" 'boxes-remove)
        ```
	
