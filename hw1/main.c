#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// Original Node structure
typedef struct Node {
    char c;
    struct Node *next;
} Node;

Node *head = NULL;
Node *current = NULL;

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

void print_token(Token token) {
    switch(token) {
        case TYPE_TOKEN: printf("int: TYPE_TOKEN\n"); break;
        case MAIN_TOKEN: printf("main: MAIN_TOKEN\n"); break;
        case IF_TOKEN: printf("if: IF_TOKEN\n"); break;
        case ELSE_TOKEN: printf("else: ELSE_TOKEN\n"); break;
        case WHILE_TOKEN: printf("while: WHILE_TOKEN\n"); break;
        case ID_TOKEN: printf("%s: ID_TOKEN\n", token_buffer); break;
        case LITERAL_TOKEN: printf("%s: LITERAL_TOKEN\n", token_buffer); break;
        case LEFTPAREN_TOKEN: printf("(: LEFTPAREN_TOKEN\n"); break;
        case RIGHTPAREN_TOKEN: printf("): RIGHTPAREN_TOKEN\n"); break;
        case LEFTBRACE_TOKEN: printf("{: LEFTBRACE_TOKEN\n"); break;
        case RIGHTBRACE_TOKEN: printf("}: RIGHTBRACE_TOKEN\n"); break;
        case SEMICOLON_TOKEN: printf(";: SEMICOLON_TOKEN\n"); break;
        case ASSIGN_TOKEN: printf("=: ASSIGN_TOKEN\n"); break;
        case EQUAL_TOKEN: printf("==: EQUAL_TOKEN\n"); break;
        case GREATEREQUAL_TOKEN: printf(">=: GREATEREQUAL_TOKEN\n"); break;
        case LESSEQUAL_TOKEN: printf("<=: LESSEQUAL_TOKEN\n"); break;
        case GREATER_TOKEN: printf(">: GREATER_TOKEN\n"); break;
        case LESS_TOKEN: printf("<: LESS_TOKEN\n"); break;
        case PLUS_TOKEN: printf("+: PLUS_TOKEN\n"); break;
        case MINUS_TOKEN: printf("-: MINUS_TOKEN\n"); break;
        default: printf("UNKNOWN: UNKNOWN_TOKEN\n"); break;
    }
}

int main(void) {
    FILE *fp = fopen("cj.c", "r");
    if (!fp) {
        perror("File opening failed");
        return EXIT_FAILURE;
    }

    int ch;
    Node *tail = NULL;
    while ((ch = fgetc(fp)) != EOF) {
        Node *new_node = malloc(sizeof(Node));
        new_node->c = ch;
        new_node->next = NULL;
        if (!head) head = tail = new_node;
        else tail = tail->next = new_node;
    }
    fclose(fp);

    current = head;
    Token token;
    do {
        token = scanner();
        if (token != END_TOKEN)
            print_token(token);
    } while (token != END_TOKEN);

    return 0;
}
