
# Python Interpreter in C/C++

This project is a custom Python interpreter built in C/C++.  
It supports basic Python features such as variables, arithmetic operations, conditional statements (`if`, `elif`, `else`), and looping constructs (`while`, `for`).

Over the course of development, I wrote several thousand lines of code implementing the lexer, parser, and runtime engine.  
I also created multiple test cases to verify the correctness of syntax parsing, expression evaluation, and control flow.

## Features
- Custom lexer and parser written in C/C++
- Execution of basic Python-like syntax
- Support for conditionals (`if`, `else`, `elif`)
- Support for loops (`while`, `for`)
- Variable handling and arithmetic operations
- Extensive testing framework

## Build Instructions
To build the interpreter:
```bash
g++ -o interpreter *.cpp
```

To run:
```bash
./interpreter test.py
```

## Example
```python
x = 0
while x < 5:
    if x % 2 == 0:
        print(x)
    x = x + 1
```

## Author
Created by Daniel Solomon.

>>>>>>> 4c83332 (Add proper README content)
