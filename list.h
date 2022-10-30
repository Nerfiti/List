#ifndef LIST_H
#define LIST_H

#include <cstdio>
#include <windows.h>

typedef int elem_t;

extern const elem_t STD_POISON;
extern const int STD_LIST_SIZE;

struct list_item
{
    elem_t data = 0;
    int    next = 0;
    int    prev = 0;
};

struct list_t
{
    int size          = -1;
    int capacity      = -1;
    int init_capacity = -1; 
    int last_item     = -1;
    int free          = -1;

    list_item *buf = nullptr;

    elem_t POISON = STD_POISON;
};

enum Errs
{
    List_OK              =  0,
    WrongListPtr         = -1,
    Non_existent_element = -2,
    DumpError            = -3
};

void initLog  ();
void closeLog ();

int listCtor      (list_t *list_ptr, int capacity = STD_LIST_SIZE, elem_t  POISON = STD_POISON);
int listInsert    (list_t *list_ptr, elem_t  item, int position);
int listPushBack  (list_t *list_ptr, elem_t  item);
int listPushFront (list_t *list_ptr, elem_t  item);
int listDelete    (list_t *list_ptr, int position, elem_t *item = nullptr);
int listPopBack   (list_t *list_ptr, elem_t *item = nullptr);
int listPopFront  (list_t *list_ptr, elem_t *item = nullptr);
int listLinearize (list_t *list_ptr);
int listDtor      (list_t *list_ptr);
int listDump      (list_t *list_ptr);

#endif //LIST_H