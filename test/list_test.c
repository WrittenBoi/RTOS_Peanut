#include <stdio.h>
#include <stdlib.h>

#include "list.h"

#define NODE_NUM    (10)

static List_t test_list;
static ListNode_t nodes[NODE_NUM];

void list_test(void) {
    vListInit(&test_list);

    for (int i = 0; i < NODE_NUM; i++) {
        vListInitNode(&nodes[i]);
//        nodes[i].xNodeValue = rand() % 100;
        nodes[i].xNodeValue = i;
        vListInsert(&test_list, &nodes[i]);
    }

    uxListDelete(&nodes[NODE_NUM - 1]);
    uxListDelete(&nodes[0]);
    uxListDelete(&nodes[NODE_NUM / 2]);

    while (1) {
        ;
    }
    return;
}
