#include "list.h"

int main()
{
    initLog();
    
    list_t lst = {};
    int last = 0;

    listDump(&lst);

    listCtor(&lst, 2);
    listDump(&lst);
    
    listPushBack(&lst, 1); 
    listDump(&lst);

    last = listPushFront(&lst, 2);
    listDump(&lst);
    
    listInsert(&lst, 3, last);
    listDump(&lst);
    
    listPushBack(&lst, 4);
    listDump(&lst);
    
    listLinearize(&lst);
    listDump(&lst);
    
    listPushBack(&lst, 5);
    listDump(&lst);
    
    listPopBack(&lst);
    listDump(&lst);
    
    listPopFront(&lst);  
    listDump(&lst);
   
    listDtor(&lst);
    listDump(&lst);

    closeLog();
}