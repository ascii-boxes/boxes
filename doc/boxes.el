;;; boxes.el --- ASCII boxes unlimited! -*- lexical-binding: t -*-

;; Copyright (C) 1999, 2001, 2006 Jason L. Shiffer

;; Author: Jason L. Shiffer <jshiffer@zerotao.com>
;; Maintainer: Jason L. Shiffer <jshiffer@zerotao.com>
;; Version: 0.0
;; Package-Requires: ((emacs "24.3"))
;; Keywords: extensions
;; URL: https://boxes.thomasjensen.com
;; Created: 1999-10-30
;; Others:
;;   Vijay Lakshminarayanan: support for choosing boxes comment by current buffer mode.
;;   Mike Woolley: Customise and packaging support.

;; boxes - Command line filter to draw/remove ASCII boxes around text
;; Copyright (c) 1999-2024 Thomas Jensen and the boxes contributors
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

;; To use this, add the following lines to your startup file:
;;
;;          (global-set-key "\C-cb" 'boxes-command-on-region)
;;          (global-set-key "\C-cq" 'boxes-create)
;;          (global-set-key "\C-cr" 'boxes-remove)
;;
;; If you didn't install the package version then you will also need to put this file somewhere in your load path and
;; add these lines to your startup file:
;;
;;          (autoload 'boxes-command-on-region "boxes" nil t)
;;          (autoload 'boxes-remove "boxes" nil t)
;;          (autoload 'boxes-create "boxes" nil t)
;;
;; If `use-package' is installed you can automatically install the boxes binary and elisp package with:
;;
;;          (use-package boxes
;;            :ensure t
;;            :ensure-system-package boxes
;;            :bind (("C-c b" . boxes-command-on-region)
;;                   ("C-c q" . boxes-create)
;;                   ("C-c r" . boxes-remove)))

;;; Code:

(defgroup boxes nil
  "ASCII boxes unlimited!"
  :prefix "boxes-"
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

(defconst boxes-minimum-version "2.1.0"
  "Minimum required version of `boxes' to support querying the available box types.")

(defvar boxes-history nil
  "Boxes types history.")

(defvar-local boxes-default-type nil
  "The default type of box.")

(defvar boxes-types-list nil
  "List of types available to the current boxes implementation, nil if not set yet.")

(defun boxes-types ()
  "Return the list of types available to the current boxes implementation.
Signal error if a supported version of `boxes' is not available."
  (condition-case nil
      (or boxes-types-list
          (setq boxes-types-list
                (let ((types (process-lines boxes-command "-q" "(all)")))
                  (mapcar (lambda(type) (replace-regexp-in-string " *\(alias\) *$" "" type))
                          types))))
    (error (error "Please install Boxes %s or later" boxes-minimum-version))))

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
  "Create or Remove boxes from a region.

To create a box select a region, hit \\[boxes-command-on-region]
& enter a box type.  Box type selection uses tab completion on
the supported types.

To remove a box simply prefix a 1 to the call, eg M-1
\\[boxes-command-on-region] will remove a box from a region.

Note that interactive use requires `boxes' >= 2.1.0 to support
querying the supported types.

When calling from Lisp, supply the region START & END and the box
TYPE to create a box.  Specifying a non-nil value for REMOVE,
removes the box."
  (interactive (let ((string
		      (completing-read (format "Box type (%s): " boxes-default-type)
				       (boxes-types) nil t nil 'boxes-history boxes-default-type)))
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
