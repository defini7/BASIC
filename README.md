# BASIC

## Introduction

It is an attempt to implement MSX Basic (1988) but there are still some missing commands.
Code must be compiled with C++20 and for convenience there is *.pro* file for Qt Creator.

## Features

1) Variables
```basic
10 LET a = "Hi, mom!"
20 b = 69
```

2) PRINT
```basic
10 LET name = "Alex"
20 PRINT "Hi, my name is "; name
```

3) INPUT
```basic
10 INPUT "What's your name?"; name
20 PRINT "Nice to meet you, "; name
```

4) FOR
```basic
10 FOR i=0 TO 5 STEP 0.5
20 PRINT i
30 NEXT i
```

5) IF
```basic
10 INPUT guess
20 IF guess == "John" THEN PRINT "yes" ELSE PRINT "no"
```

6) Other statements
- CLS
- GOTO \<line>
- SLEEP \<milliseconds>

7) Math functions
- SIN \<radians>
- COS \<radians>
- TAN \<radians>
- ARCSIN \<sin>
- ARCCOS \<cos>
- ARCTAN \<tan>
- LOG \<arg> - 10'th base log
- EXP \<arg>
- ABS \<arg>
- SIGN \<arg>
- INT \<arg> - truncates \<arg>
- VAL \<arg> - converts \<arg> to int if \<arg> is string, does not do anything if \<arg> is int

8) Operators

- = - asssign
- == - is equals
- <> - not equals
- < - less than
- \> - greater than
- <= - less than or equals
- \>= - greater than or equals
- \- minus (can be unary)
- \+ - plus (can be unary)
- \* - multiply
- / - divide
- ^ - power

## To Do

1) Arrays

## References

1) [WebMSX](https://webmsx.org/) - online MSX Basic emulator