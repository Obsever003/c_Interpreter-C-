// 测试文件：测试C语言解释器的各种功能

// 测试1：基本变量定义和算术运算
int a = 10;
int b = 5;
int c = a + b;
print(c);

int d = a * b;
print(d);

int e = a / b;
print(e);

// 测试2：函数定义和调用
def add(int x, int y) {
    int sum = x + y;
    return sum;
}

int result = add(a, b);
print(result);

// 测试3：if-else条件语句
if (a > b) {
    print(1);
} else {
    print(0);
}

// 测试4：for循环
for (int i = 0; i < 5; i = i + 1) {
    print(i);
}

// 测试5：输入函数
int input_value = input();
print(input_value);
