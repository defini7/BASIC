# Introduction

Welcome here! This is MSX BASIC interpreter with some small changes (e.g. == instead of = for comparisons and improved error messages). You can try MSX BASIC online [here](https://webmsx.org/).

## Table of Contents
1. [Getting Started](#getting-started)
2. [Numbers](#numbers)
3. [Math Functions](#math-functions)
4. [Commands Reference](#commands-reference)
5. [Program Mode](#program-mode)
6. [Contribute](#contribute)

## Getting Started

This interpreter lets you run BASIC commands either immediately (immediate mode) or write programs with line numbers (program mode). You can type commands and see results right away!

### Numbers
Numbers can be written in different formats:
- Regular decimal: `42`, `3.14`
- Hexadecimal (base 16): `&H2A`
- Binary (base 2): `&B101010`
- Octal (base 8): `&O56226`

### Math Functions

| Function | What it does |
|----------|--------------|
| `SIN(x)` | Sine of x (radians) |
| `COS(x)` | Cosine of x |
| `TAN(x)` | Tangent of x |
| `ARCSIN(x)` | Inverse sine (-1 to 1) |
| `ARCCOS(x)` | Inverse cosine (-1 to 1) |
| `ARCTAN(x)` | Inverse tangent |
| `SQR(x)` | Square root |
| `LOG(x)` | Base-10 logarithm |
| `EXP(x)` | e to the power x |
| `ABS(x)` | Absolute value |
| `SGN(x)` | Sign (-1, 0, or 1) |
| `INT(x)` | Integer part (truncates) |
| `VAL(x)` | Convert string to number |
| `RND` | Random number 0-1 |

## Commands Reference

### PRINT - Display Information
Show values on the screen:
```basic
PRINT "Hello World"
PRINT "The answer is "; 42
PRINT x + y
PRINT "Sum:"; a + b
```

Use semicolons `;` to keep output on the same line. Without a final semicolon, PRINT adds a newline.

### INPUT - Get User Input
Ask the user for information:
```basic
INPUT "What is your name? "; name
INPUT "Enter age: "; age
INPUT password
```

### LET - Assign Values
Store values in variables:
```basic
LET x = 10
LET name = "Alice"
LET result = (a + b) * 2
```

Unfortunately you can't declare variables without LET.

### IF ... THEN ... ELSE - Make Decisions
Control program flow:
```
IF x > 10 THEN PRINT "x is big"
IF score >= 90 THEN grade = "A" ELSE grade = "B"
IF a == b THEN PRINT "Equal" ELSE PRINT "Different"
```

### FOR ... TO ... STEP ... NEXT - Loops
Repeat code multiple times:
```basic
FOR i = 1 TO 10
PRINT "Count: "; i
NEXT i

FOR i = 0 TO 100 STEP 10
PRINT i
NEXT

FOR i = 10 TO 1 STEP -1
PRINT "Countdown: "; i
NEXT
```

### GOTO - Jump to Line
Move to a different line in your program:
```basic
10 PRINT "Infinite loop!"
20 GOTO 10
```

### GOSUB ... RETURN - Subroutines
Call and return from subroutines:
```basic
10 GOSUB 100
20 PRINT "Back from subroutine"
30 END

100 PRINT "In subroutine"
110 RETURN
```

### SLEEP - Pause Program
Wait for specified milliseconds:
```basic
PRINT "Wait 2 seconds..."
SLEEP 2000
PRINT "Done!"
```

### CLS - Clear Screen
Clear the display:
```basic
CLS
```

### REM - Comments
Add notes in your code (ignored by interpreter):
```basic
REM This is a comment
LET x = 42: REM This also works
```

### END - Stop Program
Terminate program execution:
```basic
IF score < 0 THEN END
```

## Program Mode

### Line Numbers
Create programs by adding line numbers:
```basic
10 PRINT "Hello"
20 INPUT "Enter number: "; n
30 IF n < 0 THEN END
40 PRINT "You entered: "; n
50 GOTO 20
```

### Program Commands

| Command | What it does |
|---------|--------------|
| `LIST` | Show all program lines |
| `RUN` | Execute the program |
| `NEW` | Clear current program |
| `LOAD "filename"` | Load program from file |

Example:
```basic
NEW
10 PRINT "My Program"
20 FOR i = 1 TO 5
30 PRINT "Line "; i
40 NEXT i
RUN
LIST
```

## Tips and Tricks

1. **Multiple statements** on one line use colons `:`:
```basic
LET x = 5 : PRINT "x = "; x : LET y = x * 2
```

2. **String concatenation** uses `+`:
```basic
full = "Hello" + " " + "World"
```

3. **Comparison operators**: `==`, `<>` (not equal), `<`, `>`, `<=`, `>=`

4. **Variable names** can be longer than a single letter!

## Contribute

If any errors occur or you want to add some more features then just fork and send PR or create an issue.
