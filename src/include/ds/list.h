#pragma once

#include "types.h"

#define MAX_ERR_LENGTH 256

#define offset_of(type, member) ((size_t)&((type*)0)->member)

#define container_of(ptr, type, member) ({ \
            const typeof( ((type*)0)->member ) *__mptr = (ptr); \
            (type*)( (char*)__mptr - offset_of(type, member) ); \
        })

typedef struct list_node {
    struct list_node* next;
    struct list_node* prev;
} ListNode;

typedef struct {
    ListNode root;
    uint32_t size;
    uint32_t err_code;
    char* err_msg;
} List;

List* list_init(uint32_t*);
uint32_t list_push_head(List*, ListNode*);
ListNode* list_pop_head(List*);
uint32_t list_push_tail(List*, ListNode*);
ListNode* list_pop_tail(List*);
uint32_t list_remove_node(List*, ListNode*);
uint32_t list_get_size(List*);
uint32_t list_get_error_code(List*);
char* list_get_err_msg(List*);
void list_free(List*);