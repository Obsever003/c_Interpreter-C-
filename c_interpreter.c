#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


// 标记类型
typedef enum {
    NUMBER,
    ID,
    OP,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    SEMICOLON,
    COMMA,
    INT,
    IF,
    ELSE,
    FOR,
    DEF,
    PRINT,
    INPUT,
    RETURN,
    END
} TokenType;

// 标记结构体
typedef struct {
    TokenType type;
    char value[256];
} Token;

// 抽象语法树节点类型
typedef enum {
    NODE_PROGRAM,
    NODE_VAR_DECL,
    NODE_ASSIGNMENT,
    NODE_IF_STMT,
    NODE_FOR_STMT,
    NODE_FUNCTION_DEF,
    NODE_FUNCTION_CALL,
    NODE_FUNCTION_CALL_EXPR,
    NODE_PRINT_STMT,
    NODE_INPUT_EXPR,
    NODE_BINARY_OP,
    NODE_NUMBER,
    NODE_IDENTIFIER,
    NODE_RETURN_STMT
} NodeType;

// 抽象语法树节点结构体
typedef struct Node {
    NodeType type;
    char name[256];
    int value;
    struct Node *left;
    struct Node *right;
    struct Node *body;
    struct Node *else_body;
    struct Node *next;
    struct Node *params;
    struct Node *args;
    struct Node *return_expr;
} Node;

// 函数结构体
typedef struct {
    char name[256];
    Node *params;
    Node *body;
    Node *return_expr;
} Function;

// 作用域结构体
typedef struct Scope {
    char variables[100][256];
    int values[100];
    int count;
    struct Scope *parent;
} Scope;

// 全局变量
Token tokens[1000];
int token_count = 0;
int current_token = 0;
Function functions[100];
int function_count = 0;
Scope global_scope = {{}, {}, 0, NULL};

// 词法分析器
void tokenize(const char *code) {
    int i = 0;
    int j = 0;
    char current_char;
    
    while ((current_char = code[i]) != '\0') {
        // 跳过空白字符
        if (isspace(current_char)) {
            i++;
            continue;
        }
        
        // 处理注释
        if (current_char == '/' && code[i+1] == '/') {
            while (code[i] != '\n' && code[i] != '\0') {
                i++;
            }
            continue;
        }
        
        // 处理数字
        if (isdigit(current_char)) {
            j = 0;
            while (isdigit(code[i])) {
                tokens[token_count].value[j++] = code[i++];
            }
            tokens[token_count].value[j] = '\0';
            tokens[token_count].type = NUMBER;
            token_count++;
            continue;
        }
        
        // 处理标识符和关键字
        if (isalpha(current_char) || current_char == '_') {
            j = 0;
            while (isalnum(code[i]) || code[i] == '_') {
                tokens[token_count].value[j++] = code[i++];
            }
            tokens[token_count].value[j] = '\0';
            
            // 检查是否是关键字
            if (strcmp(tokens[token_count].value, "int") == 0) {
                tokens[token_count].type = INT;
            } else if (strcmp(tokens[token_count].value, "if") == 0) {
                tokens[token_count].type = IF;
            } else if (strcmp(tokens[token_count].value, "else") == 0) {
                tokens[token_count].type = ELSE;
            } else if (strcmp(tokens[token_count].value, "for") == 0) {
                tokens[token_count].type = FOR;
            } else if (strcmp(tokens[token_count].value, "def") == 0) {
                tokens[token_count].type = DEF;
            } else if (strcmp(tokens[token_count].value, "print") == 0) {
                tokens[token_count].type = PRINT;
            } else if (strcmp(tokens[token_count].value, "input") == 0) {
                tokens[token_count].type = INPUT;
            } else if (strcmp(tokens[token_count].value, "return") == 0) {
                tokens[token_count].type = RETURN;
            } else {
                tokens[token_count].type = ID;
            }
            token_count++;
            continue;
        }
        
        // 处理操作符
        if (strchr("+-*/=<>!", current_char)) {
            j = 0;
            tokens[token_count].value[j++] = current_char;
            
            // 处理复合操作符
            if (current_char == '=' || current_char == '!' || current_char == '<' || current_char == '>') {
                if (code[i+1] == '=') {
                    tokens[token_count].value[j++] = code[++i];
                }
            }
            
            tokens[token_count].value[j] = '\0';
            tokens[token_count].type = OP;
            token_count++;
            i++;
            continue;
        }
        
        // 处理其他标记
        switch (current_char) {
            case '(':
                tokens[token_count].type = LPAREN;
                tokens[token_count].value[0] = '(';
                tokens[token_count].value[1] = '\0';
                token_count++;
                i++;
                break;
            case ')':
                tokens[token_count].type = RPAREN;
                tokens[token_count].value[0] = ')';
                tokens[token_count].value[1] = '\0';
                token_count++;
                i++;
                break;
            case '{':
                tokens[token_count].type = LBRACE;
                tokens[token_count].value[0] = '{';
                tokens[token_count].value[1] = '\0';
                token_count++;
                i++;
                break;
            case '}':
                tokens[token_count].type = RBRACE;
                tokens[token_count].value[0] = '}';
                tokens[token_count].value[1] = '\0';
                token_count++;
                i++;
                break;
            case ';':
                tokens[token_count].type = SEMICOLON;
                tokens[token_count].value[0] = ';';
                tokens[token_count].value[1] = '\0';
                token_count++;
                i++;
                break;
            case ',':
                tokens[token_count].type = COMMA;
                tokens[token_count].value[0] = ',';
                tokens[token_count].value[1] = '\0';
                token_count++;
                i++;
                break;
            default:
                i++;
                break;
        }
    }
    
    // 添加结束标记
    tokens[token_count].type = END;
    tokens[token_count].value[0] = '\0';
    token_count++;
}

// 创建新节点
Node *create_node(NodeType type) {
    Node *node = (Node *)malloc(sizeof(Node));
    node->type = type;
    node->name[0] = '\0';
    node->value = 0;
    node->left = NULL;
    node->right = NULL;
    node->body = NULL;
    node->else_body = NULL;
    node->next = NULL;
    node->params = NULL;
    node->args = NULL;
    node->return_expr = NULL;
    return node;
}

// 解析表达式
Node *parse_expression();

// 解释语句
void interpret(Node *node, Scope *scope);



// 解析因子
Node *parse_factor() {
    Node *node = NULL;
    
    if (tokens[current_token].type == NUMBER) {
        node = create_node(NODE_NUMBER);
        node->value = atoi(tokens[current_token].value);
        current_token++;
    } else if (tokens[current_token].type == ID) {
        node = create_node(NODE_IDENTIFIER);
        strcpy(node->name, tokens[current_token].value);
        current_token++;
        
        // 检查是否是函数调用表达式
        if (tokens[current_token].type == LPAREN) {
            Node *call_node = create_node(NODE_FUNCTION_CALL_EXPR);
            strcpy(call_node->name, node->name);
            free(node);
            node = call_node;
            
            current_token++;
            if (tokens[current_token].type != RPAREN) {
                node->args = parse_expression();
                while (tokens[current_token].type == COMMA) {
                    current_token++;
                    Node *arg_node = parse_expression();
                    arg_node->next = node->args;
                    node->args = arg_node;
                }
            }
            
            if (tokens[current_token].type != RPAREN) {
                printf("Error: Expected ')'");
                exit(1);
            }
            current_token++;
        }
    } else if (tokens[current_token].type == LPAREN) {
        current_token++;
        node = parse_expression();
        if (tokens[current_token].type != RPAREN) {
            printf("Error: Expected ')'");
            exit(1);
        }
        current_token++;
    } else if (tokens[current_token].type == INPUT) {
        node = create_node(NODE_INPUT_EXPR);
        current_token++;
        if (tokens[current_token].type != LPAREN) {
            printf("Error: Expected '('");
            exit(1);
        }
        current_token++;
        if (tokens[current_token].type != RPAREN) {
            printf("Error: Expected ')'");
            exit(1);
        }
        current_token++;
    } else {
        printf("Error: Unexpected token");
        exit(1);
    }
    
    return node;
}

// 解析项
Node *parse_term() {
    Node *node = parse_factor();
    
    while (tokens[current_token].type == OP && 
           (strcmp(tokens[current_token].value, "*") == 0 || 
            strcmp(tokens[current_token].value, "/") == 0)) {
        Node *op_node = create_node(NODE_BINARY_OP);
        strcpy(op_node->name, tokens[current_token].value);
        op_node->left = node;
        current_token++;
        op_node->right = parse_factor();
        node = op_node;
    }
    
    return node;
}

// 解析表达式
Node *parse_expression() {
    Node *node = parse_term();
    
    while (tokens[current_token].type == OP && 
           (strcmp(tokens[current_token].value, "+") == 0 || 
            strcmp(tokens[current_token].value, "-") == 0 || 
            strcmp(tokens[current_token].value, "==") == 0 || 
            strcmp(tokens[current_token].value, "!=") == 0 || 
            strcmp(tokens[current_token].value, "<") == 0 || 
            strcmp(tokens[current_token].value, ">") == 0 || 
            strcmp(tokens[current_token].value, "<=") == 0 || 
            strcmp(tokens[current_token].value, ">=") == 0)) {
        Node *op_node = create_node(NODE_BINARY_OP);
        strcpy(op_node->name, tokens[current_token].value);
        op_node->left = node;
        current_token++;
        op_node->right = parse_term();
        node = op_node;
    }
    
    return node;
}

// 解析语句列表
Node *parse_statement_list() {
    Node *head = NULL;
    Node *tail = NULL;
    
    while (tokens[current_token].type != END && 
           tokens[current_token].type != RBRACE && 
           tokens[current_token].type != RETURN) {
        Node *stmt = NULL;
        
        if (tokens[current_token].type == INT) {
            // 变量声明
            current_token++;
            stmt = create_node(NODE_VAR_DECL);
            strcpy(stmt->name, tokens[current_token].value);
            current_token++;
            
            if (tokens[current_token].type == OP && strcmp(tokens[current_token].value, "=") == 0) {
                current_token++;
                stmt->right = parse_expression();
            }
            
            if (tokens[current_token].type != SEMICOLON) {
                printf("Error: Expected ';' at token %d, type %d, value %s\n", current_token, tokens[current_token].type, tokens[current_token].value);
                exit(1);
            }
            current_token++;
        } else if (tokens[current_token].type == ID && tokens[current_token+1].type == OP && strcmp(tokens[current_token+1].value, "=") == 0) {
            // 赋值语句
            stmt = create_node(NODE_ASSIGNMENT);
            strcpy(stmt->name, tokens[current_token].value);
            current_token += 2;
            stmt->right = parse_expression();
            
            if (tokens[current_token].type != SEMICOLON) {
                printf("Error: Expected ';'");
                exit(1);
            }
            current_token++;
        } else if (tokens[current_token].type == IF) {
            // if语句
            current_token++;
            stmt = create_node(NODE_IF_STMT);
            
            if (tokens[current_token].type != LPAREN) {
                printf("Error: Expected '('");
                exit(1);
            }
            current_token++;
            stmt->left = parse_expression();
            
            if (tokens[current_token].type != RPAREN) {
                printf("Error: Expected ')'");
                exit(1);
            }
            current_token++;
            
            if (tokens[current_token].type != LBRACE) {
                printf("Error: Expected '{'");
                exit(1);
            }
            current_token++;
            stmt->body = parse_statement_list();
            
            if (tokens[current_token].type != RBRACE) {
                printf("Error: Expected '}'");
                exit(1);
            }
            current_token++;
            
            if (tokens[current_token].type == ELSE) {
                current_token++;
                if (tokens[current_token].type != LBRACE) {
                    printf("Error: Expected '{'");
                    exit(1);
                }
                current_token++;
                stmt->else_body = parse_statement_list();
                
                if (tokens[current_token].type != RBRACE) {
                    printf("Error: Expected '}'");
                    exit(1);
                }
                current_token++;
            }
        } else if (tokens[current_token].type == FOR) {
            // for语句
            current_token++;
            stmt = create_node(NODE_FOR_STMT);
            
            if (tokens[current_token].type != LPAREN) {
                printf("Error: Expected '('");
                exit(1);
            }
            current_token++;
            
            // 初始化语句
            if (tokens[current_token].type == INT) {
                // 跳过类型声明
                current_token++;
                stmt->left = create_node(NODE_ASSIGNMENT);
                strcpy(stmt->left->name, tokens[current_token].value);
                current_token += 2;
                stmt->left->right = parse_expression();
            } else if (tokens[current_token].type == ID && tokens[current_token+1].type == OP && strcmp(tokens[current_token+1].value, "=") == 0) {
                stmt->left = create_node(NODE_ASSIGNMENT);
                strcpy(stmt->left->name, tokens[current_token].value);
                current_token += 2;
                stmt->left->right = parse_expression();
            }
            
            // 跳过分号
            if (tokens[current_token].type == SEMICOLON) {
                current_token++;
            }
            
            // 条件表达式
            stmt->right = parse_expression();
            
            // 增量语句
            if (tokens[current_token].type == SEMICOLON) {
                current_token++;
                if (tokens[current_token].type == ID && tokens[current_token+1].type == OP && strcmp(tokens[current_token+1].value, "=") == 0) {
                    stmt->body = create_node(NODE_ASSIGNMENT);
                    strcpy(stmt->body->name, tokens[current_token].value);
                    current_token += 2;
                    stmt->body->right = parse_expression();
                }
            }
            
            if (tokens[current_token].type != RPAREN) {
                printf("Error: Expected ')'");
                exit(1);
            }
            current_token++;
            
            if (tokens[current_token].type != LBRACE) {
                printf("Error: Expected '{'");
                exit(1);
            }
            current_token++;
            stmt->else_body = parse_statement_list();
            
            if (tokens[current_token].type != RBRACE) {
                printf("Error: Expected '}'");
                exit(1);
            }
            current_token++;
        } else if (tokens[current_token].type == DEF) {
            // 函数定义
            current_token++;
            stmt = create_node(NODE_FUNCTION_DEF);
            strcpy(stmt->name, tokens[current_token].value);
            current_token++;
            
            if (tokens[current_token].type != LPAREN) {
                printf("Error: Expected '('");
                exit(1);
            }
            current_token++;
            
            // 解析参数
            if (tokens[current_token].type != RPAREN) {
                // 跳过类型声明
                if (tokens[current_token].type == INT) {
                    current_token++;
                }
                stmt->params = create_node(NODE_IDENTIFIER);
                strcpy(stmt->params->name, tokens[current_token].value);
                current_token++;
                
                while (tokens[current_token].type == COMMA) {
                    current_token++;
                    // 跳过类型声明
                    if (tokens[current_token].type == INT) {
                        current_token++;
                    }
                    Node *param = create_node(NODE_IDENTIFIER);
                    strcpy(param->name, tokens[current_token].value);
                    param->next = stmt->params;
                    stmt->params = param;
                    current_token++;
                }
            }
            
            if (tokens[current_token].type != RPAREN) {
                printf("Error: Expected ')'");
                exit(1);
            }
            current_token++;
            
            if (tokens[current_token].type != LBRACE) {
                printf("Error: Expected '{'");
                exit(1);
            }
            current_token++;
            
            // 解析函数体
            stmt->body = parse_statement_list();
            
            // 解析返回语句
            if (tokens[current_token].type == RETURN) {
                current_token++;
                stmt->return_expr = parse_expression();
                
                if (tokens[current_token].type != SEMICOLON) {
                    printf("Error: Expected ';'");
                    exit(1);
                }
                current_token++;
            }
            
            if (tokens[current_token].type != RBRACE) {
                printf("Error: Expected '}'");
                exit(1);
            }
            current_token++;
            
            // 添加函数到函数列表
            strcpy(functions[function_count].name, stmt->name);
            functions[function_count].params = stmt->params;
            functions[function_count].body = stmt->body;
            functions[function_count].return_expr = stmt->return_expr;
            function_count++;
        } else if (tokens[current_token].type == PRINT) {
            // print语句
            current_token++;
            stmt = create_node(NODE_PRINT_STMT);
            
            if (tokens[current_token].type != LPAREN) {
                printf("Error: Expected '('");
                exit(1);
            }
            current_token++;
            stmt->left = parse_expression();
            
            if (tokens[current_token].type != RPAREN) {
                printf("Error: Expected ')'");
                exit(1);
            }
            current_token++;
            
            if (tokens[current_token].type != SEMICOLON) {
                printf("Error: Expected ';'");
                exit(1);
            }
            current_token++;
        } else if (tokens[current_token].type == ID && tokens[current_token+1].type == LPAREN) {
            // 函数调用
            stmt = create_node(NODE_FUNCTION_CALL);
            strcpy(stmt->name, tokens[current_token].value);
            current_token++;
            
            if (tokens[current_token].type != LPAREN) {
                printf("Error: Expected '('");
                exit(1);
            }
            current_token++;
            
            if (tokens[current_token].type != RPAREN) {
                stmt->args = parse_expression();
                while (tokens[current_token].type == COMMA) {
                    current_token++;
                    Node *arg = create_node(NODE_IDENTIFIER);
                    arg->next = stmt->args;
                    stmt->args = parse_expression();
                }
            }
            
            if (tokens[current_token].type != RPAREN) {
                printf("Error: Expected ')'");
                exit(1);
            }
            current_token++;
            
            if (tokens[current_token].type != SEMICOLON) {
                printf("Error: Expected ';'");
                exit(1);
            }
            current_token++;
        } else {
            printf("Error: Unexpected token");
            exit(1);
        }
        
        if (head == NULL) {
            head = stmt;
            tail = stmt;
        } else {
            tail->next = stmt;
            tail = stmt;
        }
    }
    
    return head;
}

// 解析程序
Node *parse_program() {
    Node *program = create_node(NODE_PROGRAM);
    program->body = parse_statement_list();
    return program;
}

// 在作用域中查找变量
int find_variable(Scope *scope, const char *name) {
    for (int i = 0; i < scope->count; i++) {
        if (strcmp(scope->variables[i], name) == 0) {
            return scope->values[i];
        }
    }
    
    if (scope->parent) {
        return find_variable(scope->parent, name);
    }
    
    printf("Error: Variable not defined: %s", name);
    exit(1);
}

// 在作用域中设置变量
void set_variable(Scope *scope, const char *name, int value) {
    for (int i = 0; i < scope->count; i++) {
        if (strcmp(scope->variables[i], name) == 0) {
            scope->values[i] = value;
            return;
        }
    }
    
    if (scope->count < 100) {
        strcpy(scope->variables[scope->count], name);
        scope->values[scope->count] = value;
        scope->count++;
    }
}

// 查找函数
Function *find_function(const char *name) {
    for (int i = 0; i < function_count; i++) {
        if (strcmp(functions[i].name, name) == 0) {
            return &functions[i];
        }
    }
    
    printf("Error: Function not defined: %s", name);
    exit(1);
}

// 计算表达式
int evaluate(Node *node, Scope *scope) {
    if (node == NULL) {
        return 0;
    }
    
    switch (node->type) {
        case NODE_NUMBER:
            return node->value;
        case NODE_IDENTIFIER:
            return find_variable(scope, node->name);
        case NODE_BINARY_OP:
            {
                int left = evaluate(node->left, scope);
                int right = evaluate(node->right, scope);
                
                if (strcmp(node->name, "+") == 0) {
                    return left + right;
                } else if (strcmp(node->name, "-") == 0) {
                    return left - right;
                } else if (strcmp(node->name, "*") == 0) {
                    return left * right;
                } else if (strcmp(node->name, "/") == 0) {
                    if (right == 0) {
                        printf("Error: Division by zero");
                        exit(1);
                    }
                    return left / right;
                } else if (strcmp(node->name, "==") == 0) {
                    return left == right;
                } else if (strcmp(node->name, "!=") == 0) {
                    return left != right;
                } else if (strcmp(node->name, "<") == 0) {
                    return left < right;
                } else if (strcmp(node->name, ">") == 0) {
                    return left > right;
                } else if (strcmp(node->name, "<=") == 0) {
                    return left <= right;
                } else if (strcmp(node->name, ">=") == 0) {
                    return left >= right;
                }
            }
        case NODE_FUNCTION_CALL_EXPR:
            {
                Function *func = find_function(node->name);
                Scope local_scope = {{}, {}, 0, scope};
                
                // 绑定参数
                Node *param = func->params;
                Node *arg = node->args;
                while (param != NULL && arg != NULL) {
                    set_variable(&local_scope, param->name, evaluate(arg, scope));
                    param = param->next;
                    arg = arg->next;
                }
                
                // 执行函数体
                Node *stmt = func->body;
                while (stmt != NULL) {
                    interpret(stmt, &local_scope);
                    stmt = stmt->next;
                }
                
                // 返回值
                return evaluate(func->return_expr, &local_scope);
            }
        case NODE_INPUT_EXPR:
            {
                int value;
                printf("Input: ");
                scanf("%d", &value);
                return value;
            }
        default:
            return 0;
    }
}

// 解释语句
void interpret(Node *node, Scope *scope) {
    if (node == NULL) {
        return;
    }
    
    switch (node->type) {
        case NODE_PROGRAM:
            interpret(node->body, scope);
            break;
        case NODE_VAR_DECL:
            if (node->right) {
                set_variable(scope, node->name, evaluate(node->right, scope));
            } else {
                set_variable(scope, node->name, 0);
            }
            break;
        case NODE_ASSIGNMENT:
            set_variable(scope, node->name, evaluate(node->right, scope));
            break;
        case NODE_IF_STMT:
            if (evaluate(node->left, scope)) {
                interpret(node->body, scope);
            } else if (node->else_body) {
                interpret(node->else_body, scope);
            }
            break;
        case NODE_FOR_STMT:
            // 执行初始化语句
            if (node->left) {
                interpret(node->left, scope);
            }
            
            // 执行循环
            while (evaluate(node->right, scope)) {
                interpret(node->else_body, scope);
                if (node->body) {
                    interpret(node->body, scope);
                }
            }
            break;
        case NODE_FUNCTION_CALL:
            {
                Function *func = find_function(node->name);
                Scope local_scope = {{}, {}, 0, scope};
                
                // 绑定参数
                Node *param = func->params;
                Node *arg = node->args;
                while (param != NULL && arg != NULL) {
                    set_variable(&local_scope, param->name, evaluate(arg, scope));
                    param = param->next;
                    arg = arg->next;
                }
                
                // 执行函数体
                Node *stmt = func->body;
                while (stmt != NULL) {
                    interpret(stmt, &local_scope);
                    stmt = stmt->next;
                }
            }
            break;
        case NODE_PRINT_STMT:
            printf("%d\n", evaluate(node->left, scope));
            break;
        case NODE_FUNCTION_DEF:
            // 函数定义已经在解析时添加到函数列表
            break;
        case NODE_RETURN_STMT:
            // 返回语句在函数调用时处理
            break;
        default:
            break;
    }
    
    // 解释下一条语句
    if (node->next) {
        interpret(node->next, scope);
    }
}

// 运行代码
void run_code(const char *code) {
    // 词法分析
    tokenize(code);
    
    // 语法分析
    Node *ast = parse_program();
    
    // 解释执行
    interpret(ast, &global_scope);
}

// 运行文件
void run_file(const char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        printf("Error: Could not open file %s", file_path);
        return;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *code = (char *)malloc(file_size + 1);
    fread(code, 1, file_size, file);
    code[file_size] = '\0';
    
    fclose(file);
    run_code(code);
    free(code);
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        // 运行指定的.c文件
        run_file(argv[1]);
    } else {
        // 测试代码
        const char *test_code = "int a = 10;\n" 
                               "int b = 5;\n" 
                               "int c = a + b;\n" 
                               "print(c);\n" 
                               "def add(int x, int y) {\n" 
                               "    int sum = x + y;\n" 
                               "    return sum;\n" 
                               "}\n" 
                               "int result = add(a, b);\n" 
                               "print(result);\n" 
                               "for (int i = 0; i < 5; i = i + 1) {\n" 
                               "    print(i);\n" 
                               "}\n";
        
        run_code(test_code);
    }
    
    return 0;
}
