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

#define vListSetNodeOwner(pNode, pOwner)    ((pNode)->pvOwner = (pOwner))
#define pvListGetNodeOwner(pNode)           ((pNode)->pvOwner)

#define vListSetNodeValue(pNode, value)     ((pNode)->xNodeValue = (value))
#define xListGetNodeValue(pNode)            ((pNode)->xNodeValue)

#define pxListGetNext(pNode)            ((pNode)->pxNext)
#define pxListGetPrev(pNode)            ((pNode)->pxPrev)
#define uxListGetNodeCounts(pList)      ((pList)->uxNodeCounts)

// 获取List的节点数
#define listGET_LIST_LENGTH(pxList)  ((pxList)->uxNodeCounts)

// 空List？
#define listLIST_IS_EMPTY(pxList)   \
    ((UBaseType_t)((UBaseType_t)0 == listGET_LIST_LENGTH(pxList)))

// 设定节点的值
#define listSET_LIST_ITEM_VALUE(pxNode, xNewValue)  \
                ((pxNode)->xNodeValue = (TickType_t)(xNewValue))
// 取得节点的值
#define listGET_LIST_ITEM_VALUE(pxNode)     ((pxNode)->xNodeValue)

// 获取List的下一个节点，更新NodeIndex，并将该节点的Owner赋给pxTCB
#define listGET_OWNER_OF_NEXT_ENTRY(pxTCB, pxList)  do {                \
    List_t *const pxConstList = (pxList);                               \
    pxConstList->pxNodeIndex = pxConstList->pxNodeIndex->pxNext;        \
    if ((pxConstList->pxNodeIndex) ==                                   \
        ((ListNode_t*) (&(pxConstList->xListEnd)))) {                   \
        pxConstList->pxNodeIndex = pxConstList->pxNodeIndex->pxNext;    \
    }                                                                   \
    (pxTCB) = pvListGetNodeOwner(pxConstList->pxNodeIndex);             \
}while (0)

// 获取List的头节点
#define listGET_HEAD_ENTRY(pxList) ((&((pxList)->xListEnd))->pxNext)

// 获取List的头节点的OWNER
#define listGET_OWNER_OF_HEAD_ENTRY(pxList)     \
                (listGET_HEAD_ENTRY(pxList)->pvOwner)

void vListInit(List_t *const pxList);
void vListInitNode(ListNode_t *const pxNode);

void vListInsert(List_t *const pxList, ListNode_t *const pxNewNode);
void vListInsertEnd(List_t *const pxList, ListNode_t *const pxNewNode);

UBaseType_t uxListRemove(ListNode_t *const pxNode);

#endif // __PEANUT_LIST_H
