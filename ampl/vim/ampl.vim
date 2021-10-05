" Vim syntax file
" Language: AMPL
" Maintainer: W. H. K. Bester (whkbester@cs.sun.ac.za)
" Latest Revision: 30 July 2017

if exists("b:current_syntax")
	finish
endif

" The following sections -- keywords, literals, and comments -- define the
" character groups that are recognised by the Vim highlighter as constituting
" a meaningful unit.  They do not apply the actual highlighting, though.  This
" is done by linking the classes defined below with standard definitions in
" any particular colour sceme.  This way a colour scheme is independent of the
" language being highlighted; it does not need to know the language details.

" " keywords
syn keyword	amplBoolean				false true
syn keyword	amplConditional			if elif else
syn keyword	amplBuiltinFunction		input output
syn keyword	amplOperator			and not mod or = /= < > <= >= + - * / &
syn keyword	amplRepeat				while
syn keyword	amplBlockStatement		chillax end :
syn keyword	amplDefineStatement		as program returns takes vars
syn keyword	amplStatement			back let do
syn keyword	amplType				array boolean integer

" literals
syn match	amplNumber				"-\?\d\+"
syn match	amplIdentifier			"\<[a-zA-Z_]\w*\>"
syn match	amplUserFuncDef         /^\s*\zs[a-zA-Z_]\w*\ze\s*:/
syn match	amplUserFunction		/\<[a-zA-Z_][a-zA-Z0-9_]*\s*(/me=e-1,he=e-1
syn region	amplString matchgroup=amplString start=+"+ skip=+\\.+ end=+"+

" comments
syn region	amplComment start="{" end="}" contains=amplTodo,amplComment
syn keyword	amplTodo contained		TODO FIXME XXX DEBUG NOTE HBD
syn match	amplTodo contained		"HERE BE DRAGONS"
syn match	amplTodo contained		"HIC SVNT DRACONES"
" Anyone who alleges a Harry Potter or Game of Thrones reference, will be
" eaten by said dragons.  Fiere potest vt cerebrum tvvm liquefiat.  On the
" other hand: tugh qoH nachDaj je chevlu'ta'.

" associations
let b:current_syntax = "ampl"

" The following is a bit colourful, rather like what SublimeText fanboys are
" used to.  Do feel free to tune to tase.  (And if you are a SublimeText fan:
" I bite my thumb at you, and a plague on your house!)

hi def link	amplBlockStatement		Statement
hi def link	amplBoolean				Boolean
hi def link	amplComment				Comment
hi def link	amplConditional			Conditional
hi def link	amplDefineStatement		Statement
hi def link	amplBuiltinFunction		Statement
" hi def link	amplIdentifier			Identifier
hi def link	amplNumber				Number
hi def link	amplRepeat				Repeat
hi def link	amplTodo				Todo
hi def link	amplType				Type
hi def link amplOperator			Operator
hi def link amplStatement			Keyword
hi def link amplString				String
hi def link amplUserFunction		Identifier
hi def link amplUserFuncDef			Identifier
