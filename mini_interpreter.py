#!/usr/bin/env python3
"""
精简版C语言解释器

功能：
1. 基础语法系统：整数变量、算术运算、优先级和括号
2. 控制流结构：if-else条件语句、for循环
3. 函数系统：函数定义、调用、作用域隔离
4. 输入输出功能：input()和print()函数
5. 扩展性架构：基于列表结构的语法规则定义
"""

import re
import sys

# 词法分析器
class Lexer:
    def __init__(self, code):
        self.code = code
        self.pos = 0
        self.tokens = []
        self.token_specs = [
            ('COMMENT', r'//.*'),
            ('NUMBER', r'\d+'),
            ('ID', r'[a-zA-Z_]\w*'),
            ('OP', r'[+\-*/=<>!]=?|=='),
            ('LPAREN', r'\('),
            ('RPAREN', r'\)'),
            ('LBRACE', r'\{'),
            ('RBRACE', r'\}'),
            ('SEMICOLON', r';'),
            ('COMMA', r','),
            ('COLON', r':'),
            ('WS', r'\s+'),
        ]
        self.token_re = '|'.join(f'(?P<{pair[0]}>{pair[1]})' for pair in self.token_specs)
    
    def tokenize(self):
        for match in re.finditer(self.token_re, self.code):
            kind = match.lastgroup
            value = match.group()
            if kind in ('WS', 'COMMENT'):
                continue
            self.tokens.append((kind, value))
        return self.tokens

# 语法分析器
class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0
        self.syntax_rules = [
            ('program', ['statement_list']),
            ('statement_list', ['statement', 'statement_list'], ['statement']),
            ('statement', ['var_decl'], ['assignment'], ['if_stmt'], ['for_stmt'], ['function_def'], ['function_call'], ['print_stmt'], ['input_stmt']),
            ('var_decl', ['int', 'ID', 'SEMICOLON'], ['int', 'ID', '=', 'expression', 'SEMICOLON']),
            ('assignment', ['ID', '=', 'expression', 'SEMICOLON']),
            ('if_stmt', ['if', '(', 'expression', ')', '{', 'statement_list', '}', 'else', '{', 'statement_list', '}'], ['if', '(', 'expression', ')', '{', 'statement_list', '}']),
            ('for_stmt', ['for', '(', 'assignment', 'expression', 'SEMICOLON', 'assignment', ')', '{', 'statement_list', '}']),
            ('function_def', ['def', 'ID', '(', 'param_list', ')', '{', 'statement_list', 'return', 'expression', 'SEMICOLON', '}']),
            ('function_call', ['ID', '(', 'arg_list', ')', 'SEMICOLON']),
            ('print_stmt', ['print', '(', 'expression', ')', 'SEMICOLON']),
            ('input_stmt', ['ID', '=', 'input', '(', ')', 'SEMICOLON']),
            ('param_list', ['int', 'ID', ',', 'param_list'], ['int', 'ID']),
            ('arg_list', ['expression', ',', 'arg_list'], ['expression']),
            ('expression', ['term', '+', 'expression'], ['term', '-', 'expression'], ['term']),
            ('term', ['factor', '*', 'term'], ['factor', '/', 'term'], ['factor']),
            ('factor', ['NUMBER'], ['ID'], ['(', 'expression', ')']),
        ]
    
    def parse(self):
        try:
            return self.parse_program()
        except Exception as e:
            raise SyntaxError(f"Syntax error at token {self.tokens[self.pos] if self.pos < len(self.tokens) else 'EOF'}: {e}")
    
    def parse_program(self):
        return {'type': 'program', 'body': self.parse_statement_list()}
    
    def parse_statement_list(self):
        statements = []
        while self.pos < len(self.tokens) and self.tokens[self.pos][1] not in ('return', '}'):
            statements.append(self.parse_statement())
        return statements
    
    def parse_statement(self):
        if self.pos >= len(self.tokens):
            raise SyntaxError("Unexpected end of input")
        
        token_type = self.tokens[self.pos][0]
        token_value = self.tokens[self.pos][1]
        
        if token_type == 'ID':
            if token_value == 'int':
                self.pos += 1
                return self.parse_var_decl()
            elif token_value == 'if':
                self.pos += 1
                return self.parse_if_stmt()
            elif token_value == 'for':
                self.pos += 1
                return self.parse_for_stmt()
            elif token_value == 'def':
                self.pos += 1
                func_name = self.tokens[self.pos][1]
                self.pos += 1
                self.expect('LPAREN')
                params = self.parse_param_list()
                self.expect('RPAREN')
                self.expect('LBRACE')
                body = []
                while self.pos < len(self.tokens) and self.tokens[self.pos][1] != 'return' and self.tokens[self.pos][1] != '}':
                    body.append(self.parse_statement())
                return_expr = None
                if self.pos < len(self.tokens) and self.tokens[self.pos][1] == 'return':
                    self.pos += 1
                    return_expr = self.parse_expression()
                    self.expect('SEMICOLON')
                self.expect('RBRACE')
                return {'type': 'function_def', 'name': func_name, 'params': params, 'body': body, 'return_expr': return_expr}
            elif token_value == 'print':
                self.pos += 1
                self.expect('LPAREN')
                expr = self.parse_expression()
                self.expect('RPAREN')
                self.expect('SEMICOLON')
                return {'type': 'print_stmt', 'expr': expr}
            elif token_value == 'input':
                self.pos += 1
                self.expect('LPAREN')
                self.expect('RPAREN')
                return {'type': 'input_expr'}
            elif self.pos + 1 < len(self.tokens) and self.tokens[self.pos + 1][1] == '=':
                # 赋值语句
                var_name = token_value
                self.pos += 1
                self.expect('OP', '=')
                value = self.parse_expression()
                self.expect('SEMICOLON')
                return {'type': 'assignment', 'name': var_name, 'value': value}
            elif self.pos + 1 < len(self.tokens) and self.tokens[self.pos + 1][1] == '(':
                # 函数调用
                func_name = token_value
                self.pos += 1
                self.expect('LPAREN')
                args = []
                if self.pos < len(self.tokens) and self.tokens[self.pos][1] != ')':
                    args.append(self.parse_expression())
                    while self.pos < len(self.tokens) and self.tokens[self.pos][1] == ',':
                        self.pos += 1
                        args.append(self.parse_expression())
                self.expect('RPAREN')
                self.expect('SEMICOLON')
                return {'type': 'function_call', 'name': func_name, 'args': args}
            else:
                raise SyntaxError(f"Unexpected token: {token_value}")
        else:
            raise SyntaxError(f"Unexpected token: {self.tokens[self.pos]}")
    
    def parse_var_decl(self):
        var_name = self.tokens[self.pos][1]
        self.pos += 1
        if self.pos < len(self.tokens) and self.tokens[self.pos][1] == '=':
            self.pos += 1
            value = self.parse_expression()
        else:
            value = None
        self.expect('SEMICOLON')
        return {'type': 'var_decl', 'name': var_name, 'value': value}
    
    def parse_assignment(self):
        var_name = self.tokens[self.pos][1]
        self.pos += 1
        self.expect('OP', '=')
        value = self.parse_expression()
        self.expect('SEMICOLON')
        return {'type': 'assignment', 'name': var_name, 'value': value}
    
    def parse_if_stmt(self):
        self.expect('LPAREN')
        condition = self.parse_expression()
        self.expect('RPAREN')
        self.expect('LBRACE')
        then_body = self.parse_statement_list()
        self.expect('RBRACE')
        else_body = None
        if self.pos < len(self.tokens) and self.tokens[self.pos][1] == 'else':
            self.pos += 1
            self.expect('LBRACE')
            else_body = self.parse_statement_list()
            self.expect('RBRACE')
        return {'type': 'if_stmt', 'condition': condition, 'then_body': then_body, 'else_body': else_body}
    
    def parse_for_stmt(self):
        self.expect('LPAREN')
        # 解析初始化语句
        init = self.parse_initialization()
        # 解析条件表达式
        condition = self.parse_condition()
        # 解析增量语句
        increment = self.parse_increment()
        self.expect('RPAREN')
        self.expect('LBRACE')
        body = self.parse_statement_list()
        self.expect('RBRACE')
        return {'type': 'for_stmt', 'init': init, 'condition': condition, 'increment': increment, 'body': body}
    
    def parse_initialization(self):
        if self.tokens[self.pos][1] == 'int':
            self.pos += 1
            var_name = self.tokens[self.pos][1]
            self.pos += 1
            if self.pos < len(self.tokens) and self.tokens[self.pos][1] == '=':
                self.pos += 1
                value = self.parse_expression()
            else:
                value = None
            if self.pos < len(self.tokens) and self.tokens[self.pos][1] == ';':
                self.pos += 1
            return {'type': 'var_decl', 'name': var_name, 'value': value}
        else:
            var_name = self.tokens[self.pos][1]
            self.pos += 1
            self.expect('OP', '=')
            value = self.parse_expression()
            if self.pos < len(self.tokens) and self.tokens[self.pos][1] == ';':
                self.pos += 1
            return {'type': 'assignment', 'name': var_name, 'value': value}
    
    def parse_condition(self):
        left = self.parse_term()
        while self.pos < len(self.tokens) and self.tokens[self.pos][1] in ('<', '>', '<=', '>=', '==', '!='):
            op = self.tokens[self.pos][1]
            self.pos += 1
            right = self.parse_term()
            left = {'type': 'binary_op', 'op': op, 'left': left, 'right': right}
        if self.pos < len(self.tokens) and self.tokens[self.pos][1] == ';':
            self.pos += 1
        return left
    
    def parse_increment(self):
        var_name = self.tokens[self.pos][1]
        self.pos += 1
        self.expect('OP', '=')
        value = self.parse_expression()
        return {'type': 'assignment', 'name': var_name, 'value': value}
    
    def parse_function_def(self):
        func_name = self.tokens[self.pos][1]
        self.pos += 1
        self.expect('LPAREN')
        params = self.parse_param_list()
        self.expect('RPAREN')
        self.expect('LBRACE')
        body = self.parse_statement_list()
        if self.pos < len(self.tokens) and self.tokens[self.pos][1] == 'return':
            self.pos += 1  # skip return
            return_expr = self.parse_expression()
            self.expect('SEMICOLON')
        else:
            return_expr = None
        self.expect('RBRACE')
        return {'type': 'function_def', 'name': func_name, 'params': params, 'body': body, 'return_expr': return_expr}
    
    def parse_function_call(self):
        func_name = self.tokens[self.pos][1]
        self.pos += 1
        self.expect('LPAREN')
        args = self.parse_arg_list()
        self.expect('RPAREN')
        self.expect('SEMICOLON')
        return {'type': 'function_call', 'name': func_name, 'args': args}
    
    def parse_print_stmt(self):
        self.expect('LPAREN')
        expr = self.parse_expression()
        self.expect('RPAREN')
        self.expect('SEMICOLON')
        return {'type': 'print_stmt', 'expr': expr}
    
    def parse_input_stmt(self):
        var_name = self.tokens[self.pos][1]
        self.pos += 1
        self.expect('OP', '=')
        self.pos += 1  # skip input
        self.expect('LPAREN')
        self.expect('RPAREN')
        self.expect('SEMICOLON')
        return {'type': 'input_stmt', 'name': var_name}
    
    def parse_param_list(self):
        params = []
        while self.pos < len(self.tokens) and self.tokens[self.pos][1] == 'int':
            self.pos += 1  # skip int
            params.append(self.tokens[self.pos][1])
            self.pos += 1
            if self.pos < len(self.tokens) and self.tokens[self.pos][1] == ',':
                self.pos += 1
        return params
    
    def parse_arg_list(self):
        args = []
        if self.pos < len(self.tokens) and self.tokens[self.pos][1] != ')':
            args.append(self.parse_expression())
            while self.pos < len(self.tokens) and self.tokens[self.pos][1] == ',':
                self.pos += 1
                args.append(self.parse_expression())
        return args
    
    def parse_expression(self):
        left = self.parse_term()
        while self.pos < len(self.tokens) and self.tokens[self.pos][1] in ('+', '-', '==', '!=', '<', '>', '<=', '>=') and self.tokens[self.pos][1] != ';':
            op = self.tokens[self.pos][1]
            self.pos += 1
            right = self.parse_term()
            left = {'type': 'binary_op', 'op': op, 'left': left, 'right': right}
        return left
    
    def parse_term(self):
        left = self.parse_factor()
        while self.pos < len(self.tokens) and self.tokens[self.pos][1] in ('*', '/'):
            op = self.tokens[self.pos][1]
            self.pos += 1
            right = self.parse_factor()
            left = {'type': 'binary_op', 'op': op, 'left': left, 'right': right}
        return left
    
    def parse_factor(self):
        if self.tokens[self.pos][0] == 'NUMBER':
            value = int(self.tokens[self.pos][1])
            self.pos += 1
            return {'type': 'number', 'value': value}
        elif self.tokens[self.pos][0] == 'ID':
            var_name = self.tokens[self.pos][1]
            self.pos += 1
            if self.pos < len(self.tokens) and self.tokens[self.pos][1] == '(':
                # 函数调用表达式
                self.pos += 1
                args = []
                if self.pos < len(self.tokens) and self.tokens[self.pos][1] != ')':
                    args.append(self.parse_expression())
                    while self.pos < len(self.tokens) and self.tokens[self.pos][1] == ',':
                        self.pos += 1
                        args.append(self.parse_expression())
                self.expect('RPAREN')
                return {'type': 'function_call_expr', 'name': var_name, 'args': args}
            else:
                return {'type': 'identifier', 'name': var_name}
        elif self.tokens[self.pos][1] == '(':
            self.pos += 1
            expr = self.parse_expression()
            self.expect('RPAREN')
            return expr
        else:
            raise SyntaxError(f"Unexpected token: {self.tokens[self.pos]}")
    
    def match(self, token):
        if self.pos < len(self.tokens) and self.tokens[self.pos][1] == token:
            self.pos += 1
            return True
        return False
    
    def expect(self, token_type, token_value=None):
        if self.pos >= len(self.tokens):
            raise SyntaxError(f"Expected {token_type}, got EOF")
        if token_value:
            if self.tokens[self.pos][1] != token_value:
                raise SyntaxError(f"Expected {token_value}, got {self.tokens[self.pos][1]}")
        else:
            if self.tokens[self.pos][0] != token_type:
                raise SyntaxError(f"Expected {token_type}, got {self.tokens[self.pos][0]}")
        self.pos += 1

# 解释器核心
class Interpreter:
    def __init__(self):
        self.globals = {}
        self.functions = {}
    
    def run(self, code):
        try:
            lexer = Lexer(code)
            tokens = lexer.tokenize()
            parser = Parser(tokens)
            ast = parser.parse()
            self.interpret(ast, self.globals)
        except Exception as e:
            print(f"Error: {e}")
    
    def interpret(self, node, scope):
        if node['type'] == 'program':
            for stmt in node['body']:
                self.interpret(stmt, scope)
        elif node['type'] == 'var_decl':
            if node['value']:
                scope[node['name']] = self.evaluate(node['value'], scope)
            else:
                scope[node['name']] = 0
        elif node['type'] == 'assignment':
            scope[node['name']] = self.evaluate(node['value'], scope)
        elif node['type'] == 'if_stmt':
            if self.evaluate(node['condition'], scope):
                for stmt in node['then_body']:
                    self.interpret(stmt, scope)
            elif node['else_body']:
                for stmt in node['else_body']:
                    self.interpret(stmt, scope)
        elif node['type'] == 'for_stmt':
            self.interpret(node['init'], scope)
            while self.evaluate(node['condition'], scope):
                for stmt in node['body']:
                    self.interpret(stmt, scope)
                self.interpret(node['increment'], scope)
        elif node['type'] == 'function_def':
            self.functions[node['name']] = node
        elif node['type'] == 'function_call':
            if node['name'] not in self.functions:
                raise NameError(f"Function not defined: {node['name']}")
            func = self.functions[node['name']]
            if len(node['args']) != len(func['params']):
                raise TypeError(f"Expected {len(func['params'])} arguments, got {len(node['args'])}")
            local_scope = {}
            for param, arg in zip(func['params'], node['args']):
                local_scope[param] = self.evaluate(arg, scope)
            for stmt in func['body']:
                self.interpret(stmt, local_scope)
            return self.evaluate(func['return_expr'], local_scope)
        elif node['type'] == 'print_stmt':
            value = self.evaluate(node['expr'], scope)
            print(value)
        elif node['type'] == 'input_stmt':
            try:
                value = int(input())
                scope[node['name']] = value
            except ValueError:
                raise RuntimeError("Input must be an integer")
    
    def evaluate(self, node, scope):
        if node['type'] == 'number':
            return node['value']
        elif node['type'] == 'identifier':
            if node['name'] in scope:
                return scope[node['name']]
            elif node['name'] in self.globals:
                return self.globals[node['name']]
            else:
                raise NameError(f"Variable not defined: {node['name']}")
        elif node['type'] == 'function_call_expr':
            if node['name'] == 'input':
                # 处理内置 input() 函数
                try:
                    return int(input())
                except ValueError:
                    raise RuntimeError("Input must be an integer")
            elif node['name'] not in self.functions:
                raise NameError(f"Function not defined: {node['name']}")
            func = self.functions[node['name']]
            if len(node['args']) != len(func['params']):
                raise TypeError(f"Expected {len(func['params'])} arguments, got {len(node['args'])}")
            local_scope = {}
            for param, arg in zip(func['params'], node['args']):
                local_scope[param] = self.evaluate(arg, scope)
            for stmt in func['body']:
                self.interpret(stmt, local_scope)
            return self.evaluate(func['return_expr'], local_scope)
        elif node['type'] == 'binary_op':
            left_val = self.evaluate(node['left'], scope)
            right_val = self.evaluate(node['right'], scope)
            if node['op'] == '+':
                return left_val + right_val
            elif node['op'] == '-':
                return left_val - right_val
            elif node['op'] == '*':
                return left_val * right_val
            elif node['op'] == '/':
                if right_val == 0:
                    raise ZeroDivisionError("Division by zero")
                return left_val // right_val
            elif node['op'] == '==':
                return left_val == right_val
            elif node['op'] == '!=':
                return left_val != right_val
            elif node['op'] == '<':
                return left_val < right_val
            elif node['op'] == '>':
                return left_val > right_val
            elif node['op'] == '<=':
                return left_val <= right_val
            elif node['op'] == '>=':
                return left_val >= right_val
        else:
            raise RuntimeError(f"Unknown expression type: {node['type']}")

# 测试用例
def test_basic_operations():
    code = """
    int a = 10;
    int b = 5;
    int c = a + b;
    print(c);
    int d = a * b;
    print(d);
    int e = a / b;
    print(e);
    """
    print("Test 1: Basic Operations")
    interpreter = Interpreter()
    interpreter.run(code)
    print()

def test_control_flow():
    code = """
    int x = 10;
    if (x > 5) {
        print(1);
    } else {
        print(0);
    }
    for (int i = 0; i < 5; i = i + 1) {
        print(i);
    }
    """
    print("Test 2: Control Flow")
    interpreter = Interpreter()
    interpreter.run(code)
    print()

def test_functions():
    code = """
    def add(int a, int b) {
        int sum = a + b;
        return sum;
    }
    int result = add(5, 3);
    print(result);
    """
    print("Test 3: Functions")
    interpreter = Interpreter()
    interpreter.run(code)
    print()

def run_file(file_path):
    """运行指定路径的.c文件"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            code = f.read()
        interpreter = Interpreter()
        interpreter.run(code)
    except Exception as e:
        print(f"Error running file {file_path}: {e}")

if __name__ == "__main__":
    import sys
    if len(sys.argv) > 1:
        # 运行指定的.c文件
        run_file(sys.argv[1])
    else:
        # 运行测试用例
        test_basic_operations()
        test_control_flow()
        test_functions()
