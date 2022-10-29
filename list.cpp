#include <assert.h>

#include "list.h"


const elem_t STD_POISON = 0xDEADBEEF;
const int std_list_size = 8;

static const int invalid_element_num = 0;
static const int factor              = 2;


static void initFillValues (list_t *list_ptr);
static void fillPoison     (list_t *list_ptr);
static void SetLastItem    (int *last_item, int cur_item);
static void listResizeUP   (list_t *list_ptr);
static void listResizeDOWN (list_t *list_ptr);

#define CHECK_LIST_PTR(list_ptr)                                                                    \
    if (!list_ptr || IsBadReadPtr(list_ptr, sizeof(list_t)))                                        \
    {                                                                                               \
        printf("Error! In function: %s. Wrong list pointer: 0x%p.", __PRETTY_FUNCTION__, list_ptr); \
        return WrongListPtr;                                                                        \
    }                                                           

#define CHECK_ELEMENT(position)                                                                     \
    if (list_ptr->buf[position].prev == -1)                                                             \
    {                                                                                               \
        printf("Error! In function: %s. Wrong position: %d", __PRETTY_FUNCTION__, position);        \
        return Non_existent_element;                                                                \
    }   

int listCtor(list_t *list_ptr, int capacity, elem_t POISON)
{
    CHECK_LIST_PTR(list_ptr);
    assert(capacity >= 0);    

    list_ptr->size          = 0;
    list_ptr->capacity      = capacity;
    list_ptr->init_capacity = capacity;
    list_ptr->last_item     = 0;    
    list_ptr->free          = 1;

    list_ptr->buf = (list_item *)calloc(capacity + 1, sizeof(list_item));
    
    list_ptr->POISON = POISON;

    initFillValues(list_ptr);

    return List_OK;
}

int listInsert(list_t *list_ptr, elem_t item, int position)
{   
    CHECK_LIST_PTR(list_ptr);
    CHECK_ELEMENT (position);

    listResizeUP(list_ptr);

    int pos        = list_ptr->free;
    list_ptr->free = list_ptr->buf[pos].next;

    list_ptr->buf[pos].data = item;
    list_ptr->buf[pos].next = list_ptr->buf[position].next;
    list_ptr->buf[pos].prev = position;

    list_ptr->buf[position].next                = pos;
    list_ptr->buf[list_ptr->buf[pos].next].prev = pos;

    list_ptr->size++;
    SetLastItem(&list_ptr->last_item, pos);

    return pos;
}

int listPushBack(list_t *list_ptr, elem_t item)
{
    CHECK_LIST_PTR(list_ptr);
    return listInsert(list_ptr, item, list_ptr->buf[invalid_element_num].prev);
}

int listPushFront(list_t *list_ptr, elem_t item)
{
    CHECK_LIST_PTR(list_ptr);
    return listInsert(list_ptr, item, invalid_element_num);
}

int listDelete(list_t *list_ptr, elem_t *item, int position)
{
    CHECK_LIST_PTR(list_ptr);
    CHECK_ELEMENT (position);
    if (position == invalid_element_num)
    {
        return Non_existent_element;
    }

    listResizeDOWN(list_ptr);

    *item = list_ptr->buf[position].data;
    
    list_ptr->buf[list_ptr->buf[position].prev].next = list_ptr->buf[position].next;
    list_ptr->buf[list_ptr->buf[position].next].prev = list_ptr->buf[position].prev;
    
    list_ptr->buf[position].data = list_ptr->POISON;
    list_ptr->buf[position].next = list_ptr->free;
    list_ptr->buf[position].prev = -1;

    list_ptr->free = position;

    return List_OK;
}

int listPopBack(list_t *list_ptr, elem_t *item)
{
    CHECK_LIST_PTR(list_ptr);
    return listDelete(list_ptr, item, list_ptr->buf[invalid_element_num].prev);
}

int listPopFront(list_t *list_ptr, elem_t *item)
{
    CHECK_LIST_PTR(list_ptr);
    return listDelete(list_ptr, item, list_ptr->buf[invalid_element_num].next);
}

int listLinearize(list_t *list_ptr)
{
    CHECK_LIST_PTR(list_ptr);

    int cur_num = list_ptr->buf[invalid_element_num].next;
    for (int i = 1; i <= list_ptr->size; ++i)
    {
        list_ptr->buf[i].prev = list_ptr->buf[cur_num].data;
        cur_num = list_ptr->buf[cur_num].next;
    }

    for (int i = 1; i <= list_ptr->size; ++i)
    {
        list_item *item = list_ptr->buf + i;
        
        item->data = item->prev;
        item->next = i + 1;
        item->prev = i - 1;
    }
    list_ptr->buf[list_ptr->size].next      = invalid_element_num;
    list_ptr->buf[invalid_element_num].prev = list_ptr->size;
    list_ptr->buf[invalid_element_num].next = 1;

    for (int i = list_ptr->size + 1; i <= list_ptr->capacity; ++i)
    {
        list_item *item = list_ptr->buf + i;

        item->data = list_ptr->POISON;
        item->next = i + 1;
        item->prev = - 1;
    }
    list_ptr->buf[list_ptr->capacity].next      = invalid_element_num;
    
    list_ptr->free = list_ptr->size + 1;

    return List_OK;
}

int listDtor(list_t *list_ptr)
{
    CHECK_LIST_PTR(list_ptr);

    list_ptr->free = -1;
    fillPoison(list_ptr);

    return List_OK;
}

void listDump(list_t list_ptr)
{
    printf("Free = %d\n", list_ptr.free);
    const int data_len = 100000;

    char data_d[data_len] = "";
    int  data_pos      =  0;
    int  data_delta    =  0;

    char next_d[data_len] = "";
    int  next_pos      =  0;
    int  next_delta    =  0;

    char prev_d[data_len] = "";
    int  prev_pos      =  0;
    int  prev_delta    =  0;
    for (int i = 0; i <= list_ptr.capacity; ++i)
    {
        printf(" %10d ", i);
        sprintf(data_d + data_pos, " %10d ", list_ptr.buf[i].data);
        sprintf(next_d + next_pos, " %10d ", list_ptr.buf[i].next);
        sprintf(prev_d + prev_pos, " %10d ", list_ptr.buf[i].prev);
        data_pos += 12;
        next_pos += 12;
        prev_pos += 12;
    }   
    printf("\n%s\n" "%s\n" "%s\n", data_d, next_d, prev_d);
    system("pause");
}



static void initFillValues(list_t *list_ptr)
{
    for (int i = 0; i <= list_ptr->capacity; ++i)
    {
        list_ptr->buf[i].data = list_ptr->POISON;
        list_ptr->buf[i].next = i + 1;
        list_ptr->buf[i].prev = -1;
    }
    list_ptr->buf[invalid_element_num].next = invalid_element_num;
    list_ptr->buf[invalid_element_num].prev = invalid_element_num;

    list_ptr->buf[list_ptr->capacity ].next = invalid_element_num;
}

static void fillPoison(list_t *list_ptr)
{
    for (int i = 0; i <= list_ptr->capacity; ++i)
    {
        list_ptr->buf[i].data = list_ptr->POISON;
        list_ptr->buf[i].next = list_ptr->POISON;
        list_ptr->buf[i].prev = list_ptr->POISON;
    }
}

static void SetLastItem(int *last_item, int cur_item)
{
    if (*last_item < cur_item)
    {
        *last_item = cur_item;
    }
}

static void listResizeUP(list_t *list_ptr)
{
    if (list_ptr->size >= list_ptr->capacity)
    {
        list_ptr->buf       = (list_item *)realloc(list_ptr->buf, sizeof(list_item)*(factor*list_ptr->capacity + 1));
        list_ptr->capacity *= factor;
        
        for (int i = list_ptr->size + 1; i <= list_ptr->capacity; ++i)
        {
            list_item *item = list_ptr->buf + i;
            
            item->data = list_ptr->POISON;
            item->next = i+1;
            item->prev = -1;
        }

        list_ptr->buf[list_ptr->capacity].next = invalid_element_num;

        list_ptr->free = list_ptr->size + 1;
    }
}

static void listResizeDOWN (list_t *list_ptr)
{
    int lim_num = (list_ptr->last_item > list_ptr->init_capacity) ? list_ptr->last_item : list_ptr->init_capacity;

    if (lim_num*factor*factor < list_ptr->capacity)
    {
        list_ptr->capacity /= factor;

        list_ptr->buf = (list_item *)realloc(list_ptr->buf, sizeof(list_item)*(list_ptr->capacity + 1));
    }

    list_ptr->buf[list_ptr->capacity].next = invalid_element_num;
}

#undef CHECK_LIST_PTR