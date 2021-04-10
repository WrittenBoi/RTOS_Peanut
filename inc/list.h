#ifndef __PEANUT_LIST_H
#define __PEANUT_LIST_H

#include "port_macro.h"

typedef struct stLIST_NODE {
    TickType_t xNodeValue;
    struct stLIST_NODE *pxNext;
    struct stLIST_NODE *pxPrev;
    void *pvOwner;              // Object this node embedded, may be optimized.
    void *pvContainer;          // List header
} ListNode_t;

typedef struct stLIST_NODE_MINI {
    TickType_t xNodeValue;
    ListNode_t *pxNext;
    ListNode_t *pxPrev;
} ListNodeMini_t;

typedef struct {
    UBaseType_t uxNodeCounts;
    ListNode_t *pxNodeIndex;
    ListNodeMini_t xListEnd;
} List_t;

#define vListSetNodeOwner(pNode, pOwner) ((pNode)->pvOwner = (pOwner))
#define pvListGetNodeOwner(pNode) ((pNode)->pvOwner)

#define vListSetNodeValue(pNode, value) ((pNode)->xNodeValue = (value))
#define xListGetNodeValue(pNode) ((pNode)->xNodeValue)

#define pxListGetNext(pNode) ((pNode)->pxNext)
#define pxListGetPrev(pNode) ((pNode)->pxPrev)
#define uxListGetNodeCounts(pList) ((pList)->uxNodeCounts)

void vListInit(List_t *const pxList);
void vListInitNode(ListNode_t *const pxNode);

void vListInsert(List_t *const pxList, ListNode_t *const pxNewNode);
void vListInsertEnd(List_t *const pxList, ListNode_t *const pxNewNode);

UBaseType_t uxListDelete(ListNode_t *const pxNode);

#endif // __PEANUT_LIST_H
