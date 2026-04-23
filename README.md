# Introduction

Welcome here! This is MSX BASIC interpreter with some small changes (e.g. == instead of = for comparisons and improved error messages). You can try MSX BASIC online [here](https://webmsx.org/).

## Table of Contents
1. [Getting Started](#getting-started)
2. [Numbers](#numbers)
3. [Math Functions](#math-functions)
4. [Commands Reference](#commands-reference)
5. [Arrays](#arrays)
6. [Program Mode](#program-mode)
7. [Tips and Tricks](#tips-and-tricks)
8. [Contribute](#contribute)

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
| `LN(x)` | Base-e logarithm |
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
result = (a + b) * 2
```

### IF ... THEN ... ELSE - Make Decisions
Control program flow:
```basic
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

## Arrays

### Array Declaration (DIM Statement)
Declare arrays to store multiple values:
```basic
DIM array_name(size)
DIM arr1(10), arr2(20), arr3(5)
```

- Use `DIM` keyword to declare arrays
- Specify array name and size in parentheses
- Multiple arrays can be declared in a single DIM statement, separated by commas
- Array sizes must be positive integers
- All elements are initialized to 0.0

### Array Assignment and Access
Access and modify array elements:
```basic
LET array(index) = value
LET A(0) = 100
LET B(i) = A(i) + 1
PRINT array(index)
```

- Use parentheses `(index)` to access array elements
- Indices are 0-based (first element is at index 0)
- Array indices must be numeric expressions (0 to size-1)
- Bounds checking prevents accessing invalid indices

### Array Usage Tips
1. Declare all arrays at the beginning of your program
2. Use consistent naming: `scores(10)`, `names(50)`, etc.
3. Remember arrays are 0-indexed
4. Use FOR loops to efficiently process array elements

### Example: Array Processing
```basic
10 DIM values(10)
20 FOR i = 0 TO 9
30   LET values(i) = i * 10
40 NEXT
50 FOR i = 0 TO 9
60   PRINT values(i)
70 NEXT
```

### Example: Statistics
```basic
10 DIM scores(5)
20 LET scores(0) = 85
30 LET scores(1) = 92
40 LET scores(2) = 78
50 LET scores(3) = 95
60 LET scores(4) = 88
70 LET sum = 0
80 FOR i = 0 TO 4
90   LET sum = sum + scores(i)
100 NEXT
110 LET average = sum / 5
120 PRINT "Sum: "; sum
130 PRINT "Average: "; average
140 END
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

5. **Logical operators**: `AND`, `OR` for combining conditions:
```basic
IF x > 0 AND x < 10 THEN PRINT "Between 0 and 10"
IF a == 1 OR b == 1 THEN PRINT "At least one is 1"
```

## Contribute

If any errors occur or you want to add some more features then just fork and send PR or create an issue.
