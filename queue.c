#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

#define q_is_empty(__head) list_empty(__head)

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *new_q = malloc(sizeof(struct list_head));

    if (!new_q)
        goto failed_new_q;

    INIT_LIST_HEAD(new_q);
failed_new_q:
    return new_q;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    element_t *curr_elem;
    struct list_head *curr, *safe;

    if (!head)
        return;

    list_for_each_safe (curr, safe, head) {
        curr_elem = list_entry(curr, typeof(*curr_elem), list);
        list_del(curr);
        free(curr_elem->value);
        free(curr_elem);
    }
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    element_t *new_elem = malloc(sizeof(element_t));
    if (!new_elem)
        goto failed_alloc_elem;

    new_elem->value = strdup(s);
    if (!new_elem->value)
        goto failed_alloc_value;

    list_add(&new_elem->list, head);
    return true;

failed_alloc_value:
    free(new_elem);
failed_alloc_elem:
    return false;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    element_t *new_elem = malloc(sizeof(element_t));
    if (!new_elem)
        goto failed_alloc_elem;

    new_elem->value = strdup(s);
    if (!new_elem->value)
        goto failed_alloc_value;

    list_add_tail(&new_elem->list, head);
    return true;

failed_alloc_value:
    free(new_elem);
failed_alloc_elem:
    return false;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    element_t *rm_elem;

    if (!head || q_is_empty(head))
        return NULL;

    rm_elem = list_entry(head->next, typeof(*rm_elem), list);

    if (sp) {
        strncpy(sp, rm_elem->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    list_del(head->next);
    return rm_elem;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    element_t *rm_elem;

    if (!head || q_is_empty(head))
        return NULL;

    rm_elem = list_entry(head->prev, typeof(*rm_elem), list);

    if (sp) {
        strncpy(sp, rm_elem->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    list_del(head->prev);
    return rm_elem;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    struct list_head *curr;
    int size;

    if (!head || q_is_empty(head))
        return 0;

    size = 0;
    list_for_each (curr, head)
        size += 1;

    return size;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    element_t *del_elem;
    struct list_head *slow, *fast;

    if (!head || q_is_empty(head))
        return false;

    slow = head->next;
    fast = head->next;
    while ((fast->next != head) & (fast->next->next != head)) {
        slow = slow->next;
        fast = fast->next->next;
    }

    list_del(slow);

    del_elem = list_entry(slow, typeof(*del_elem), list);
    free(del_elem->value);
    free(del_elem);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    element_t *curr_elem, *iter_elem, *del_elem;
    struct list_head *curr, *iter, *next;

    if (!head || q_is_empty(head))
        return false;

    list_for_each (curr, head) {
        bool dup = false;
        curr_elem = list_entry(curr, typeof(*curr_elem), list);

        iter = curr->next;
        while (iter != head) {
            iter_elem = list_entry(iter, typeof(*iter_elem), list);
            if (strcmp(iter_elem->value, curr_elem->value))
                break;
            iter = iter->next;
            dup = true;
        }

        if (dup) {
            while (curr != iter) {
                del_elem = list_entry(curr, typeof(*del_elem), list);
                next = curr->next;
                list_del(&del_elem->list);
                free(del_elem->value);
                free(del_elem);
                curr = next;
            }
            curr = curr->prev;
        }
    }

    return true;
}

static void qnode_move_before(struct list_head *node1, struct list_head *node2)
{
    list_del(node1);
    node1->prev = node2->prev;
    node1->next = node2;
    node2->prev->next = node1;
    node2->prev = node1;
}

static void qnode_move_after(struct list_head *node1, struct list_head *node2)
{
    list_del(node1);
    node1->prev = node2;
    node1->next = node2->next;
    node2->next->prev = node1;
    node2->next = node1;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    struct list_head *curr;

    if (!head || q_is_empty(head))
        return;

    list_for_each (curr, head) {
        if (curr->next == head)
            break;
        list_move(curr->next, curr->prev);
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    struct list_head *curr = head, *temp;
    do {
        temp = curr->next;
        curr->next = curr->prev;
        curr->prev = temp;
        curr = curr->prev;
    } while (curr != head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    struct list_head *curr, *iter, *temp, *prev, *new_head, *new_tail;
    int num;
    list_for_each (curr, head) {
        for (iter = curr, num = k; num && iter != head; num--)
            iter = iter->next;

        if (num)
            break;

        prev = curr->prev;
        do {
            temp = curr->next;
            curr->next = curr->prev;
            curr->prev = temp;
            curr = curr->prev;
        } while (curr != iter);
        new_tail = prev->next;
        new_head = curr->prev;
        prev->next = new_head;
        new_head->prev = prev;
        new_tail->next = curr;
        curr->prev = new_tail;
        curr = new_tail;
    }
}

static bool cmp_ascend(const char *str1, const char *str2)
{
    return strcmp(str1, str2) > 0;
}

static bool cmp_descend(const char *str1, const char *str2)
{
    return strcmp(str1, str2) < 0;
}

static int q_sort_interval(struct list_head **head,
                           struct list_head **tail,
                           bool (*cmp)(const char *, const char *))
{
    element_t *l_elem, *r_elem;
    struct list_head *l, *l_tail, *r, *temp;
    int l_num, r_num, ret_num;

    /* Handle the simplest cases. */
    if (*head == *tail)
        return 1;
    else if ((*head)->next == *tail) {
        l_elem = list_entry(*head, typeof(*l_elem), list);
        r_elem = list_entry(*tail, typeof(*r_elem), list);
        if (cmp(l_elem->value, r_elem->value))
            qnode_move_after(*head, *tail);
        return 2;
    }

    /* Split the queue into two sub-queues. */
    l = *head;
    r = *tail;

    while (true) {
        if (l == r)
            break;
        else if (l->next == r)
            break;
        l = l->next;
        r = r->prev;
    }

    if (l == r)
        r = r->next;

    /* Sort two sub-queues respectively. */
    l_num = q_sort_interval(head, &r->prev, cmp);
    l = *head;
    l_tail = r->prev;

    r_num = q_sort_interval(&l_tail->next, tail, cmp);
    r = l_tail->next;

    ret_num = l_num + r_num;

    /* Find the new head for the sorted queue after merging. */
    l_elem = list_entry(l, typeof(*l_elem), list);
    r_elem = list_entry(r, typeof(*r_elem), list);
    if (cmp(l_elem->value, r_elem->value))
        *head = r;

    /* Merge two sub-queues into one queue.*/
    while (l_num || r_num) {
        if (!l_num) {
            while (r_num--) {
                temp = r->next;
                qnode_move_after(r, l_tail);
                l_tail = l_tail->next;
                r = temp;
            }
            break;
        } else if (!r_num) {
            while (l_num--)
                ;
            break;
        }

        l_elem = list_entry(l, typeof(*l_elem), list);
        r_elem = list_entry(r, typeof(*r_elem), list);

        if (cmp(l_elem->value, r_elem->value)) {
            temp = r->next;
            qnode_move_before(r, l);
            if (r_num) {
                r = temp;
                r_num -= 1;
            }
        } else if (l_num) {
            l = l->next;
            l_num -= 1;
        }
    }

    *tail = l_tail;
    return ret_num;
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    bool (*cmp[2])(const char *, const char *) = {cmp_ascend, cmp_descend};

    if (!head || q_is_empty(head))
        return;

    q_sort_interval(&head->next, &head->prev, cmp[descend]);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    element_t *curr_elem, *iter_elem;
    struct list_head *curr, *iter, *safe;
    int size;

    if (!head || q_is_empty(head))
        return 0;

    size = 0;
    list_for_each (curr, head) {
        curr_elem = list_entry(curr, typeof(*curr_elem), list);
        size += 1;
        for (iter = curr_elem->list.next, safe = iter->next; iter != head;
             iter = safe, safe = safe->next) {
            iter_elem = list_entry(iter, typeof(*iter_elem), list);
            if (strcmp(curr_elem->value, iter_elem->value) > 0) {
                list_del(iter);
                free(iter_elem->value);
                free(iter_elem);
            }
        }
    }

    return size;
}

/* Remove every node which has a node with a strictly greater value anywhere
 * to the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    element_t *curr_elem, *iter_elem;
    struct list_head *iter, *safe;
    int size;

    if (!head || q_is_empty(head))
        return 0;

    size = 0;
    for (curr_elem = list_entry(head->prev, typeof(*curr_elem), list);
         &curr_elem->list != head;
         curr_elem =
             list_entry(curr_elem->list.prev, typeof(*curr_elem), list)) {
        size += 1;
        for (iter = curr_elem->list.prev, safe = iter->prev; iter != head;
             iter = safe, safe = safe->prev) {
            iter_elem = list_entry(iter, typeof(*iter_elem), list);
            if (strcmp(curr_elem->value, iter_elem->value) > 0) {
                list_del(iter);
                free(iter_elem->value);
                free(iter_elem);
            }
        }
    }

    return size;
}

/* Merge all the queues into one sorted queue, which is in
 * ascending/descending order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    bool (*cmp[2])(const char *, const char *) = {cmp_ascend, cmp_descend};

    queue_contex_t *q1_contex, *q2_contex;
    element_t *q1_curr_elem, *q2_curr_elem;
    struct list_head *q1_chain, *q2_chain, *q1_curr, *q2_curr, *temp;
    int size;

    if (!head || list_empty(head))
        return 0;

    q1_chain = head->next;
    q1_contex = list_entry(q1_chain, typeof(*q1_contex), chain);
    size = q1_contex->size;
    list_for_each (q2_chain, head) {
        if (q2_chain == q1_chain)
            continue;
        q1_contex = list_entry(q1_chain, typeof(*q1_contex), chain);
        q2_contex = list_entry(q2_chain, typeof(*q2_contex), chain);
        q1_curr = q1_contex->q->next;
        q2_curr = q2_contex->q->next;

        do {
            if (q1_curr == q1_contex->q) {
                temp = q2_curr->next;
                list_del(q2_curr);
                list_add_tail(q2_curr, q1_contex->q);
                q2_curr = temp;
                size += 1;
                goto next_run;
            }
            if (q2_curr == q2_contex->q)
                break;

            q1_curr_elem = list_entry(q1_curr, typeof(*q1_curr_elem), list);
            q2_curr_elem = list_entry(q2_curr, typeof(*q2_curr_elem), list);

            if (cmp[!descend](q1_curr_elem->value, q2_curr_elem->value))
                q1_curr = q1_curr->next;
            else {
                temp = q2_curr->next;
                list_del(q2_curr);
                list_add_tail(q2_curr, q1_curr);
                q2_curr = temp;
                size += 1;
            }
        next_run:
        } while (q1_curr != q1_contex->q || q2_curr != q2_contex->q);
    }

    return size;
}
