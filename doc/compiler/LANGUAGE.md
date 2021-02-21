# Chapman Language 

### Example
```chapman
// This is a comment
val aConstant = 10;
var aVariable = 20;

#addTwelve(value) {
    return value + 12;
}
```

### Grammar
```
stmt -> 
    | val_decl
    | var_decl
    | func_decl

val_decl ->
    "val" id static_assign? ";"
var_decl ->
    "var" id static_assign? ";"

func_decl ->
    "#" id "(" func_args? ")" "{" func_body "}"

func_body -> 
    | val_decl
    | var_decl
    | func_invoke

func_invoke ->
    id "(" func_args? ")" ";"

func_args ->
    id ("," func_args)
```