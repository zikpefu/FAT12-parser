/* list.h
 *
 * Public functions for two-way linked list
 *
 * You should not need to change any of the code this file.  If you do, you
 * must get permission from the instructor.
 */
#include <stdbool.h>
typedef struct chunk_tag {
        //length in bytes
        unsigned char filename[9]; //8
        unsigned char extension[3]; //3
        unsigned char attributes[1]; //1
        unsigned short int first_clust; //2cd g
        bool deleted;
        size_t file_size; //4
}data_t;

typedef data_t* DataPtr;

typedef struct list_node_tag {
        // private members for list.c only
        data_t *data_ptr;
        struct list_node_tag *prev;
        struct list_node_tag *next;
} list_node_t;

typedef struct list_tag {
        // private members for list.c only
        list_node_t *head;
        list_node_t *tail;
        int current_list_size;
} list_t;

// public definition of pointer into linked list
typedef list_node_t * IteratorPtr;
typedef list_t * ListPtr;
typedef data_t* DataPtr;
// public prototype definitions for list.c

// build and cleanup lists
ListPtr list_construct(void);
void list_destruct(ListPtr list_ptr);

// iterators into positions in the list
IteratorPtr list_iter_front(ListPtr list_ptr);
IteratorPtr list_iter_back(ListPtr list_ptr);
IteratorPtr list_iter_next(IteratorPtr idx_ptr);

data_t * list_access(ListPtr list_ptr, IteratorPtr idx_ptr);
IteratorPtr list_elem_find(ListPtr list_ptr, data_t *elem_ptr);

void list_insert(ListPtr list_ptr, data_t *elem_ptr, IteratorPtr idx_ptr);
data_t * list_remove(ListPtr list_ptr, IteratorPtr idx_ptr);

int list_size(ListPtr list_ptr);

/* commands for vim. ts: tabstop, sts: soft tabstop sw: shiftwidth */
/* vi:set ts=8 sts=4 sw=4 et: */
