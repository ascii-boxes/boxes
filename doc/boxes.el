;;; boxes.el --- use boxes to comment regions

;; Copyright (C) 1999, 2001, 2006 Jason L. Shiffer

;; Author: Jason L. Shiffer <jshiffer@zerotao.com>
;; Maintainer: jshiffer@zerotao.com
;; Keywords: extensions
;; Created: 1999-10-30
;; Others:
;;   Vijay Lakshminarayanan: support for choosing boxes comment by current buffer mode.

;; boxes - Command line filter to draw/remove ASCII boxes around text
;; Copyright (c) 1999-2021 Thomas Jensen and the boxes contributors
;;
;; This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
;; License, version 3, as published by the Free Software Foundation.
;; This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
;; warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
;; details.
;; You should have received a copy of the GNU General Public License along with this program.
;; If not, see <https://www.gnu.org/licenses/>.

;;; Commentary:

;; This program provides an interface to the boxes application which can be found at
;; https://boxes.thomasjensen.com/

;; To use this, put it somewhere in your load path and add the following
;; lines to your .emacs:
;;
;;          (autoload 'boxes-command-on-region "boxes" nil t)
;;          (autoload 'boxes-remove "boxes" nil t)
;;          (autoload 'boxes-create "boxes" nil t)
;;          (global-set-key "\C-cq" 'boxes-create)
;;          (global-set-key "\C-cr" 'boxes-remove)

;;; Code:

(defgroup boxes nil
  "ASCII boxes unlimited!"
  :group 'convenience)

(defcustom boxes-command "boxes"
  "The boxes command."
  :type 'string
  :group 'boxes)

(defcustom boxes-args ""
  "Additional arguments to the boxes command."
  :type 'string
  :group 'boxes)

(defcustom boxes-known-modes
  '((c-mode . "c-cmt2") (c++-mode . "c-cmt2") (java-mode . "java-cmt")
    (html-mode . "html-cmt") (sh-mode . "pound-cmt") (perl-mode . "pound-cmt")
    (python-mode . "pound-cmt") (ruby-mode . "pound-cmt")
    (emacs-lisp-mode . "lisp-cmt") (lisp-mode . "lisp-cmt"))
  "Default box type based on the major mode of the buffer."
  :type '(alist :key-type symbol :value-type string)
  :group 'boxes)

(defconst boxes-types-list
  (let ((types (process-lines boxes-command "-q" "(all)")))
    (mapcar (lambda(type) (replace-regexp-in-string " *\(alias\) *$" "" type)) types))
  "List of types available to the current boxes implementation.")

(defvar boxes-history nil
  "Boxes types history.")

(defvar-local boxes-default-type nil
  "The default type of box.")

(defun boxes-default-type (mode)
  "Get the default box type for the given buffer major MODE."
  (or (cdr (assoc mode boxes-known-modes)) "c-cmt2"))

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
  "Create/Remove boxes from a region.
To create just select a region and M-x boxes-command-on-region then you will be prompted to enter a box type.
The type selection can use tab completion on the types available.
To remove a box simply prefix a 1 to the call: M-1 M-x boxes-command-on-region will remove a box from a region."
  (interactive (let ((string
		      (completing-read (format "Box type (%s): " boxes-default-type)
				       boxes-types-list nil t nil 'boxes-history boxes-default-type)))
		(list (region-beginning) (region-end)
		      string
		      current-prefix-arg)))
  (when (or (null type) (string= type ""))
    (setq type (boxes-default-type major-mode)))
  (setq boxes-default-type type)
  (let ((command-string
	 (concat boxes-command
		 (if remove
		     (concat boxes-args " -r "))
		 (if type
		     (concat boxes-args " -d " type)))))
    (shell-command-on-region start end command-string nil 1)))

(provide 'boxes)
;;; boxes.el ends here
