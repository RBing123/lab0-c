#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if(!head)
        return NULL;
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head) {
    if(!head)
        return;
    element_t *entry, *safe;
    list_for_each_entry_safe(entry, safe, head, list) 
        q_release_element(entry);
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if(!head || !s)
        return false;
    element_t *e = malloc(sizeof(element_t));
    if(!e)
        return false;
    e->value = strdup(s);
    if(!e->value){
        free(e);
        return false;
    }
    list_add(&e->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if(!head || !s)
        return false;
    return q_insert_head(head->prev, s);
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if(!head || list_empty(head))
        return NULL;
    element_t *e = list_first_entry(head, element_t, list);
    list_del(&e->list);
    if(sp && bufsize && e->value){
        strncpy(sp, e->value, bufsize-1);
        sp[bufsize-1]='\0';
    }
    return e;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if(!head || list_empty(head))
        return NULL;
    return q_remove_head(head->prev->prev, sp, bufsize);
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if(!head || list_empty(head))
        return 0;
    struct list_head *node;
    int count=0;
    list_for_each(node, head)
        count++;
    return count;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if(!head || list_empty(head))
        return false;
    struct list_head *left = head->next, *right = head->prev;
    while(left!=right && left->next != right){
        left=left->next;
        right=right->prev;
    }
    list_del(right);
    q_release_element(list_entry(right, element_t, list));
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if(!head || list_empty(head))
        return false;
    bool is_dup = false;
    element_t *entry, *safe;
    list_for_each_entry_safe(entry, safe, head, list){
        if(&safe->list != head && strcmp(entry->value, safe->value)==0){
            list_del(&entry->list);
            q_release_element(entry);
            is_dup = true;
        }
        else if(is_dup){
            list_del(&entry->list);
            q_release_element(entry);
            is_dup=false;
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head))
        return;

    struct list_head **indir = &head->next;
    while (*indir != head && (*indir)->next != head) {
        list_move(*indir, (*indir)->next);
        indir = &(*indir)->next->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head) {
    if (!head || list_empty(head))
        return;

    struct list_head *node, *safe;
    list_for_each_safe(node, safe, head)
        list_move(node, head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (!head || list_empty(head))
        return;

    struct list_head *it, *safe, *cut;
    int count = k;
    cut = head;
    list_for_each_safe(it, safe, head) {
        if (--count)
            continue;
        LIST_HEAD(tmp);
        count = k;
        list_cut_position(&tmp, cut, it);
        q_reverse(&tmp);
        list_splice(&tmp, cut);
        cut = safe->prev;
    }
}

/* Sort elements of queue in ascending/descending order */
bool cmp_func(void *priv,
              struct list_head *left,
              struct list_head *right,
              bool descend)
{
    (*(int *) priv)++;

    char *left_value = list_first_entry(left, element_t, list)->value;
    char *right_value = list_first_entry(right, element_t, list)->value;

    // XOR logic
    return descend ^ (strcmp(left_value, right_value) > 0);
}
int q_merge_two(struct list_head *left, struct list_head *right, bool descend)
{
    if (!left || !right)
        return 0;

    if (list_empty(left) || list_empty(right)) {
        list_splice_init(right, left);
        return q_size(left);
    }

    LIST_HEAD(head);
    int count = 0;
    int priv = 0;

    for (;;) {
        count++;
        if (cmp_func(&priv, left, right, descend)) {
            list_move_tail(right->next, &head);
            if (list_empty(right)) {
                count += q_size(left);
                list_splice_init(&head, left);
                break;
            }
        } else {
            list_move_tail(left->next, &head);
            if (list_empty(left)) {
                count += q_size(right);
                list_splice_init(right, left);
                list_splice_init(&head, left);
                break;
            }
        }
    }

    return count;
}
void q_sort(struct list_head *head, bool descend) {
    if (!head || head->next == head->prev)
        return;

    // find middle point (two-pointer)
    struct list_head *start = head, *end = head;
    do {
        start = start->next;
        end = end->prev;
    } while (start != end && start->next != end);

    // parition (recursive)
    LIST_HEAD(new_head);
    list_cut_position(&new_head, head, start);

    q_sort(head, descend);
    q_sort(&new_head, descend);

    q_merge_two(head, &new_head, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || head->next == head->prev)
        return 0;

    element_t *current = list_last_entry(head, element_t, list);
    struct list_head *it = head->prev->prev;
    int count = 1;

    while (current->list.prev != head) {
        element_t *it_entry = list_entry(it, element_t, list);
        if (strcmp(current->value, it_entry->value) < 0) {
            // remove lesser value
            it = it->prev;
            list_del(it->next);
            q_release_element(it_entry);
        } 
        else {
            // update current
            current = it_entry;
            count++;
            it = it->prev;
        }
    }

    return count;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || head->next == head->prev)
        return 0;

    element_t *current = list_last_entry(head, element_t, list);
    struct list_head *it = head->prev->prev;
    int count = 1;

    while (current->list.prev != head) {
        element_t *it_entry = list_entry(it, element_t, list);
        if (strcmp(current->value, it_entry->value) > 0) {
            // remove lesser value
            it = it->prev;
            list_del(it->next);
            q_release_element(it_entry);
        } else {
            // update current
            current = it_entry;
            count++;
            it = it->prev;
        }
    }

    return count;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head))
        return 0;

    queue_contex_t *first = list_entry(head->next, queue_contex_t, chain);
    if (head->next == head->prev)
        return first->size;

    queue_contex_t *second =
        list_entry(first->chain.next, queue_contex_t, chain);
    int size = 0;
    int n = q_size(head) - 1;

    while (n--) {
        size = q_merge_two(first->q, second->q, descend);
        list_move_tail(&second->chain, head);
        second = list_entry(first->chain.next, queue_contex_t, chain);
    }

    return size;
}
