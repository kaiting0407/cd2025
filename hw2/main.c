#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_TOKEN_LEN 256

// Token definitions
typedef enum {
    TYPE_TOKEN, MAIN_TOKEN, IF_TOKEN, ELSE_TOKEN, WHILE_TOKEN,
    ID_TOKEN, LITERAL_TOKEN,
    LEFTPAREN_TOKEN, RIGHTPAREN_TOKEN, LEFTBRACE_TOKEN, RIGHTBRACE_TOKEN,
    SEMICOLON_TOKEN, ASSIGN_TOKEN, EQUAL_TOKEN, GREATEREQUAL_TOKEN,
    LESSEQUAL_TOKEN, GREATER_TOKEN, LESS_TOKEN, PLUS_TOKEN, MINUS_TOKEN,
    UNKNOWN_TOKEN, END_TOKEN
} Token;

char token_buffer[MAX_TOKEN_LEN];
Token current_token;

// 節點結構
typedef struct Node {
    char c;
    struct Node *next;
} Node;

Node *head = NULL;
Node *current = NULL;

// 用於縮排顯示的深度
int indent_level = 0;

// Helper functions
int is_alpha(char ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

int is_digit(char ch) {
    return ch >= '0' && ch <= '9';
}

int is_alnum(char ch) {
    return is_alpha(ch) || is_digit(ch);
}

int is_space(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
}

// 顯示縮排空格
void print_indent() {
    for (int i = 0; i < indent_level; i++) {
        printf("  "); // 兩個空格表示一層縮排
    }
}

char get_next_char() {
    if (current == NULL) return EOF;
    char ch = current->c;
    current = current->next;
    return ch;
}

void unget_char() {
    if (current != head) {
        Node *tmp = head;
        while (tmp->next != current) tmp = tmp->next;
        current = tmp;
    }
}

// Check keywords
Token check_keyword(const char *buffer) {
    if (!strcmp(buffer, "int")) return TYPE_TOKEN;
    if (!strcmp(buffer, "main")) return MAIN_TOKEN;
    if (!strcmp(buffer, "if")) return IF_TOKEN;
    if (!strcmp(buffer, "else")) return ELSE_TOKEN;
    if (!strcmp(buffer, "while")) return WHILE_TOKEN;
    return ID_TOKEN;
}

// Scanner logic
Token scanner() {
    int ch;
    while ((ch = get_next_char()) != EOF && is_space(ch));

    if (ch == EOF) return END_TOKEN;

    if (is_alpha(ch)) {
        int idx = 0;
        do {
            token_buffer[idx++] = ch;
        } while ((ch = get_next_char()) != EOF && (is_alnum(ch) || ch == '_'));
        token_buffer[idx] = '\0';
        if (ch != EOF) unget_char();
        return check_keyword(token_buffer);
    }

    if (is_digit(ch)) {
        int idx = 0;
        do {
            token_buffer[idx++] = ch;
        } while ((ch = get_next_char()) != EOF && is_digit(ch));
        token_buffer[idx] = '\0';
        if (ch != EOF) unget_char();
        return LITERAL_TOKEN;
    }

    switch(ch) {
        case '(': return LEFTPAREN_TOKEN;
        case ')': return RIGHTPAREN_TOKEN;
        case '{': return LEFTBRACE_TOKEN;
        case '}': return RIGHTBRACE_TOKEN;
        case ';': return SEMICOLON_TOKEN;
        case '=':
            if ((ch = get_next_char()) == '=') return EQUAL_TOKEN;
            unget_char();
            return ASSIGN_TOKEN;
        case '>':
            if ((ch = get_next_char()) == '=') return GREATEREQUAL_TOKEN;
            unget_char();
            return GREATER_TOKEN;
        case '<':
            if ((ch = get_next_char()) == '=') return LESSEQUAL_TOKEN;
            unget_char();
            return LESS_TOKEN;
        case '+': return PLUS_TOKEN;
        case '-': return MINUS_TOKEN;
        default: return UNKNOWN_TOKEN;
    }
}

// Parser function declarations
void parse_S();
void parse_Sprime();
void parse_E();

// Parse error handler
void parse_error() {
    printf("Parse error!\n");
    exit(1);
}

// Get next token
void get_token() {
    current_token = scanner();
}

// RD Parser implementation
// Grammar:
// S -> ES'
// S'-> ε
// S'-> +S
// E -> num
// E -> (S)

void parse_S() {
    // S -> ES'
    switch (current_token) {
        case LITERAL_TOKEN:  // num
        case LEFTPAREN_TOKEN: // '('
            print_indent();
            printf("S -> E S'\n");
            indent_level++;
            parse_E();
            parse_Sprime();
            indent_level--;
            return;
        default:
            parse_error();
    }
}

void parse_Sprime() {
    // S' -> +S | ε
    switch (current_token) {
        case PLUS_TOKEN: // '+'
            print_indent();
            printf("S' -> + S\n");
            get_token(); // consume '+'
            indent_level++;
            parse_S();
            indent_level--;
            return;
        case RIGHTPAREN_TOKEN: // ')'
        case END_TOKEN:
            print_indent();
            printf("S' -> ε\n");
            return; // ε production
        default:
            parse_error();
    }
}

void parse_E() {
    // E -> num | (S)
    switch (current_token) {
        case LITERAL_TOKEN: // num
            print_indent();
            printf("E -> %s\n", token_buffer); // 輸出數字
            get_token(); // consume num
            return;
        case LEFTPAREN_TOKEN: // '('
            print_indent();
            printf("E -> ( S )\n");
            get_token(); // consume '('
            indent_level++;
            parse_S();
            indent_level--;
            if (current_token != RIGHTPAREN_TOKEN) {
                parse_error();
            }
            get_token(); // consume ')'
            return;
        default:
            parse_error();
    }
}

int main(void) {
    char expression[1000];
    printf("請輸入算術表達式 (例如 1+2 或 (1+2)+3): ");
    fgets(expression, sizeof(expression), stdin);
    
    // 建立連結串列以供掃描器使用
    int i = 0;
    Node *tail = NULL;
    while (expression[i] != '\0' && expression[i] != '\n') {
        Node *new_node = malloc(sizeof(Node));
        new_node->c = expression[i++];
        new_node->next = NULL;
        if (!head) head = tail = new_node;
        else tail = tail->next = new_node;
    }
    
    current = head;
    get_token(); // 獲取第一個token
    
    // 開始剖析
    printf("\n開始剖析表達式...\n");
    printf("\n剖析樹:\n");
    parse_S();
    
    if (current_token == END_TOKEN) {
        printf("\n剖析成功!\n");
    } else {
        printf("\n剖析未完全完成，可能有多餘的字符。\n");
    }
    
    // 釋放連結串列記憶體
    while (head) {
        Node *temp = head;
        head = head->next;
        free(temp);
    }
    
    return 0;
} 