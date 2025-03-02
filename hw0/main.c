#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 256

typedef struct Node {
    char c;
    struct Node *next;
} Node;

typedef struct HashEntry {
    char key;
    int count;
    struct HashEntry *next;
} HashEntry;

unsigned int hash_function(char key) {
    return ((unsigned char)key) % TABLE_SIZE;
}

void hash_table_insert(HashEntry **table, char key) {
    unsigned int index = hash_function(key);
    HashEntry *entry = table[index];

    while (entry != NULL) {
        if (entry->key == key) {
            entry->count++;
            return;
        }
        entry = entry->next;
    }

    entry = (HashEntry *)malloc(sizeof(HashEntry));
    if (!entry) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    entry->key = key;
    entry->count = 1;
    entry->next = table[index];
    table[index] = entry;
}

void free_hash_table(HashEntry **table) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        HashEntry *entry = table[i];
        while (entry != NULL) {
            HashEntry *temp = entry;
            entry = entry->next;
            free(temp);
        }
    }
}

int main(void) {
    FILE *fp = fopen("main.c", "r");
    if (fp == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }
    
    Node *head = NULL;
    Node *tail = NULL;
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        Node *new_node = (Node *)malloc(sizeof(Node));
        if (!new_node) {
            perror("Memory allocation failed");
            return EXIT_FAILURE;
        }
        new_node->c = (char)ch;
        new_node->next = NULL;
        if (head == NULL) {
            head = new_node;
            tail = new_node;
        } else {
            tail->next = new_node;
            tail = new_node;
        }
    }
    fclose(fp);
    
    HashEntry *hash_table[TABLE_SIZE] = {0};
    Node *current = head;
    while (current != NULL) {
        hash_table_insert(hash_table, current->c);
        current = current->next;
    }
    
    int printed[256] = {0};
    current = head;
    while (current != NULL) {
        unsigned char key = (unsigned char)current->c;
        if (!printed[key]) {
            unsigned int index = hash_function(current->c);
            HashEntry *entry = hash_table[index];
            while (entry != NULL) {
                if (entry->key == current->c) {
                    if (key >= 32 && key <= 126)
                        printf("%c : %d\n", entry->key, entry->count);
                    else
                        printf("0x%02X : %d\n", key, entry->count);
                    break;
                }
                entry = entry->next;
            }
            printed[key] = 1;
        }
        current = current->next;
    }
    
    current = head;
    while (current != NULL) {
        Node *temp = current;
        current = current->next;
        free(temp);
    }
    free_hash_table(hash_table);
    
    return EXIT_SUCCESS;
}
