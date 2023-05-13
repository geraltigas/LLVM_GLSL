```
toplevel :=
	version definition*

version :=
	'#version' NUMBER

definition :=
	function_definition
	global_variable_definition

function_definition :=
	function_prototype '{' sentence* '}'

function_prototype :=
	TYPE identifier '(' ')'
	TYPE identifier '(' TYPE identifier (',' TYPE identifier)* ')'

sentence :=
	'{' sentence* '}'
	expression ';'
	variable_definition ';'
	'if' '(' expression ')' sentence
	'if' '(' expression ')' sentence 'else' sentence
	'while' '(' expression ')' sentence
	'do' sentence 'while' '(' expression ')' ';'
	'for' '(' sentence ';' expression ';' expression ')' sentence
	'break' ';'
	'continue' ';'
	'return' ';'
	'return' expression ';'

variable_definition :=
	TYPE identifier '=' expression
	'const' TYPE identifier '=' expression

expression :=
	'{' expression_list '}'
	'(' expression ')'
	NUMBER
	identifier
	identifier '(' expression_list ')'
	identifier '[' expression ']'
	expression '++'
	expression '--'
	'++' expression
	'--' expression
	'+' expression
	'-' expression
	'~' expression
	'!' expression
	expression '*' expression
	expression '/' expression
	expression '%' expression
	expression '+' expression
	expression '-' expression
	expression '<<' expression
	expression '>>' expression
	expression '>=' expression
	expression '<=' expression
	expression '>' expression
	expression '<' expression
	expression '==' expression
	expression '!=' expression
	expression '&' expression
	expression '^' expression
	expression '|' expression
	expression '&&' expression
	expression '^' expression
	expression '||' expression
	expression '?' expression ':' expression
	expression '=' expression
	expression '+=' expression
	expression '-=' expression
	expression '*=' expression
	expression '/=' expression
	expression '%=' expression
	expression '<<=' expression
	expression '>>=' expression
	expression '||=' expression
	expression '&&=' expression
	expression '|=' expression
	expression '&=' expression
	expression_list

expression_list :=
	/* empty */
	expression (',' expression)*

global_variable_definition :=
	layout_defaults TYPE identifier ';'

layout_defaults :=
	layout_uniform_defaults
	layout_in_defaults
	layout_out_defaults
	/* empty */

layout_uniform_defaults :=
	layout_qualifier 'uniform'

layout_in_defaults :=
	layout_qualifier 'in'

layout_out_defaults :=
	layout_qualifier 'out'

layout_qualifier :=
	'layout' '(' layout_qualifier_id_list ')'

layout_qualifier_id_list :=
	layout_qualifier_id (',' layout_qualifier_id)*

layout_qualifier_id :=
	layout_identidier
	layout_identidier '=' NUMBER

layout_identidier :=
	'location'
	'binding'
```