#include <stdio.h>
#include <string.h>

#define MAX_TOKEN_LEN 256
#define MAX_TREE_NODES 100

// Token definitions
typedef enum {
    TYPE_TOKEN, MAIN_TOKEN, IF_TOKEN, ELSE_TOKEN, WHILE_TOKEN,
    ID_TOKEN, LITERAL_TOKEN,
    LEFTPAREN_TOKEN, RIGHTPAREN_TOKEN, LEFTBRACE_TOKEN, RIGHTBRACE_TOKEN,
    SEMICOLON_TOKEN, ASSIGN_TOKEN, EQUAL_TOKEN, GREATEREQUAL_TOKEN,
    LESSEQUAL_TOKEN, GREATER_TOKEN, LESS_TOKEN, PLUS_TOKEN, MINUS_TOKEN,
    UNKNOWN_TOKEN, END_TOKEN
} Token;

// 定義節點類型
typedef enum {
    S_NODE, SPRIME_NODE, E_NODE, TERMINAL_NODE
} NodeType;

// 定義語法樹節點
typedef struct TreeNode {
    NodeType type;
    char value[MAX_TOKEN_LEN];
    int child_count;
    struct TreeNode *children[5]; // 最多支援5個子節點
} TreeNode;

char token_buffer[MAX_TOKEN_LEN];
Token current_token;

// 節點結構
typedef struct Node {
    char c;
    struct Node *next;
} Node;

Node *head = NULL;
Node *current = NULL;

// 記憶體管理 - 增加記憶體池大小
#define MEMORY_POOL_SIZE 100000  // 從 10000 增加到 100000
char memory_pool[MEMORY_POOL_SIZE];
int memory_index = 0;

// 簡易記憶體分配函數，替代 malloc
void* my_malloc(int size) {
    if (memory_index + size > MEMORY_POOL_SIZE) {
        printf("記憶體不足！\n");
        return NULL;
    }
    void* ptr = &memory_pool[memory_index];
    memory_index += size;
    return ptr;
}

// 檢查記憶體分配是否成功
void check_memory(void* ptr) {
    if (ptr == NULL) {
        printf("記憶體分配失敗，程式將結束\n");
        return;
    }
}

// 當前的縮進層級，用於輸出
int indent_level = 0;

// 語法樹的根節點
TreeNode *tree_root = NULL;

// 創建新的語法樹節點
TreeNode *create_tree_node(NodeType type, const char *value) {
    TreeNode *node = (TreeNode *)my_malloc(sizeof(TreeNode));
    check_memory(node);
    if (node == NULL) return NULL;
    
    node->type = type;
    strcpy(node->value, value);
    node->child_count = 0;
    return node;
}

// 添加子節點
void add_child(TreeNode *parent, TreeNode *child) {
    if (parent && child && parent->child_count < 5) {
        parent->children[parent->child_count++] = child;
    }
}

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

// 輸出縮進空格
void print_indent() {
    for (int i = 0; i < indent_level; i++) {
        printf("  ");
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
TreeNode *parse_S();
TreeNode *parse_Sprime();
TreeNode *parse_E();

// Parse error handler
void parse_error() {
    printf("Parse error!\n");
    // 不使用 exit(1)，改為返回
    printf("程式將結束。\n");
    // 我們無法在沒有 stdlib.h 的情況下呼叫 exit
    // 因此我們將在 main 中處理這種情況
    current_token = END_TOKEN; // 標記為結束
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

TreeNode *parse_S() {
    // S -> ES'
    TreeNode *node = create_tree_node(S_NODE, "S");
    
    switch (current_token) {
        case LITERAL_TOKEN:  // num
        case LEFTPAREN_TOKEN: // '('
            print_indent();
            printf("S -> E S'\n");
            indent_level++;
            
            TreeNode *e_node = parse_E();
            add_child(node, e_node);
            
            TreeNode *sprime_node = parse_Sprime();
            add_child(node, sprime_node);
            
            indent_level--;
            return node;
        default:
            parse_error();
    }
    return NULL; // 不會到達這裡
}

TreeNode *parse_Sprime() {
    // S' -> +S | ε
    TreeNode *node = create_tree_node(SPRIME_NODE, "S'");
    
    switch (current_token) {
        case PLUS_TOKEN: // '+'
            print_indent();
            printf("S' -> + S\n");
            get_token(); // consume '+'
            
            // 創建 '+' 節點
            TreeNode *plus_node = create_tree_node(TERMINAL_NODE, "+");
            add_child(node, plus_node);
            
            indent_level++;
            TreeNode *s_node = parse_S();
            add_child(node, s_node);
            indent_level--;
            return node;
        case RIGHTPAREN_TOKEN: // ')'
        case END_TOKEN:
            print_indent();
            printf("S' -> ε\n");
            strcpy(node->value, "ε");
            return node; // ε production
        default:
            parse_error();
    }
    return NULL; // 不會到達這裡
}

TreeNode *parse_E() {
    // E -> num | (S)
    TreeNode *node = create_tree_node(E_NODE, "E");
    
    switch (current_token) {
        case LITERAL_TOKEN: // num
            print_indent();
            printf("E -> %s\n", token_buffer); // 輸出數字
            
            // 創建數字節點
            TreeNode *num_node = create_tree_node(TERMINAL_NODE, token_buffer);
            add_child(node, num_node);
            
            get_token(); // consume num
            return node;
        case LEFTPAREN_TOKEN: // '('
            print_indent();
            printf("E -> ( S )\n");
            
            // 創建 '(' 節點
            TreeNode *lparen_node = create_tree_node(TERMINAL_NODE, "(");
            add_child(node, lparen_node);
            
            get_token(); // consume '('
            indent_level++;
            
            TreeNode *s_node = parse_S();
            add_child(node, s_node);
            
            indent_level--;
            if (current_token != RIGHTPAREN_TOKEN) {
                parse_error();
            }
            
            // 創建 ')' 節點
            TreeNode *rparen_node = create_tree_node(TERMINAL_NODE, ")");
            add_child(node, rparen_node);
            
            get_token(); // consume ')'
            return node;
        default:
            parse_error();
    }
    return NULL; // 不會到達這裡
}

// 遞迴顯示語法樹
void print_tree_node(TreeNode *node, int level) {
    if (!node) return;
    
    // 輸出縮進
    for (int i = 0; i < level; i++) {
        printf("  ");
    }
    
    // 根據節點類型輸出
    switch (node->type) {
        case S_NODE:
            printf("S -> E S'\n");
            break;
        case E_NODE:
            if (node->child_count > 0 && node->children[0]->type == TERMINAL_NODE) {
                if (strcmp(node->children[0]->value, "(") == 0) {
                    printf("E -> ( S )\n");
                } else {
                    printf("E -> %s\n", node->children[0]->value);
                }
            }
            break;
        case SPRIME_NODE:
            if (strcmp(node->value, "ε") == 0) {
                printf("S' -> ε\n");
            } else {
                printf("S' -> + S\n");
            }
            break;
        case TERMINAL_NODE:
            // 只輸出括號和加號，不輸出數字
            if (strcmp(node->value, "(") == 0 || strcmp(node->value, ")") == 0 || 
                strcmp(node->value, "+") == 0) {
                printf("%s\n", node->value);
            }
            break;
    }
    
    // 遞迴輸出子節點
    for (int i = 0; i < node->child_count; i++) {
        print_tree_node(node->children[i], level + 1);
    }
}

// 產生輸出樹
void generate_output_tree() {
    printf("\nThe Output tree\n");
    if (tree_root) {
        print_tree_node(tree_root, 0);
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
        Node *new_node = (Node *)my_malloc(sizeof(Node));
        if (new_node == NULL) {
            printf("記憶體分配失敗，程式將結束\n");
            return 1;
        }
        
        new_node->c = expression[i++];
        new_node->next = NULL;
        if (!head) head = tail = new_node;
        else tail = tail->next = new_node;
    }
    
    current = head;
    get_token(); // 獲取第一個token
    
    // 開始剖析
    printf("\n開始剖析表達式...\n");
    printf("\n剖析過程:\n");
    tree_root = parse_S();
    
    if (tree_root == NULL) {
        printf("\n剖析失敗，可能是記憶體不足或表達式有誤。\n");
        return 1;
    }
    
    if (current_token == END_TOKEN) {
        printf("\n剖析成功!\n");
        generate_output_tree();
    } else {
        printf("\n剖析未完全完成，可能有多餘的字符。\n");
    }
    
    // 不需要釋放記憶體，因為我們使用自己的記憶體池
    
    return 0;
} 