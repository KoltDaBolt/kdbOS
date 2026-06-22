#include "list.h"
#include "kmemory.h"
#include "kstring.h"

List* list_init(uint32_t* err_check) {
    List* new_list = (List*)kmalloc(sizeof(List));
    if (!new_list) {
        *err_check = 1;
        return NULL;
    }

    new_list->err_msg = (char*)kmalloc(MAX_ERR_LENGTH * sizeof(char));
    if (!new_list->err_msg) {
        kfree(new_list);
        *err_check = 1;
        return NULL;
    }

    new_list->root.next = &new_list->root;
    new_list->root.prev = &new_list->root;
    
    new_list->size = 0;
    new_list->err_code = 0;

    return new_list;
}

static inline void __list_add(ListNode* new_node, ListNode* prev, ListNode* next) {
    next->prev = new_node;
    new_node->next = next;
    new_node->prev = prev;
    prev->next = new_node;
}

uint32_t list_push_head(List* list, ListNode* new_node) {
    if (!list || !new_node) return 1;

    __list_add(new_node, &list->root, list->root.next);
    list->size++;
    return 0;
}

ListNode* list_pop_head(List* list) {
    if (!list || list->size == 0) {
        list->err_code = 1;
        kstrncpy(list->err_msg, "ERROR in list_pop_head: list is empty", MAX_ERR_LENGTH);
        return NULL;
    }

    ListNode* target = list->root.next;
    list_remove_node(list, target);
    return target;
}

uint32_t list_push_tail(List* list, ListNode* new_node) {
    if (!list || !new_node) return 1;
    
    __list_add(new_node, list->root.prev, &list->root);
    list->size++;
    return 0;
}

ListNode* list_pop_tail(List* list) {
    if (!list || list->size == 0) {
        list->err_code = 1;
        kstrncpy(list->err_msg, "ERROR in list_pop_tail: list is empty", MAX_ERR_LENGTH);
        return NULL;
    }

    ListNode* target = list->root.prev;
    list_remove_node(list, target);
    return target;
}

uint32_t list_remove_node(List* list, ListNode* node) {
    if (!list || !node || node == &list->root || list->size == 0) {
        return 1;
    }

    node->prev->next = node->next;
    node->next->prev = node->prev;

    node->next = NULL;
    node->prev = NULL;

    list->size--;
    return 0;
}

uint32_t list_get_size(List* list) {
    return list ? list->size : 0;
}

uint32_t list_get_error_code(List* list) {
    return list ? list->err_code : 1;
}

char* list_get_err_msg(List* list) {
    return list ? list->err_msg : "Invalid list pointer";
}

void list_free(List* list) {
    if (!list) return;

    while (list->size > 0) {
        list_pop_head(list);
    }

    kfree(list->err_msg);
    kfree(list);
}