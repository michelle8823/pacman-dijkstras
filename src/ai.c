#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <math.h>


#include "ai.h"
#include "utils.h"
#include "priority_queue.h"

#define GAME_OVER    -1
#define ACTION_NO     4

struct heap h;

// Function prototypes
float get_reward( node_t* n );
void propagateBackScoreToFirstAction(propagation_t propagation, node_t n);


/**
 * Function called by pacman.c
*/
void initialize_ai(){
	heap_init(&h);
}

/**
 * function to copy a src into a dst state
*/
void copy_state(state_t* dst, state_t* src){
	//Location of Ghosts and Pacman
	memcpy( dst->Loc, src->Loc, 5*2*sizeof(int) );

    //Direction of Ghosts and Pacman
	memcpy( dst->Dir, src->Dir, 5*2*sizeof(int) );

    //Default location in case Pacman/Ghosts die
	memcpy( dst->StartingPoints, src->StartingPoints, 5*2*sizeof(int) );

    //Check for invincibility
    dst->Invincible = src->Invincible;
    
    //Number of pellets left in level
    dst->Food = src->Food;
    
    //Main level array
	memcpy( dst->Level, src->Level, 29*28*sizeof(int) );

    //What level number are we on?
    dst->LevelNumber = src->LevelNumber;
    
    //Keep track of how many points to give for eating ghosts
    dst->GhostsInARow = src->GhostsInARow;

    //How long left for invincibility
    dst->tleft = src->tleft;

    //Initial points
    dst->Points = src->Points;

    //Remiaining Lives
    dst->Lives = src->Lives;   

}

node_t* create_init_node( state_t* init_state ){
	node_t * new_n = (node_t *) malloc(sizeof(node_t));
	new_n->parent = NULL;	
	new_n->priority = 0;
	new_n->depth = 0;
	new_n->num_childs = 0;
	copy_state(&(new_n->state), init_state);
	new_n->acc_reward =  get_reward( new_n );

	return new_n;	
}


// MY CODE
float heuristic(node_t* n){
	float h = 0;
	
    float invincible = 0;
    float lost_life = 0;
    float game_over = 0;
    
    invincible = n->state.Invincible;
    
    // Check if life is lost in the state
    if (n->state.Lives == n->parent->state.Lives - 1) {
        lost_life = 10;
    }
    
    // Check if game over in the state
    if (n->state.Lives == GAME_OVER) {
        game_over = 100;
    }
    
    // Calculate final heuristic score
    h = invincible - lost_life - game_over;    

	return h;
}


/* Calculate reward based on heuristic and discount factor */
float get_reward (node_t* n){
	float reward = 0;
	
	// MY CODE
	// Calculate reward
	float heur = heuristic(n);
	float score = n->acc_reward - n->parent->acc_reward;
	float discount = pow(0.99,n->depth);
   	
	reward = (heur + score) * discount;
	
	return reward;
}

/**
 * Apply an action to node n and return a new node resulting from executing
 * the action, changing the state. Returns false if move is invalid (eg. 
 * pacman bumps into a wall) 
 */
bool applyAction(node_t* n, node_t** new_node, move_t action){

	bool changed_dir = false;

    // MY CODE
	// Update parent, move, reward, priority, child of new node
	(*new_node)->parent = n;
	(*new_node)->move = action; 
	(*new_node)->acc_reward = get_reward(n);
	(*new_node)->priority = -n->depth;
    (*new_node)->num_childs += 1;
	

    changed_dir = execute_move_t( &((*new_node)->state), action );

	return changed_dir;

}


/**
 * Find best action by building all possible paths up to budget
 * and back propagate using either max or avg
 */

move_t get_next_move(state_t init_state, int budget, 
                     propagation_t propagation, char* stats) {
	move_t best_action = rand() % 4;

	float best_action_score[ACTION_NO];
	for(unsigned i = 0; i < ACTION_NO; i++)
	    best_action_score[i] = INT_MIN;

	unsigned generated_nodes = 0;
	unsigned expanded_nodes = 0;
	unsigned max_depth = 0;
	

    // MY CODE
	// GRAPH ALGORITHM
	node_t* node = create_init_node(&init_state);
    node_t *explored[budget];
    
    // Create priority queue and add starting node to it
    struct heap *frontier = (struct heap*)malloc(sizeof(frontier));
	heap_push(frontier, node);
    
	int index = 0;
	enum moves possible_moves;
    int len_explored = sizeof(explored) / sizeof(explored[0]);
    
    while (frontier) {
		// Pop node in priority queue and mark it as explored
        node = heap_delete(frontier);
		explored[index] = node;
		index++;

		if (len_explored < budget) {
			int i;
			for (i = 0; i < ACTION_NO; i++) {
				node_t *new;
                applyAction(node, &new, i);
				// Propagation only possible if not first iteration of heap
				if (i != 0) {
					propagateBackScoreToFirstAction(propagation, *node);
				}
				// Delete node if life is lost, otherwise add it to the 
				// frontier priority queue
				if (new->state.Lives == node->parent->state.Lives - 1) {
					free(new);
				}
				else {
					heap_push(frontier, new);
				}
			}
		}
    }
    
    // Find action with highest score and udpdate the best action
    int node_no;
    for (node_no = 0; node_no < ACTION_NO; node_no++) {
        if (explored[node_no]->acc_reward > explored[node_no]->parent->acc_reward) {
            best_action = explored[node_no]->move;
        }    
    }
    
    // Free frontier heap
    free(frontier);
    
	// Free the expanded nodes in the explored array
	int i;
	for (i = 0; i < len_explored; i++) {
		free(explored[i]);
	}

	// best_action (move_t) equals to.... 0-3 representing the move, returned 
	 // at the end already
    


	sprintf(stats, "Max Depth: %d Expanded nodes: %d  Generated nodes: %d\n",
	        max_depth,expanded_nodes,generated_nodes);
	
	if(best_action == left)
		sprintf(stats, "%sSelected action: Left\n",stats);
	if(best_action == right)
		sprintf(stats, "%sSelected action: Right\n",stats);
	if(best_action == up)
		sprintf(stats, "%sSelected action: Up\n",stats);
	if(best_action == down)
		sprintf(stats, "%sSelected action: Down\n",stats);

	sprintf(stats, "%sScore Left %f Right %f Up %f Down %f",stats,
	        best_action_score[left],best_action_score[right],
			best_action_score[up],best_action_score[down]);

	return best_action;
}

// EXTRA FUNCTIONS - MY CODE
/* Takes the reward of a new node and update the reward score of the 
   first move*/
void 
propagateBackScoreToFirstAction(propagation_t propagation, node_t n) {
	float max_prop = 0;
	float avg_prop = 0;
	int node_count = 0;
	float total_score = 0;

	// Base case: reached node of the first move
	if (n.parent == NULL) {
		// Case 1: Max propagation
		if (propagation = max) {
			n.acc_reward = max_prop;
		}
		// Case 2: Average propagation
		else {
			n.acc_reward = avg_prop;	
		}
	}

	// Recusrive case: traverse to parent node whilst calculating scores
	else {
		// Keep track of score 
		node_count++;
		total_score += n.acc_reward;

		// Update max score
		if (n.acc_reward > max_prop) {
			max_prop = n.acc_reward;
		}
		// Update average score
		else {
			avg_prop = total_score / node_count;
		}

		propagateBackScoreToFirstAction(propagation, n);
	}
}

