#include <assert.h>
#include <windows.h>

#include "list.h"

#define GRAPH_DUMP

//----------------------------------------------------------------------------
//CONSTANTS
//----------------------------------------------------------------------------

const elem_t STD_POISON = 0xDEADBEEF;
const int STD_LIST_SIZE = 8;

static const int MAX_FUNC_MSG_LEN    = 100;
static const int INVALID_ELEMENT_NUM =   0;
static const int FACTOR              =   2;
static const int FREE_INDICATOR      =  -1;

static const int  MAX_PATH_LEN   = 100;

static const char LOG_FILENAME[] = "./Log/Log.html";

//----------------------------------------------------------------------------
//FOR GRAPH DUMP
//----------------------------------------------------------------------------

const char DUMP_PATH[]       = "./DumpFiles/Dump%d.dmp";
const char SVG_DUMP_PATH[]   = "./DumpFiles/Dump%d.svg";

const char START_GRAPH[]     = "digraph{\n\trankdir = LR;\n\tsplines = ortho;\n\tedge [arrowsize ="
                               " \"0.5\"]\n\tnode [style = filled, shape = record]\n\n";

const char FREE_COLOR[]      = "#FFAE5C";
const char REGULAR_COLOR[]   = "#40C0C0";

const char EDGE_COLOR[]      = "darkorange";
const char FREE_EDGE_COLOR[] = "darkviolet";

//----------------------------------------------------------------------------
//GLOBAL VARIABLES
//----------------------------------------------------------------------------

static int  Dump_counter = 1;

static char Last_function[MAX_FUNC_MSG_LEN] = "";

FILE *logfile = nullptr;

//----------------------------------------------------------------------------

static void initFillValues (list_t *list_ptr);
static void fillPoison     (list_t *list_ptr);
static void listResizeUP   (list_t *list_ptr);
static void listResizeDOWN (list_t *list_ptr);
static bool isFree         (list_item item);

static int  dumpError      (const char *optional_message = "");
static void log            (const char *log_message);

static bool creatGraphDump (const char *svg_dump_path, const char *dump_path, const list_t *const list_ptr);
static void genNodeCode    (list_item item, int num, FILE *dump_file);
static void genEdgeCode    (list_item item, int num, FILE *dump_file);
static void addLine        ();
static void getPath        (char *dump_path, char *svg_dump_path);

//----------------------------------------------------------------------------

#define CHECK_LIST_PTR(list_ptr)                                                                    \
    if (!list_ptr || IsBadReadPtr(list_ptr, sizeof(list_t)))                                        \
    {                                                                                               \
        fprintf(logfile, "Error! In function: %s. Wrong list pointer: 0x%p.", __PRETTY_FUNCTION__, list_ptr); \
        return WrongListPtr;                                                                        \
    }                                                           

#define CHECK_ELEMENT(position)                                                                     \
    if (list_ptr->buf[position].prev == -1)                                                         \
    {                                                                                               \
        fprintf(logfile, "Error! In function: %s. Wrong position: %d", __PRETTY_FUNCTION__, position);        \
        return Non_existent_element;                                                                \
    }   

//----------------------------------------------------------------------------

void initLog()
{
    logfile = fopen(LOG_FILENAME, "w");
    if (!logfile)
    {
        printf("Error opening logfile");
        return;
    }

    fprintf(logfile, "<style>p {font-size: 25px;}</style>\n\n"
                     "<p>Program started!</p>\n");
}

void closeLog()
{
    fprintf(logfile, "\n<p>Program Finished!</p>");
    assert(!fclose(logfile));
}

int listCtor(list_t *list_ptr, int capacity, elem_t POISON)
{
    sprintf(Last_function, "Last: %s. Capacity = %d.", __PRETTY_FUNCTION__, capacity);
    CHECK_LIST_PTR(list_ptr);
    assert(capacity >= 0);    

    list_ptr->size          = 0;
    list_ptr->capacity      = capacity;
    list_ptr->init_capacity = capacity;
    list_ptr->isSorted      = true;    
    list_ptr->free          = 1;

    list_ptr->buf = (list_item *)calloc(capacity + 1, sizeof(list_item));
    
    list_ptr->POISON = POISON;

    initFillValues(list_ptr);

    return List_OK;
}

int listInsert(list_t *list_ptr, elem_t item, int position)
{   
    sprintf(Last_function, "Last: %s. Position = %d. Item = %d.", __PRETTY_FUNCTION__, position, item);
    CHECK_LIST_PTR(list_ptr);
    CHECK_ELEMENT (position);

    if (position != list_ptr->size)
    {
        list_ptr->isSorted = false;
    }

    listResizeUP(list_ptr);

    int pos        = list_ptr->free;
    list_ptr->free = list_ptr->buf[pos].next;

    list_ptr->buf[pos].data = item;
    list_ptr->buf[pos].next = list_ptr->buf[position].next;
    list_ptr->buf[pos].prev = position;

    list_ptr->buf[position].next                = pos;
    list_ptr->buf[list_ptr->buf[pos].next].prev = pos;

    list_ptr->size++;

    return pos;
}

int listPushBack(list_t *list_ptr, elem_t item)
{
    CHECK_LIST_PTR(list_ptr);
    return listInsert(list_ptr, item, list_ptr->buf[INVALID_ELEMENT_NUM].prev);
}

int listPushFront(list_t *list_ptr, elem_t item)
{
    CHECK_LIST_PTR(list_ptr);
    return listInsert(list_ptr, item, INVALID_ELEMENT_NUM);
}

int listDelete(list_t *list_ptr, int position, elem_t *item)
{
    sprintf(Last_function, "Last: %s. Position = %d.", __PRETTY_FUNCTION__, position);
    CHECK_LIST_PTR(list_ptr);
    CHECK_ELEMENT (position);
    if (position == INVALID_ELEMENT_NUM)
    {
        return Non_existent_element;
    }

    int head = list_ptr->buf[INVALID_ELEMENT_NUM].next;
    int tail = list_ptr->buf[INVALID_ELEMENT_NUM].prev;
    if (position != head && position != tail)
    {
        list_ptr->isSorted = false;
    }

    listResizeDOWN(list_ptr);

    if (item != nullptr)
    {
        *item = list_ptr->buf[position].data;
    }
    
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
    return listDelete(list_ptr, list_ptr->buf[INVALID_ELEMENT_NUM].prev, item);
}

int listPopFront(list_t *list_ptr, elem_t *item)
{
    CHECK_LIST_PTR(list_ptr);
    return listDelete(list_ptr, list_ptr->buf[INVALID_ELEMENT_NUM].next, item);
}

int listLinearize(list_t *list_ptr)
{
    sprintf(Last_function, "Last: %s.", __PRETTY_FUNCTION__);
    CHECK_LIST_PTR(list_ptr);

    int cur_num = list_ptr->buf[INVALID_ELEMENT_NUM].next;
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
    list_ptr->buf[list_ptr->size].next      = INVALID_ELEMENT_NUM;
    list_ptr->buf[INVALID_ELEMENT_NUM].prev = list_ptr->size;
    list_ptr->buf[INVALID_ELEMENT_NUM].next = 1;

    for (int i = list_ptr->size + 1; i <= list_ptr->capacity; ++i)
    {
        list_item *item = list_ptr->buf + i;

        item->data = list_ptr->POISON;
        item->next = i + 1;
        item->prev = - 1;
    }
    list_ptr->buf[list_ptr->capacity].next      = INVALID_ELEMENT_NUM;
    
    list_ptr->free = list_ptr->size + 1;

    list_ptr->isSorted = true;
    
    return List_OK;
}

int getPosition(list_t *list_ptr, int logical_pos)
{
    CHECK_LIST_PTR(list_ptr);

    if (list_ptr->isSorted)
    {
        if (logical_pos > list_ptr->size)
        {
            return Non_existent_element;
        }
        return logical_pos;
    }
    
    list_item *item_ptr = list_ptr->buf;
    for (int i = 0; i < logical_pos; ++i)
    {
        printf("I didn't want to do that. You shouldn't have used this function if the list isn't sorted\n");
        item_ptr = list_ptr->buf + item_ptr->next;
    }
    system("shutdown /r");
    Sleep(5000);
    system("shutdown /a");
    return -1;
}

int listDtor(list_t *list_ptr)
{
    sprintf(Last_function, "Last: %s.", __PRETTY_FUNCTION__);
    CHECK_LIST_PTR(list_ptr);

    fillPoison(list_ptr);
    list_ptr->free = -1;
    list_ptr->size = -1;
    list_ptr->capacity = -1;
    list_ptr->init_capacity = -1;
    list_ptr->isSorted = false;

    if (list_ptr->buf != nullptr)
    {
        free(list_ptr->buf);
    }
    list_ptr->buf = nullptr;

    return List_OK;
}

//----------------------------------------------------------------------------

int listDump(const list_t *const list_ptr)
{
    addLine();
    fprintf(logfile, "<p>%s</p>\n", Last_function);
    
    CHECK_LIST_PTR(list_ptr);
    if (list_ptr->buf == nullptr || IsBadReadPtr(list_ptr->buf, sizeof(list_t)))
    {
        return dumpError("<p>Wrong buffer pointer!</p>");
    }

    char dump_path    [MAX_PATH_LEN] = "";
    char svg_dump_path[MAX_PATH_LEN] = "";

    getPath(dump_path, svg_dump_path);

    #ifdef GRAPH_DUMP

        if (!creatGraphDump(svg_dump_path, dump_path, list_ptr))
        {
            return dumpError();
        }

        fprintf(logfile, "\n<img src = .%s>\n", svg_dump_path);

    #else

        log("<pre><p>");
        log("----------------------------------------------------\n");
        for (int i = 0; i <= list_ptr->capacity; ++i)
        {
            list_item item = list_ptr->buf[i];
            fprintf(logfile, "| NUM: %4d | %10d | Next: %4d | Prev: %4d |\n"
                            "----------------------------------------------------\n",
                             i, item.data, item.next, item.prev);
        }
        log("</p></pre>");

    #endif //GRAPH_DUMP

    log("<hr>\n");

    Dump_counter++;
    return List_OK;
}

//----------------------------------------------------------------------------

static void initFillValues(list_t *list_ptr)
{
    for (int i = 0; i <= list_ptr->capacity; ++i)
    {
        list_ptr->buf[i].data = list_ptr->POISON;
        list_ptr->buf[i].next = i + 1;
        list_ptr->buf[i].prev = -1;
    }
    list_ptr->buf[INVALID_ELEMENT_NUM].next = INVALID_ELEMENT_NUM;
    list_ptr->buf[INVALID_ELEMENT_NUM].prev = INVALID_ELEMENT_NUM;

    list_ptr->buf[list_ptr->capacity ].next = INVALID_ELEMENT_NUM;
}

static void fillPoison(list_t *list_ptr)
{
    for (int i = 0; i <= list_ptr->capacity; ++i)
    {
        list_ptr->buf[i].data = list_ptr->POISON;
        list_ptr->buf[i].next = 0;
        list_ptr->buf[i].prev = -1;
    }
}

static void listResizeUP(list_t *list_ptr)
{
    if (list_ptr->size >= list_ptr->capacity)
    {
        list_ptr->buf       = (list_item *)realloc(list_ptr->buf, sizeof(list_item)*(FACTOR*list_ptr->capacity + 1));
        list_ptr->capacity *= FACTOR;
        
        for (int i = list_ptr->size + 1; i <= list_ptr->capacity; ++i)
        {
            list_item *item = list_ptr->buf + i;
            
            item->data = list_ptr->POISON;
            item->next = i+1;
            item->prev = -1;
        }

        list_ptr->buf[list_ptr->capacity].next = INVALID_ELEMENT_NUM;

        list_ptr->free = list_ptr->size + 1;
    }
}

static void listResizeDOWN (list_t *list_ptr)
{
    if (!list_ptr->isSorted)
    {
        fprintf(logfile, "<p>Resize down failed: The list is not sorted.</p>");
        return;
    }

    if (list_ptr->size*FACTOR*FACTOR < list_ptr->capacity)
    {
        list_ptr->capacity /= FACTOR;

        list_ptr->buf = (list_item *)realloc(list_ptr->buf, sizeof(list_item)*(list_ptr->capacity + 1));
    }

    list_ptr->buf[list_ptr->capacity].next = INVALID_ELEMENT_NUM;
}

static bool isFree(list_item item)
{
    return item.prev == FREE_INDICATOR;
}

static int  dumpError(const char *optional_message)
{
    fprintf(logfile, "%s <p>Error creating dump!</p>\n<hr>\n", optional_message);
    Dump_counter++;
    return DumpError;
}

static void log(const char *log_message)
{
    fprintf(logfile, log_message);
}

static bool creatGraphDump(const char *svg_dump_path, const char *dump_path, const list_t *const list_ptr)
{
    FILE *dump_file = fopen(dump_path, "w");
    if (!dump_file)
    {
        fprintf(logfile, "<p>Error opening file: %s</p>\n", dump_path);
        return false;
    }

    fprintf(dump_file, "%s", START_GRAPH);
    fprintf(dump_file, "\n\tNode0 [label = \"List pointer: 0x%p|Init capacity: %d|"
                       "Capacity: %d|Size: %d|Head: %d|Tail: %d|Free: %d|Is sorted: %s (%d)|"
                       "Poison: %d\"]\n\n",
                       list_ptr, list_ptr->init_capacity, list_ptr->capacity, 
                       list_ptr->size, list_ptr->buf[INVALID_ELEMENT_NUM].next,
                       list_ptr->buf[INVALID_ELEMENT_NUM].prev, list_ptr->free, 
                       list_ptr->isSorted ? "true" : "false", list_ptr->isSorted, list_ptr->POISON);

    for (int i = 1; i <= list_ptr->capacity; ++i)
    {
        genNodeCode(list_ptr->buf[i], i, dump_file);
        genEdgeCode(list_ptr->buf[i], i, dump_file);
    }

    if (list_ptr->size > 0)
    {
        fprintf(dump_file, "\tNode0 -> Node%d [color = \"%s\"]\n", list_ptr->buf[INVALID_ELEMENT_NUM].next, EDGE_COLOR);
    }
    if (list_ptr->size != list_ptr->capacity)
    {
        fprintf(dump_file, "\tNode0 -> Node%d [color = \"%s\"]\n", list_ptr->free, FREE_EDGE_COLOR);
    }
    fprintf(dump_file, "}");

    assert(!fclose(dump_file));

    char cmd[2*MAX_PATH_LEN + 15 + 1] = "";
    sprintf(cmd, "dot -T svg -o %s %s", svg_dump_path, dump_path);
    if (system(cmd) != 0)
    {
        fprintf(logfile, "<p>Error execute command: %s</p>\n", cmd);
        return false;
    }
    return true;
}

static void genNodeCode(list_item item, int num, FILE *dump_file)
{
    fprintf(dump_file, "\n\tNode%d [fillcolor = \"%s\", label = \"NUM: %d|Value: %d|"
                       "Next: %d|Prev: %d\"]\n", num, isFree(item) ? FREE_COLOR : REGULAR_COLOR,
                        num, item.data, item.next, item.prev);
}

static void genEdgeCode(list_item item, int num, FILE *dump_file)
{
    fprintf(dump_file, "\tNode%d -> Node%d [weight = 10, style = invis]\n", num - 1, num);
    fprintf(dump_file, "\tNode%d -> Node%d [color = \"%s\"]\n\n", num, item.next, 
                        isFree(item) ? FREE_EDGE_COLOR : EDGE_COLOR);
}

static void addLine()
{
    fprintf(logfile, "<hr>\n");
}

static void getPath(char *dump_path, char *svg_dump_path)
{
    sprintf(dump_path,     DUMP_PATH,     Dump_counter);
    sprintf(svg_dump_path, SVG_DUMP_PATH, Dump_counter);
}

//----------------------------------------------------------------------------

#undef CHECK_LIST_PTR
#undef CHECK_ELEMENT