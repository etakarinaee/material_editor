Material Scripting Language

This is a very simplified version of [GLSL](https://wikis.khronos.org/opengl/Core_Language_(GLSL)), intended to be easy
to use even for artists

### Assignment

An assigment binds a value to a named variable: `<identifier> = <expression>`

```c++
x = 1
y = 2
z = 3
bar = 12 + 5
foo = bar
```

### Identifiers

An identifier must:

* start with a lowercase letter, uppercase letter. note that they cannot start with an underscore to avoid ambiguous
  names like `_temp` or `__internal`.
* be followed by zero or more letters, digits, or underscores
* not be a [reserved keyword](#reserved-keywords)

### Expressions

A right-hand of an assigment is an expression, which can be:

* integer literal
* float literal
* boolean literal
* [identifier](#identifiers)
* [function call](#function-calls)
* [binary operator](#binary-operators)
* unary negation
* grounded expressions

#### Binary operators

* `+` - addition
* `-` - subtraction
* `*` - multiplication
* `/` - division

#### Operator precedence

From highest to lowest:

1. `()`
2. `-`
3. `*` `/`
4. `+` `-`

### Function calls

A function call invokes a built-in (or user defined) function and evaluates it to value. Function calls are expressions,
which means they can appear on the right-hand of an [assignment](#assignment) or as arguments to other function calls:
`<function>(<arg0>, <arg1>, ..., <argN>`

* function name follows the same rules as [identifiers](#identifiers)
* opening parenthesis must immediately follow the function name
* arguments are comma-separated expressions
* trailing commas are not allowed
* a function may accept zero or more arguments
* closing parenthesis ends the call

TODO: how do we define the functions?

```c++
foo = sqrt(2.0)
color = pow(foo, 2)
```

#### Built-in functions:

TODO

#### Reserved keywords

`true  false`

#### EBNF Grammar

```
program = { statement } ;

statement = assignment ;

assignment = identifier , "=" , expression ;

expression = term , { ( "+" | "-" ) , term } ;

term = factor , { ( "*" | "/" ) , factor } ;

factor = unary ;

unary = [ "-" ] , primary ;

primary = function_call
        | literal
        | identifier
        | "(" , expression , ")" ;

function_call = identifier , "(" , [ arguments ] , ")" ;

arguments = expression , { "," , expression } ;

literal = float
        | integer
        | boolean ;

identifier = letter , { letter | digit | "_" } ;

float = digit , { digit } , "." , { digit }
      | "." , digit , { digit } ;

integer = digit , { digit } ;

boolean = "true" | "false" ;

letter = "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" | "j" 
       | "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r" | "s" | "t" 
       | "u" | "v" | "w" | "x" | "y" | "z" 
       | "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J" 
       | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" | "S" | "T" 
       | "U" | "V" | "W" | "X" | "Y" | "Z" ;

digit = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;
```