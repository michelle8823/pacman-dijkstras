#ifndef __NODE__
#define __NODE__

#include "utils.h"



/**
 * Data structure containing the node information
 */
struct node_s{
    int priority;
    float acc_reward; // accumulated reward
    int depth;
    int num_childs;
    move_t move;
    state_t state;
    struct node_s* parent; // same node before movement is updated
};

typedef struct node_s node_t;


#endif
