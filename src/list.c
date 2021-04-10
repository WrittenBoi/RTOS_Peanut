#include "list.h"

void vListInit(List_t *const pxList) {
    pxList->pxNodeIndex = (ListNode_t*) (&(pxList->xListEnd));
    pxList->xListEnd.pxNext = (ListNode_t*) (&(pxList->xListEnd));
    pxList->xListEnd.pxPrev = (ListNode_t*) (&(pxList->xListEnd));

    pxList->xListEnd.xNodeValue = portMAX_DELAY;
    pxList->uxNodeCounts = (UBaseType_t) 0U;
}

void vListInitNode(ListNode_t *const pxNode) {
    pxNode->pvContainer = NULL;
    pxNode->pxNext = NULL;
    pxNode->pxPrev = NULL;
}

static inline void vListInsertAfterNode(ListNode_t *const pxNewNode,
        ListNode_t *const pxNode) {
    pxNewNode->pxNext = pxNode->pxNext;
    pxNewNode->pxPrev = pxNode;
    pxNode->pxNext->pxPrev = pxNewNode;
    pxNode->pxNext = pxNewNode;
}

static inline void vListInsertBeforeNode(ListNode_t *const pxNewNode,
        ListNode_t *const pxNode) {
    pxNewNode->pxNext = pxNode;
    pxNewNode->pxPrev = pxNode->pxPrev;
    pxNewNode->pxPrev->pxNext = pxNewNode;
    pxNode->pxPrev = pxNewNode;
}

void vListInsertEnd(List_t *const pxList, ListNode_t *const pxNewNode) {
    ListNode_t *const pxIndex = pxList->pxNodeIndex;

    vListInsertBeforeNode(pxNewNode, pxIndex);

    pxNewNode->pvContainer = (void*) pxList;
    pxList->uxNodeCounts++;
}

void vListInsert(List_t *const pxList, ListNode_t *const pxNewNode) {
    ListNode_t *pxNodeIterator = (ListNode_t*) &(pxList->xListEnd);
    TickType_t xNewNodeValue = pxNewNode->xNodeValue;

    //Find the position to insert
    if (portMAX_DELAY != xNewNodeValue) {
        for (; pxNodeIterator->pxNext->xNodeValue <= xNewNodeValue;
                pxNodeIterator = pxNodeIterator->pxNext) {
            ; //Do nothing!
        }
    } else {
        pxNodeIterator = pxNodeIterator->pxPrev;
    }

    // Insert the new node
    vListInsertAfterNode(pxNewNode, pxNodeIterator);
    pxNewNode->pvContainer = (void*) pxList;

    // Update list node counts
    pxList->uxNodeCounts++;

}

UBaseType_t uxListDelete(ListNode_t *const pxNode) {
    List_t *const pxList = (List_t*) pxNode->pvContainer;

    // Unlink the node chain
    pxNode->pxPrev->pxNext = pxNode->pxNext;
    pxNode->pxNext->pxPrev = pxNode->pxPrev;

    // Adjust the list node index
    if (pxList->pxNodeIndex == pxNode) {
        pxList->pxNodeIndex = pxNode->pxPrev;
    }

    // Re-init the node
    vListInitNode(pxNode);

    // Update list node counts
    pxList->uxNodeCounts--;

    return pxList->uxNodeCounts;
}
