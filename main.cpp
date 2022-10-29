#include "list.h"

int main()
{
    list_t lst = {};
    int last = 0;
    int value = 0;

    listDump(lst);

    listCtor(&lst, 2);
    listDump(lst);
    
    listPushBack(&lst, 1); 
    listDump(lst);

    last = listPushFront(&lst, 2);
    printf("Last = %d\n", last);
    listDump(lst);
    
    listInsert(&lst, 3, last);
    listDump(lst);
    
    listPushBack(&lst, 4);
    listDump(lst);
    
    listLinearize(&lst);
    listDump(lst);
    
    listPushBack(&lst, 5);
    listDump(lst);
    
    listDelete(&lst, &value, last);
    listDump(lst);
    printf("Value = %d\n", value);
    
    listPopBack(&lst, &value);
    listDump(lst);
    printf("Value = %d\n", value);
    
    listPopFront(&lst, &value);  
    listDump(lst);
    printf("Value = %d\n", value);
    
    listDtor(&lst);
    listDump(lst);
}