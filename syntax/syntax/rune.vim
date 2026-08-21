if exists("b:current_syntax")
	finish
endif

syntax clear

syntax keyword rune_type let leb setw setb fn
syntax match rune_type "%."
syntax match rune_type "\\."

syntax match rune_operator "[+\-*/><=%\[\]&|^~!]"
syntax keyword rune_keyword pub if else while for arg ret
syntax match rune_functionCall "\k\+\s*\ze("
syntax match rune_functionCall "\k\+\s*\ze {"

syntax region rune_string start=+"+ end=+"+ skip=+\\"+
syntax region rune_char start=+'+ end=+'+ skip=+\\'+

syntax match rune_number "\<\d\+\>"
syntax match rune_number "\<\d\+\.\d\+\>"
syntax match rune_number "\<0b[01]\+\>"
syntax match rune_number "\<0x[0-9a-fA-F]\+\>"
syntax match rune_number "\<[A-Z_A-Z]\+\>"

syntax match rune_preproc "^#.*$"

syntax match rune_comment "//.*$" contains=NONE containedin=ALL

highlight default link rune_comment Comment
highlight default link rune_keyword Keyword
highlight default link rune_type Type
highlight default link rune_string String
highlight default link rune_char Character
highlight default link rune_number Number
highlight default link rune_operator Operator
highlight default link rune_functionName Function
highlight default link rune_functionCall Function
highlight default link rune_preproc PreProc

let b:current_syntax = "rune"
