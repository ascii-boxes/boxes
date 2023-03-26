"
" boxes - Command line filter to draw/remove ASCII boxes around text
" Copyright (c) 1999-2023 Thomas Jensen and the boxes contributors
"
" This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
" License, version 3, as published by the Free Software Foundation.
" This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
" warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
" details.
" You should have received a copy of the GNU General Public License along with this program.
" If not, see <https://www.gnu.org/licenses/>.
" ___________________________________________________________________________________________________________________
"
" Vim syntax file
" ---------------
" Latest version available from:
"         https://github.com/ascii-boxes/boxes/blob/master/boxes.vim
" Status: Stable
" Notes:  - Vim requires this file to have system-dependent line endings (CR/LF on Windows, just LF otherwise)
"         - To activate, copy this file to $VIMRUNTIME/syntax/, for example /usr/share/vim/vim81/syntax/.
"
"====================================================================================================================


if exists("b:current_syntax")
    finish
endif


" Removes any old syntax stuff hanging around
syntax clear

" ignore case
syntax case ignore

" Set the keyword characters
syntax iskeyword a-z,A-Z


"
"  Key Words
"
syntax keyword boxesClassicKeywords author designer revision created revdate tags indent
syntax keyword boxesBlocks          elastic replace reverse padding shapes
syntax keyword boxesRegStuff        with to once global
syntax keyword boxesParent          parent skipwhite nextgroup=boxesParentPath,boxesParentGlobal
syntax keyword boxesEnd             end skipwhite skipempty nextgroup=boxesNameAtEnd

" Shape Names
syntax keyword boxesShapes nw nnw n nne ne ene e ese
syntax keyword boxesShapes se sse s ssw sw wsw w wnw

" Padding Areas
syntax keyword boxesPads   a[ll] l[eft] r[ight] t top
syntax keyword boxesPads   b[ottom] h[orizontal] v[ertical]


"
"  Error - this is matched when the others don't match
"
syntax match   boxesError /\S/


" Delim Regions
" Within these regions, a different special character is used as escape character in strings.
syntax region  delimRegion01 matchgroup=boxesString start=/[ \t\r\n]\+"[~'`!@%&*=:;<>?/|.\\][ \t\r\n]/  end=/\<end\>\|\<delim\>\|\<delimiter\>/he=s-1 contains=boxesString01,boxesList01,@boxesDelimContained
syntax region  delimRegion02 matchgroup=boxesString start=/[ \t\r\n]\+\~["'`!@%&*=:;<>?/|.\\][ \t\r\n]/ end=/\<end\>\|\<delim\>\|\<delimiter\>/he=s-1 contains=boxesString02,boxesList02,@boxesDelimContained
syntax region  delimRegion03 matchgroup=boxesString start=/[ \t\r\n]\+'["~`!@%&*=:;<>?/|.\\][ \t\r\n]/  end=/\<end\>\|\<delim\>\|\<delimiter\>/he=s-1 contains=boxesString03,boxesList03,@boxesDelimContained
syntax region  delimRegion04 matchgroup=boxesString start=/[ \t\r\n]\+`["~'!@%&*=:;<>?/|.\\][ \t\r\n]/  end=/\<end\>\|\<delim\>\|\<delimiter\>/he=s-1 contains=boxesString04,boxesList04,@boxesDelimContained
syntax region  delimRegion05 matchgroup=boxesString start=/[ \t\r\n]\+!["~'`@%&*=:;<>?/|.\\][ \t\r\n]/  end=/\<end\>\|\<delim\>\|\<delimiter\>/he=s-1 contains=boxesString05,boxesList05,@boxesDelimContained
syntax region  delimRegion06 matchgroup=boxesString start=/[ \t\r\n]\+@["~'`!%&*=:;<>?/|.\\][ \t\r\n]/  end=/\<end\>\|\<delim\>\|\<delimiter\>/he=s-1 contains=boxesString06,boxesList06,@boxesDelimContained
syntax region  delimRegion07 matchgroup=boxesString start=/[ \t\r\n]\+%["~'`!@&*=:;<>?/|.\\][ \t\r\n]/  end=/\<end\>\|\<delim\>\|\<delimiter\>/he=s-1 contains=boxesString07,boxesList07,@boxesDelimContained
syntax region  delimRegion08 matchgroup=boxesString start=/[ \t\r\n]\+&["~'`!@%*=:;<>?/|.\\][ \t\r\n]/  end=/\<end\>\|\<delim\>\|\<delimiter\>/he=s-1 contains=boxesString08,boxesList08,@boxesDelimContained
syntax region  delimRegion09 matchgroup=boxesString start=/[ \t\r\n]\+\*["~'`!@%&=:;<>?/|.\\][ \t\r\n]/ end=/\<end\>\|\<delim\>\|\<delimiter\>/he=s-1 contains=boxesString09,boxesList09,@boxesDelimContained
syntax region  delimRegion10 matchgroup=boxesString start=/[ \t\r\n]\+=["~'`!@%&*:;<>?/|.\\][ \t\r\n]/  end=/\<end\>\|\<delim\>\|\<delimiter\>/he=s-1 contains=boxesString10,boxesList10,@boxesDelimContained
syntax region  delimRegion11 matchgroup=boxesString start=/[ \t\r\n]\+:["~'`!@%&*=;<>?/|.\\][ \t\r\n]/  end=/\<end\>\|\<delim\>\|\<delimiter\>/he=s-1 contains=boxesString11,boxesList11,@boxesDelimContained
syntax region  delimRegion12 matchgroup=boxesString start=/[ \t\r\n]\+;["~'`!@%&*=:<>?/|.\\][ \t\r\n]/  end=/\<end\>\|\<delim\>\|\<delimiter\>/he=s-1 contains=boxesString12,boxesList12,@boxesDelimContained
syntax region  delimRegion13 matchgroup=boxesString start=/[ \t\r\n]\+<["~'`!@%&*=:;>?/|.\\][ \t\r\n]/  end=/\<end\>\|\<delim\>\|\<delimiter\>/he=s-1 contains=boxesString13,boxesList13,@boxesDelimContained
syntax region  delimRegion14 matchgroup=boxesString start=/[ \t\r\n]\+>["~'`!@%&*=:;<?/|.\\][ \t\r\n]/  end=/\<end\>\|\<delim\>\|\<delimiter\>/he=s-1 contains=boxesString14,boxesList14,@boxesDelimContained
syntax region  delimRegion15 matchgroup=boxesString start=/[ \t\r\n]\+?["~'`!@%&*=:;<>/|.\\][ \t\r\n]/  end=/\<end\>\|\<delim\>\|\<delimiter\>/he=s-1 contains=boxesString15,boxesList15,@boxesDelimContained
syntax region  delimRegion16 matchgroup=boxesString start=/[ \t\r\n]\+\/["~'`!@%&*=:;<>?|.\\][ \t\r\n]/ end=/\<end\>\|\<delim\>\|\<delimiter\>/he=s-1 contains=boxesString16,boxesList16,@boxesDelimContained
syntax region  delimRegion17 matchgroup=boxesString start=/[ \t\r\n]\+|["~'`!@%&*=:;<>?/.\\][ \t\r\n]/  end=/\<end\>\|\<delim\>\|\<delimiter\>/he=s-1 contains=boxesString17,boxesList17,@boxesDelimContained
syntax region  delimRegion18 matchgroup=boxesString start=/[ \t\r\n]\+\.["~'`!@%&*=:;<>?/|\\][ \t\r\n]/ end=/\<end\>\|\<delim\>\|\<delimiter\>/he=s-1 contains=boxesString18,boxesList18,@boxesDelimContained
syntax region  delimRegion   matchgroup=boxesString start=/[ \t\r\n]\+\\["~'`!@%&*=:;<>?/|.][ \t\r\n]/  end=/\<end\>\|\<delim\>\|\<delimiter\>/he=s-1 contains=boxesString,boxesList,@boxesDelimContained

" Trivial elements such as words, numbers, braces
syntax match   boxesComma     contained display /,/
syntax match   boxesBraces    display /[{}]/
syntax match   boxesNameAtEnd display /[a-zA-Z][a-zA-Z0-9_-]*/
syntax match   boxesWord      display /[a-zA-Z][a-zA-Z0-9_-]*/
syntax match   boxesNumber    display /[-+]\=\d\+/

" These items may appear inside a DELIM region
syntax cluster boxesDelimContained contains=boxesComment,boxesWord,boxesElasticList
syntax cluster boxesDelimContained add=boxesError,boxesBlocks,boxesSample,boxesPads,boxesNumber,boxesBraces
syntax cluster boxesDelimContained add=boxesClassicKeywords,boxesRegStuff,boxesShapes

" a list, used inside shape blocks and for the elastic list
syntax cluster boxesListContained contains=boxesShapes,boxesComma,boxesError,boxesComment
syntax region  boxesList01 contained matchgroup=Normal start="(" end=")" contains=boxesString01,@boxesListContained
syntax region  boxesList02 contained matchgroup=Normal start="(" end=")" contains=boxesString02,@boxesListContained
syntax region  boxesList03 contained matchgroup=Normal start="(" end=")" contains=boxesString03,@boxesListContained
syntax region  boxesList04 contained matchgroup=Normal start="(" end=")" contains=boxesString04,@boxesListContained
syntax region  boxesList05 contained matchgroup=Normal start="(" end=")" contains=boxesString05,@boxesListContained
syntax region  boxesList06 contained matchgroup=Normal start="(" end=")" contains=boxesString06,@boxesListContained
syntax region  boxesList07 contained matchgroup=Normal start="(" end=")" contains=boxesString07,@boxesListContained
syntax region  boxesList08 contained matchgroup=Normal start="(" end=")" contains=boxesString08,@boxesListContained
syntax region  boxesList09 contained matchgroup=Normal start="(" end=")" contains=boxesString09,@boxesListContained
syntax region  boxesList10 contained matchgroup=Normal start="(" end=")" contains=boxesString10,@boxesListContained
syntax region  boxesList11 contained matchgroup=Normal start="(" end=")" contains=boxesString11,@boxesListContained
syntax region  boxesList12 contained matchgroup=Normal start="(" end=")" contains=boxesString12,@boxesListContained
syntax region  boxesList13 contained matchgroup=Normal start="(" end=")" contains=boxesString13,@boxesListContained
syntax region  boxesList14 contained matchgroup=Normal start="(" end=")" contains=boxesString14,@boxesListContained
syntax region  boxesList15 contained matchgroup=Normal start="(" end=")" contains=boxesString15,@boxesListContained
syntax region  boxesList16 contained matchgroup=Normal start="(" end=")" contains=boxesString16,@boxesListContained
syntax region  boxesList17 contained matchgroup=Normal start="(" end=")" contains=boxesString17,@boxesListContained
syntax region  boxesList18 contained matchgroup=Normal start="(" end=")" contains=boxesString18,@boxesListContained
syntax region  boxesList             matchgroup=Normal start="(" end=")" contains=boxesString,@boxesListContained

" Strings
syntax region  boxesString01 contained display start=/\z\([~'`!@%&*=:;<>?/|.\\]\)/ skip=/""\|"\z1/    end=/\z1/ oneline
syntax region  boxesString02 contained display start=/\z\(["'`!@%&*=:;<>?/|.\\]\)/ skip=/\~\~\|\~\z1/ end=/\z1/ oneline
syntax region  boxesString03 contained display start=/\z\(["~`!@%&*=:;<>?/|.\\]\)/ skip=/''\|'\z1/    end=/\z1/ oneline
syntax region  boxesString04 contained display start=/\z\(["~'!@%&*=:;<>?/|.\\]\)/ skip=/``\|`\z1/    end=/\z1/ oneline
syntax region  boxesString05 contained display start=/\z\(["~'`@%&*=:;<>?/|.\\]\)/ skip=/!!\|!\z1/    end=/\z1/ oneline
syntax region  boxesString06 contained display start=/\z\(["~'`!%&*=:;<>?/|.\\]\)/ skip=/@@\|@\z1/    end=/\z1/ oneline
syntax region  boxesString07 contained display start=/\z\(["~'`!@&*=:;<>?/|.\\]\)/ skip=/%%\|%\z1/    end=/\z1/ oneline
syntax region  boxesString08 contained display start=/\z\(["~'`!@%*=:;<>?/|.\\]\)/ skip=/&&\|&\z1/    end=/\z1/ oneline
syntax region  boxesString09 contained display start=/\z\(["~'`!@%&=:;<>?/|.\\]\)/ skip=/\*\*\|\*\z1/ end=/\z1/ oneline
syntax region  boxesString10 contained display start=/\z\(["~'`!@%&*:;<>?/|.\\]\)/ skip=/==\|=\z1/    end=/\z1/ oneline
syntax region  boxesString11 contained display start=/\z\(["~'`!@%&*=;<>?/|.\\]\)/ skip=/::\|:\z1/    end=/\z1/ oneline
syntax region  boxesString12 contained display start=/\z\(["~'`!@%&*=:<>?/|.\\]\)/ skip=/;;\|;\z1/    end=/\z1/ oneline
syntax region  boxesString13 contained display start=/\z\(["~'`!@%&*=:;>?/|.\\]\)/ skip=/<<\|<\z1/    end=/\z1/ oneline
syntax region  boxesString14 contained display start=/\z\(["~'`!@%&*=:;<?/|.\\]\)/ skip=/>>\|>\z1/    end=/\z1/ oneline
syntax region  boxesString15 contained display start=/\z\(["~'`!@%&*=:;<>/|.\\]\)/ skip=/??\|?\z1/    end=/\z1/ oneline
syntax region  boxesString16 contained display start=/\z\(["~'`!@%&*=:;<>?|.\\]\)/ skip=/\/\/\|\/\z1/ end=/\z1/ oneline
syntax region  boxesString17 contained display start=/\z\(["~'`!@%&*=:;<>?/.\\]\)/ skip=/||\||\z1/    end=/\z1/ oneline
syntax region  boxesString18 contained display start=/\z\(["~'`!@%&*=:;<>?/|\\]\)/ skip=/\.\.\|\.\z1/ end=/\z1/ oneline
syntax region  boxesString             display start=/\z\(["~'`!@%&*=:;<>?/|.]\)/  skip=/\\\\\|\\\z1/ end=/\z1/ oneline

" File path of a 'parent' definition
syntax match   boxesParentGlobal display "\s:global:\s*$"
syntax match   boxesParentPath contained display "[^:]\{-}$"

" a BOX definition with aliases
syntax region  boxesNames matchgroup=boxesBoxStmt start=/\<box\s\+/ end=/\>[ \t\r\n]\+\</re=s,he=s keepend contains=boxesComma

" a DELIM statement starts a delim region
syntax match   boxesDelim display /\<delim\%(iter\)\?\>/ nextgroup=delimRegion15,delimRegion06,delimRegion03,delimRegion04,delimRegion05,delimRegion02,delimRegion07,delimRegion08,delimRegion09,delimRegion10,delimRegion11,delimRegion12,delimRegion13,delimRegion14,delimRegion01,delimRegion16,delimRegion17,delimRegion18,delimRegion

" Comments may appear anywhere in the file
syntax match   boxesComment display /#.*$/ contains=@Spell

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

    hi link boxesString01 String
    hi link boxesString02 String
    hi link boxesString03 String
    hi link boxesString04 String
    hi link boxesString05 String
    hi link boxesString06 String
    hi link boxesString07 String
    hi link boxesString08 String
    hi link boxesString09 String
    hi link boxesString10 String
    hi link boxesString11 String
    hi link boxesString12 String
    hi link boxesString13 String
    hi link boxesString14 String
    hi link boxesString15 String
    hi link boxesString16 String
    hi link boxesString17 String
    hi link boxesString18 String
    hi link boxesString   String

    hi link boxesBoxStmt PreProc
    hi link boxesEnd PreProc
    hi link boxesNames Special
    hi link boxesNameAtEnd Special

    hi link boxesParent PreProc
    hi link boxesParentGlobal PreProc
    hi link boxesParentPath String

    hi link boxesClassicKeywords Keyword
    hi link boxesDelim Keyword
    hi link boxesBlocks Statement
    hi link boxesRegStuff Label

    hi link boxesShapes Type
    hi link boxesPads Type
    hi link boxesNumber Number

    hi link boxesComment Comment
    hi link boxesError Error

    hi clear boxesSample
    hi clear boxesWord
    hi clear boxesBraces
    hi clear boxesComma
endif


"
"  The current buffer now uses "boxes" syntax highlighting
"
let b:current_syntax = "boxes"



"EOF                                      vim: sw=4 nowrap:
