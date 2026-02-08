
### Toy-Style C Language Interpreter
#### Project Overview
This is a toy-level C language interpreter that implements the core features of the C language, including variable definition, arithmetic operations, control flow statements, function definition and invocation, input/output, etc. The interpreter has two versions: a Python version and a C language version.

#### Supported Syntax
1. Variable Definition and Assignment
```c
int a = 10;
int b = 5;
a = 20;
```
2. Arithmetic Operations
```c
int sum = a + b;
int difference = a - b;
int product = a * b;
int quotient = a / b;
```
3. Control Flow Statements
- if-else conditional statements
```c
if (a > b) {
    print(1);
} else {
    print(0);
}
```
- for loops
```c
for (int i = 0; i < 5; i = i + 1) {
    print(i);
}
```
4. Function Definition and Invocation
```c
def add(int x, int y) {
    int sum = x + y;
    return sum;
}

int result = add(a, b);
print(result);
```
5. Input and Output
```c
int input_value = input();
print(input_value);
```

#### Usage of the Python Version
1. Run an external .c file
```bash
python mini_interpreter.py test.c
```
2. Use within Python code
```python
from mini_interpreter import Interpreter

interpreter = Interpreter()
code = """
int a = 10;
int b = 5;
int c = a + b;
print(c);
"""
interpreter.run(code)
```

#### Usage of the C Version
Compile and run:
```bash
gcc -o c_interpreter c_interpreter.c
./c_interpreter test.c
```

#### Version Comparison
| Aspect                | Python Version                                                                 | C Language Version                                                              |
|-----------------------|--------------------------------------------------------------------------------|---------------------------------------------------------------------------------|
| Advantages            | Concise implementation, easy to understand and extend                          | Fast execution speed, input function works properly                            |
| Disadvantages         | Slow execution speed, potential issues with the input function                 | Relatively complex code, higher difficulty in extension                        |
| Applicable Scenarios  | Learning interpreter principles, rapid prototype development                  | Scenarios with high performance requirements, learning to implement interpreters in C |

#### Limitations and Notes
- Only integer type is supported; advanced features such as floating-point numbers, strings, and arrays are not included.
- Function definitions must include a return statement.
- The input function `input()` only supports integer input.
- Advanced C language features like preprocessing directives, structs, and pointers are not supported.
- This is only a toy-style interpreter, intended for learning and understanding the working principles of interpreters.

#### Test Cases
The project includes a `test.c` file for testing all features of the interpreter:
- Basic variable definition and arithmetic operations
- Function definition and invocation
- if-else conditional statements
- for loops
- Input function

#### Extension Suggestions
If you want to extend this interpreter, consider the following directions:
- Add support for floating-point numbers
- Implement string type and related operations
- Add support for arrays and pointers
- Implement more standard library functions
- Optimize the syntax parser to support more C language features



# 玩具版 C 语言解释器

## 项目简介

这是一个玩具性质的 C 语言解释器，实现了 C 语言的核心功能，包括变量定义、算术运算、控制流语句、函数定义和调用、输入输出等功能。该解释器有两个版本：Python 版本和 C 语言版本。

## 支持的语法

### 1. 变量定义与赋值

```c
int a = 10;
int b = 5;
a = 20;
```

### 2. 算术运算

```c
int sum = a + b;
int difference = a - b;
int product = a * b;
int quotient = a / b;
```

### 3. 控制流语句

#### if-else 条件语句

```c
if (a > b) {
    print(1);
} else {
    print(0);
}
```

#### for 循环

```c
for (int i = 0; i < 5; i = i + 1) {
    print(i);
}
```

### 4. 函数定义与调用

```c
def add(int x, int y) {
    int sum = x + y;
    return sum;
}

int result = add(a, b);
print(result);
```

### 5. 输入输出

```c
int input_value = input();
print(input_value);
```

## Python 版本使用方法

### 运行外部 .c 文件

```bash
python mini_interpreter.py test.c
```

### 在 Python 代码中使用

```python
from mini_interpreter import Interpreter

interpreter = Interpreter()
code = """
int a = 10;
int b = 5;
int c = a + b;
print(c);
"""
interpreter.run(code)
```

## C 版本使用方法

### 编译与运行

```bash
gcc -o c_interpreter c_interpreter.c
./c_interpreter test.c
```

## 版本对比

### Python 版本

- **优点**：实现简洁，易于理解和扩展
- **缺点**：执行速度较慢，输入函数可能存在问题
- **适用场景**：学习解释器原理，快速原型开发

### C 语言版本

- **优点**：执行速度快，输入函数工作正常
- **缺点**：代码相对复杂，扩展难度较大
- **适用场景**：性能要求较高的场景，学习 C 语言实现解释器

## 限制与注意事项

- 仅支持整数类型，不支持浮点数、字符串、数组等高级特性
- 函数定义必须包含 `return` 语句
- 输入函数 `input()` 仅支持整数输入
- 不支持 C 语言的预处理指令、结构体、指针等高级特性
- 这只是一个玩具性质的解释器，用于学习和理解解释器的工作原理

## 测试用例

项目包含一个 `test.c` 文件，用于测试解释器的各项功能：

- 基本变量定义和算术运算
- 函数定义和调用
- if-else 条件语句
- for 循环
- 输入函数

## 扩展建议

如果您想扩展这个解释器，可以考虑以下方向：

1. 添加对浮点数的支持
2. 实现字符串类型和相关操作
3. 添加数组和指针支持
4. 实现更多的标准库函数
5. 优化语法分析器，支持更多 C 语言特性
