" Vim syntax file
" Language:      "boxes" configuration file
" Maintainer:    Thomas Jensen <boxes@thomasjensen.com>
" Latest Change: September 12, 1999 (Sunday, 01:14h)
" Vim Version:   5.4
" Filenames:     .boxes boxes-config boxes
" URL:           https://github.com/ascii-boxes/boxes/blob/master/boxes.vim
"
" Problems:    - Strings must start and end with double quotes (") and the
"                escape character is always backslash (\), which does not
"                allow correct colorization of strings following a DELIM.
"
"              + Region ends are matched inside contained strings
"                SOLVED! Thanks Dr. Charles E. Campbell!


" Removes any old syntax stuff hanging around
syntax clear

" ignore case
syntax case ignore

" Set the keyword characters
set iskeyword=a-z,A-Z


"
"  Key Words
"
syntax keyword boxesClassicKeywords contained author revision created revdate indent skipwhite skipempty nextgroup=boxesString
syntax keyword boxesBlocks   contained elastic replace reverse padding shapes
syntax keyword boxesRegStuff contained with to once global

" Shape Names
syntax keyword boxesShapes   contained nw nnw n nne ne ene e ese
syntax keyword boxesShapes   contained se sse s ssw sw wsw w wnw

" Padding Areas
syntax keyword boxesPads     contained a[ll] l[eft] r[ight] t top
syntax keyword boxesPads     contained b[ottom] h[orizontal] v[ertical]

" Delimiter statement
syntax keyword boxesDelim    contained delim delimiter skipwhite skipempty nextgroup=boxesDelSpec


"
"  Error - this is matched when the others don't match
"
syntax match   boxesError /\S/


"
"  Main syntax definition part
"
syntax match   boxesComma   contained /,/
syntax match   boxesBraces  contained /[{}]/
syntax match   boxesDelSpec contained /[^ \t\r]\+/
syntax match   boxesWord    contained /[a-zA-ZäöüÄÖÜ][a-zA-Z0-9\-_üäöÜÄÖß]*/
syntax match   boxesNumber  contained /[-+]\=\d\+/

" a list, used inside shape blocks and for the elastic list
syntax region  boxesList    contained matchgroup=Normal start="(" end=")" contains=boxesString,boxesShapes,boxesComma,boxesError,boxesComment

" Strings
syntax region  boxesString contained start=/"/ skip=/\\\\\|\\"/ end=/"/ oneline


" These items may appear inside a BOX..END block
syntax cluster boxesInside contains=boxesComment,boxesWord,boxesElasticList
syntax cluster boxesInside add=boxesError,boxesBlocks,boxesSample,boxesDelim
syntax cluster boxesInside add=boxesClassicKeywords,boxesShapeBlk,boxesPadBlock
syntax cluster boxesInside add=boxesRegStuff,boxesShapes,boxesList
syntax cluster boxesInside add=boxesString,boxesPads,boxesNumber,boxesBraces

" The main box design blocks BOX..END
syntax region  boxesDesign matchgroup=boxesBoxStmt start="box" skip="ends" end="end" keepend contains=@boxesInside skipwhite skipempty nextgroup=boxesWord

" The SAMPLE block
syntax region  boxesSample contained matchgroup=boxesBlocks start="sample" end=+^[ \t]*ends[ \t\r]*$+ keepend contains=NONE

" Comments may appear anywhere in the file
syntax match   boxesComment /#.*$/



"
"  Synchronisation
"
syntax sync clear
syntax sync match boxesSync grouphere boxesDesign "box"


"
"  Highlighting
"
if !exists("did_boxes_syntax_inits")
    let did_boxes_syntax_inits = 1

    hi link boxesBoxStmt Define
    hi link boxesNumber Number
    hi link boxesString String
    hi link boxesDelSpec boxesString
    hi link boxesComment Comment
    hi link boxesError Error
    hi link boxesClassicKeywords Keyword
    hi link boxesBlocks Statement
    hi link boxesDelim boxesBlocks
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




"EOF                                      vim: sw=4
