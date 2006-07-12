;;; boxes.el --- use the boxes to comment regions

;; Copyright (C) 1999 Jason L. Shiffer

;; Author: Jason L. Shiffer <jshiffer@zerotao.com>
;; Maintainer: jshiffer@zerotao.com
;; Keywords: extensions
;; Created: 1999-10-30

;; $Id: boxes.el,v 1.0 1999/10/30 02:45:06 jshiffer Exp $

;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 2, or (at your option)
;; any later version.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with this program; if not, you can either send email to this
;; program's maintainer or write to: The Free Software Foundation,
;; Inc.; 675 Massachusetts Avenue; Cambridge, MA 02139, USA.

;;; Commentary:

;; This program provides an interface to the boxes application which can be found at
;; http://home.pages.de/~jensen/boxes/

;; To use this, put it somewhere in your load path and add the following
;; lines to your .emacs:
;;
;;          (autoload 'boxes-command-on-region "boxes" nil t)
;;          (autoload 'boxes-remove "boxes" nil t)
;;          (autoload 'boxes-create "boxes" nil t)
;;          (global-set-key "\C-cq" 'boxes-create)
;;          (global-set-key "\C-cr" 'boxes-remove)

;;; Code:

;;;###autoload
(defvar boxes-command "boxes"
  "The boxes command.")

;;;###autoload
(defvar boxes-types-alist '(("c" . "c")
			  ("parchment" . "parchment")
			  ("columns" . "columns")
			  ("whirly" . "whirly")
			  ("scroll" . "scroll")
			  ("simple" . "simple")
			  ("c-cmt" . "c-cmt")
			  ("c-cmt2" . "c-cmt2")
			  ("html" . "html")
			  ("shell" . "shell")
			  ("nuke" . "nuke")
			  ("diamonds" . "diamonds")
			  ("mouse" . "mouse")
			  ("sunset" . "sunset")
			  ("boy" . "boy")
			  ("girl" . "girl")
			  ("tjc" . "tjc")
			  ("netdata" . "netdata")
			  ("xes" . "xes")
			  ("dog" . "dog")
			  ("cat" . "cat")
			  ("capgirl" . "capgirl")
			  ("santa" . "santa")
			  ("spring" . "spring")
			  ("stark2" . "stark2")
			  ("stark1" . "stark1")
			  ("peek" . "peek")
			  ("java-cmt" . "java-cmt")
			  ("pound-cmt" . "pound-cmt")
			  ("html-cmt" . "html-cmt")
			  ("vim-cmt" . "vim-cmt")
			  ("right" . "right")
			  ("headline" . "headline"))
  "Association of types available to the current boxes implementation.")
(make-variable-buffer-local 'boxes-types-alist)

(defvar boxes-history (list nil))

;;;###autoload
(defvar boxes-default-type "c-cmt2"
  "The default type of comment.")
(make-variable-buffer-local 'boxes-default-type)

;;;###autoload
(defvar boxes-args ""
  "Arguments to the boxes command.")
(make-variable-buffer-local 'boxes-args)

;;;###autoload
(defun boxes-create ()
  (interactive)
  (boxes-command-on-region (region-beginning) (region-end) boxes-default-type)
  "Automagicly create a new box around the region based on the default type.")

;;;###autoload
(defun boxes-remove ()
  (interactive)
  (boxes-command-on-region (region-beginning) (region-end) boxes-default-type 1)
  "Automagicly remove a new box around the region based on the default type.")

;;;###autoload
(defun boxes-command-on-region (start end type &optional remove)
  (interactive (let ((string
		      (completing-read (format "Box type (%s): " boxes-default-type)
				       boxes-types-alist nil t nil 'boxes-history boxes-default-type)))
		(list (region-beginning) (region-end) 
		      string
		      current-prefix-arg)))
  (if type
      (setq boxes-default-type type))
  (let ((command-string
	 (concat boxes-command  
		 (if remove
		     (concat boxes-args " -r "))
		 (if type
		     (concat boxes-args " -d " type)))))
    (shell-command-on-region start end command-string nil 1 nil)))

(provide 'boxes)
;;; boxes.el ends here
