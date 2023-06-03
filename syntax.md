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

`   

expression :=
    sequence_expression
    
sequence_expression :=
    sequence_expression ',' assignment_expression
    assignment_expression
    
assignment_expression :=
    conditional_expression '=' assignment_expression 
    conditional_expression '+=' assignment_expression
    conditional_expression '-=' assignment_expression
    conditional_expression '*=' assignment_expression
    conditional_expression '/=' assignment_expression
    conditional_expression '%=' assignment_expression
    conditional_expression '<<=' assignment_expression
    conditional_expression '>>=' assignment_expression
    conditional_expression '||=' assignment_expression
    conditional_expression '&&=' assignment_expression
    conditional_expression '|=' assignment_expression
    conditional_expression '&=' assignment_expression
    conditional_expression
    
conditional_expression :=
    logical_inclusive_or_expression '?' expression ':' assignment_expression
    logical_inclusive_or_expression

logical_inclusive_or_expression :=
    logical_inclusive_or_expression '||' logical_exclusive_or_expression
    logical_exclusive_or_expression
    
logical_exclusive_or_expression :=
    logical_exclusive_or_expression '^^' logical_and_expression
    logical_and_expression
    
logical_and_expression :=
    logical_and_expression '&&' bitwise_inclusive_or_expression
    bitwise_inclusive_or_expression
    
bitwise_inclusive_or_expression :=
    bitwise_inclusive_or_expression '|' bitwise_exclusive_or_expression
    bitwise_exclusive_or_expression
    
bitwise_exclusive_or_expression :=
    bitwise_exclusive_or_expression '^' bitwise_and_expression
    bitwise_and_expression
    
bitwise_and_expression :=
    bitwise_and_expression '&' equality_expression
    equality_expression
    
equality_expression :=
    equality_expression '==' relational_expression
    equality_expression '!=' relational_expression
    relational_expression
        
relational_expression :=
    relational_expression '<' shift_expression
    relational_expression '>' shift_expression
    relational_expression '<=' shift_expression
    relational_expression '>=' shift_expression
    shift_expression
    
shift_expression :=
    shift_expression '<<' additive_expression
    shift_expression '>>' additive_expression
    additive_expression
    
additive_expression :=
    additive_expression '+' multiplicative_expression
    additive_expression '-' multiplicative_expression
    multiplicative_expression
    
multiplicative_expression :=
    multiplicative_expression '*' prefix_expression
    multiplicative_expression '/' prefix_expression
    multiplicative_expression '%' prefix_expression
    prefix_expression
    
prefix_expression :=
    '++' prefix_expression
    '--' prefix_expression
    '+' prefix_expression
    '-' prefix_expression
    '~' prefix_expression
    '!' prefix_expression
    postfix_expression

postfix_expression :=
    postfix_expression '++'
    postfix_expression '--'
    postfix_expression '.' identifier
    primary_expression

primary_expression :=
    identifier '[' expression ']'
    identifier '(' expression_list ')'
    TYPE '(' expression_list ')'
    NUMBER
    identifier
    '(' expression ')'

expression_ :=
    // TODO

    expression'.x'
    expression'.y'
    expression'.z'
    expression'.w'
    TYPE '(' expression_list ')'
    # 这几个新语法肯定被用了
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

expression_list :=
	/* empty */
	# 这是什么？？？ 为什么会empty
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