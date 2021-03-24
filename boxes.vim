" boxes - command line ASCII boxes unlimited!
" Copyright (C) 1999-2020 Thomas Jensen and the boxes contributors
"
" This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public
" License, version 2, as published by the Free Software Foundation.
" This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
" warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
" You should have received a copy of the GNU General Public License along with this program; if not, write to the Free
" Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
"
" Vim syntax file
" ---------------
" Latest version available from:
"         https://github.com/ascii-boxes/boxes/blob/master/boxes.vim
" Status: This syntax file is not officially part of a boxes release, and simply intended to be downloaded via the link
"         above, but we think it's great when it gets packaged and installed with boxes!
" Known Issues:
"         https://github.com/ascii-boxes/boxes/issues?q=is%3Aissue+is%3Aopen+Vim+Syntax
" _____________________________________________________________________________________________________________________


if exists("b:current_syntax")
    finish
endif


" Removes any old syntax stuff hanging around
syntax clear

" ignore case
syntax case ignore

" Set the keyword characters
syntax iskeyword a-z,A-Z

" TODO spell checking - In a boxes config file, only comments should be spell checked.


"
"  Key Words
"
syntax keyword boxesClassicKeywords author designer revision created revdate tags indent skipwhite skipempty nextgroup=boxesString
syntax keyword boxesBlocks          elastic replace reverse padding shapes skipwhite skipempty
syntax keyword boxesRegStuff        with to once global skipwhite skipempty
syntax keyword boxesParent          parent skipwhite nextgroup=boxesParentPath,boxesParentGlobal
syntax keyword boxesEnd             end skipwhite skipempty nextgroup=boxesNameAtEnd

" Shape Names
syntax keyword boxesShapes nw nnw n nne ne ene e ese
syntax keyword boxesShapes se sse s ssw sw wsw w wnw

" Padding Areas
syntax keyword boxesPads   a[ll] l[eft] r[ight] t top
syntax keyword boxesPads   b[ottom] h[orizontal] v[ertical]

" Delimiter statement
syntax keyword boxesDelim  delim delimiter skipwhite skipempty nextgroup=boxesDelSpec


"
"  Error - this is matched when the others don't match
"
syntax match   boxesError /\S/


"
"  Main syntax definition part
"
syntax match   boxesComma     contained display /,/
syntax match   boxesBraces    display /[{}]/
syntax match   boxesDelSpec   contained display /[^ \t\r]\+/
syntax match   boxesNameAtEnd display /[a-zA-ZäöüÄÖÜ][a-zA-Z0-9_\-üäöÜÄÖß]*/ skipwhite skipempty
syntax match   boxesWord      display /[a-zA-ZäöüÄÖÜ][a-zA-Z0-9_\-üäöÜÄÖß]*/
syntax match   boxesNumber    display /[-+]\=\d\+/

" TODO Introduce a region between delim and delim|end which exists multiple times to cover all escape characters
"      with specialized string definitions

" a list, used inside shape blocks and for the elastic list
syntax region  boxesList matchgroup=Normal start="(" end=")" contains=boxesString,boxesShapes,boxesComma,boxesError,boxesComment

" Strings
syntax region  boxesString display start=/\z\(["~'`!@%&*=:;<>?/|.\\]\)/ skip=/\\\\\|\\\z1/ end=/\z1/ oneline

" File path of a 'parent' definition
syntax match   boxesParentGlobal display "\s:global:\s*$"
syntax match   boxesParentPath contained display "[^:]\{-}$"

" a BOX definition with aliases
syntax region boxesNames matchgroup=boxesBoxStmt start=/\<box\s\+/ end=/\>[ \t\r\n]\+\</re=s,he=s keepend contains=boxesComma

" Comments may appear anywhere in the file
syntax match   boxesComment display /#.*$/

" The SAMPLE block
syntax region  boxesSample matchgroup=boxesBlocks start="sample" end=+^[ \t]*ends[ \t\r]*$+ keepend



"
"  Synchronisation
"
syntax sync clear
syntax sync match boxesSync grouphere boxesNames "box"
syntax sync match boxesSync "parent"
syntax sync minlines=40 maxlines=200


"
"  Highlighting
"
if !exists("did_boxes_syntax_inits")
    let did_boxes_syntax_inits = 1

    hi link boxesNames Special
    hi link boxesNameAtEnd Special
    hi link boxesBoxStmt PreProc
    hi link boxesEnd PreProc
    hi link boxesNumber Number
    hi link boxesString String
    hi link boxesDelSpec boxesString
    hi link boxesComment Comment
    hi link boxesError Error
    hi link boxesClassicKeywords Keyword
    hi link boxesParent PreProc
    hi link boxesParentGlobal PreProc
    hi link boxesParentPath String
    hi link boxesBlocks Statement
    hi link boxesDelim Keyword
    hi link boxesRegStuff Label
    hi link boxesShapes Type
    hi link boxesPads Type
    hi clear boxesSample
    hi clear boxesWord
    hi clear boxesBraces
    hi clear boxesComma
endif


"
"  The current buffer now uses "boxes" syntax highlighting
"
let b:current_syntax = "boxes"



"EOF                                      vim: sw=4 nowrap :
