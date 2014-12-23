;;; boxes.el --- use boxes to comment regions

;; Copyright (C) 1999, 2001, 2006 Jason L. Shiffer

;; Author: Jason L. Shiffer <jshiffer@zerotao.com>
;; Maintainer: jshiffer@zerotao.com
;; Keywords: extensions
;; Created: 1999-10-30
;; Others: 
;;   Vijay Lakshminarayanan: support for choosing boxes comment by current buffer mode. 

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
;; http://boxes.thomasjensen.com/

;; To use this, put it somewhere in your load path and add the following
;; lines to your .emacs:
;;
;;          (autoload 'boxes-command-on-region "boxes" nil t)
;;          (autoload 'boxes-remove "boxes" nil t)
;;          (autoload 'boxes-create "boxes" nil t)
;;          (global-set-key "\C-cq" 'boxes-create)
;;          (global-set-key "\C-cr" 'boxes-remove)

;;; Code:
(eval-when-compile (require 'cl))

;;;###autoload
(defvar boxes-command "boxes"
  "The boxes command.")

(defvar boxes-types-alist
  (ignore-errors
   (with-temp-buffer
     (call-process "boxes" nil t nil "-l")
     (goto-char (point-min))
     (let ((retval nil))
       (while (re-search-forward "^\\([a-zA-Z][a-zA-Z0-9-]+\\) (.*):" nil t)
	 (add-to-list 'retval (cons (match-string 1)
				    (match-string 1))))
       retval)))
  "Association of types available to the current boxes implementation." )
(make-variable-buffer-local 'boxes-types-alist)

(defvar boxes-history (list nil))

;;;###autoload
(defvar boxes-known-modes
  '((c-mode . "c-cmt2") (c++-mode . "c-cmt2") (java-mode . "java-cmt")
    (html-mode . "html-cmt") (sh-mode . "pound-cmt") (perl-mode . "pound-cmt")
    (python-mode . "pound-cmt") (ruby-mode . "pound-cmt")
    (emacs-lisp-mode . "lisp-cmt") (lisp-mode . "lisp-cmt"))
  "The default comment type based on file names.")
(make-variable-buffer-local 'boxes-known-modes)

;;;###autoload
(defun boxes-set-default-type (mode)
  "Set the default box mode according to the buffer's major mode."
  (setq boxes-default-type (or (cdr (assoc mode boxes-known-modes)) "c-cmt2")))

;;;###autoload
(defvar boxes-default-type nil  "The default type of comment.")
(make-variable-buffer-local 'boxes-default-type)

;;;###autoload
(defvar boxes-args ""
  "Arguments to the boxes command.")
(make-variable-buffer-local 'boxes-args)
;;;###autoload
(defun boxes-create ()
  "Automagicly create a new box around the region based on the default type."
  (interactive "*")
  (boxes-command-on-region (region-beginning) (region-end) boxes-default-type))


;;;###autoload
(defun boxes-remove ()
  "Automagicly remove a new box around the region based on the default type."
  (interactive "*")
  (boxes-command-on-region (region-beginning) (region-end) boxes-default-type 1))


;;;###autoload
(defun boxes-command-on-region (start end type &optional remove)
  "Create/Remove boxes from a region.  To create just select a region and M-x boxes-command-on-region
then you will be prompted to enter a box type.  The type selection can use tab completion on the types available.
To remove a box simply prefix a 1 to the callL M-1 M-x boxes-command-on-region will remove a box from a region."
  (interactive (let ((string
		      (completing-read (format "Box type (%s): " boxes-default-type)
				       boxes-types-alist nil t nil 'boxes-history boxes-default-type)))
		(list (region-beginning) (region-end) 
		      string
		      current-prefix-arg)))
  (if type
      (setq boxes-default-type type)
    (setq boxes-default-type (boxes-set-default-type major-mode)
          type boxes-default-type))
  (let ((command-string
	 (concat boxes-command  
		 (if remove
		     (concat boxes-args " -r "))
		 (if type
		     (concat boxes-args " -d " type)))))
    (shell-command-on-region start end command-string nil 1 nil)))

(provide 'boxes)
;;; boxes.el ends here


