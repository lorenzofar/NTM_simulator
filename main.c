#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONFIG_DIM 3

typedef enum
{
    FALSE, 
    TRUE
} bool;

// Define directions as enum in order to use them in sums
typedef enum{
    L,
    S, 
    R
} dir;

// Transition representation
typedef struct transition
{
    int target;
    char in;
    char out;
    dir move;
    struct transition *next;
} transition;

// State representation
typedef struct state
{
    bool final;
    transition *transitions;
} state;

// Table to store data
typedef struct
{
    int size;
    state *states;
} ht;

// Representation of a computation
typedef struct computation{
    char *tape;
    int state;
    int steps;
    int offset;
} computation;

// Functions prototypes
void parse();
void simulate();
void fork_computation();

// Computation queue
typedef struct qn{
    computation comp;
    struct qn *prev;
} qn;

typedef struct queue{
    qn *head;
    qn *tail;
    int size;
} queue;

qn *dequeue(queue *Q);
void enqueue(queue *Q, qn *item);

ht MT; // Define the machine as a global variable
int MAX_MV; // Container for maximum steps

int main()
{
    char temp_input[CONFIG_DIM]; // Buffer to recognize input delimiters
    int scan_result, acc, old_size, temp;

    // Start reading input and parse transitions
    scanf("%s", temp_input);
    parse(); 

    // Read final states
    scanf("%s", temp_input);
    scan_result = scanf("%d", &acc);
    while (scan_result)
    {
        // If a final state is not in table create one
        if (acc >= MT.size)
        {
            old_size = MT.size;
            temp = acc + 1;
            MT.size = temp;
            MT.states = (state *)realloc(MT.states, (temp) * sizeof(state));
            // Initialize transitions to null and final states to FALSE
            while (old_size < MT.size)
            {
                MT.states[old_size].transitions = NULL;
                MT.states[old_size].final = FALSE;
                old_size++;
            }
        }
        MT.states[acc].final = TRUE;
        scan_result = scanf("%d", &acc);
    }

    // Read maximum number of moves
    scanf("%s", temp_input);
    scanf("%d", &MAX_MV);

    //Start reading strings
    scanf("%s\n", temp_input);
    simulate();
    return 0;
}

void parse()
{
    int t_start, t_end, scan_result, temp, old_size;
    char t_in, t_out, t_mv;
    transition *temp_tr, *new_tr;

    // Initialize MT
    MT.size = 1;
    MT.states = (state *)malloc(sizeof(state));
    MT.states[0].transitions = NULL;

    // Read input until end
    scan_result = scanf("%d %c %c %c %d", &t_start, &t_in, &t_out, &t_mv, &t_end);
    while (scan_result)
    {
        // Check if the size of the ht is enough
        temp = t_start + 1;
        if (temp > MT.size)
        {
            old_size = MT.size;
            MT.size = temp;
            MT.states = (state *)realloc(MT.states, temp * sizeof(state));
            // Initialize transitions to null
            while (old_size < MT.size)
            {
                MT.states[old_size].transitions = NULL;
                old_size++;
            }
        }

        // Create new transition according to input
        new_tr = (transition *)malloc(sizeof(transition));
        new_tr->in = t_in;
        new_tr->out = t_out;
        new_tr->move = t_mv == 'L' ? 0 : t_mv == 'S' ? 1 : 2;
        new_tr->target = t_end;
        new_tr->next = NULL;

        // Check if already exists a state
        temp_tr = MT.states[t_start].transitions;
        // If a state already exists insert the new one at the head
        if (temp_tr != NULL)
            new_tr->next = temp_tr;

        // Save data in ht
        MT.states[t_start].final = FALSE;
        MT.states[t_start].transitions = new_tr;

        // Read input
        scan_result = scanf("%d %c %c %c %d", &t_start, &t_in, &t_out, &t_mv, &t_end);
    }
}

void simulate()
{
    int i, j, sys_clk, fork_flag, cursor_offset, limit;
    const int TAPE_LEN = 2 * MAX_MV + 1;
    char in, *TAPE, *CENTER, *copy_helper;
    queue COMP_QUEUE;
    bool accepted, mv_overflow;  
    qn *q_temp, *q_val;
    transition *t_temp;

COMP_QUEUE.size = 0;
        COMP_QUEUE.head = COMP_QUEUE.tail = NULL;

     // Initialize tape (it will only be used the first time to read the input)
    TAPE = (char *)malloc(TAPE_LEN * sizeof(char));
    // initialize copy helper to backup tape content during forks
    copy_helper = (char *)malloc(TAPE_LEN * sizeof(char));
    CENTER = TAPE + MAX_MV;                             
    
    in = getchar();
    while (in != EOF)
    {
        // Free memory used by previous computations queue
        while(COMP_QUEUE.size){
            q_temp = dequeue(&COMP_QUEUE);
            free(q_temp->comp.tape);
            free(q_temp);
        }
        // Initialize computations queue
        COMP_QUEUE.size = 0;
        COMP_QUEUE.head = COMP_QUEUE.tail = NULL;
        
        // Set all the tape cells as blank symbols
        for (i = 0; i < TAPE_LEN; i++)
            TAPE[i] = '_';

        // Copy the input to the tape
        i = 0;
        while (in != '\n' && i <= MAX_MV)
        {
            if(in != -1) CENTER[i] = in; // Avoid putting invalid characters in tape
            i++;
            in = getchar();
        }

        // Finish reading the string if there are trailing characters
        if (in != '\n' && in != -1) while (in != '\n' && in != -1) in = getchar();        

        accepted = FALSE;
        mv_overflow = FALSE;

        // Add first computation (root) to the computations queue
        q_temp = (qn *)malloc(sizeof(qn)); // Allocate memory to store computation
        q_temp->comp.tape = (char *)malloc(TAPE_LEN * sizeof(char)); // Allocate memory to store tape
        memcpy(q_temp->comp.tape, TAPE, TAPE_LEN); // Copy tape content
        q_temp->comp.offset = MAX_MV; // Initialize cursor
        q_temp->comp.state = 0; // Set starting state
        q_temp->comp.steps = 0; // Initialize steps counter
        enqueue(&COMP_QUEUE, q_temp); // Add computation to queue

        sys_clk = 0; // Initialize system clock
        // Repeat until there are some computations in queue and the moves limit has not been reached
        while(COMP_QUEUE.size && sys_clk < MAX_MV){

            sys_clk++; // Increase system clock

             // Store the current queue size to avoid issues when forking or removing
            limit = COMP_QUEUE.size;
            for(i=0; i<limit; i++){                   
                q_val = dequeue(&COMP_QUEUE); // Get a computation from queue
                in = q_val->comp.tape[q_val->comp.offset]; // Read tape
                
                fork_flag = 0; // Reset fork flag
                t_temp = MT.states[q_val->comp.state].transitions; // Get state transations
                // Count how many transitions can handle the input
                while(t_temp != NULL){
                    if(t_temp->in == in) fork_flag++; 
                    t_temp = t_temp->next;                    
                }

                // Check if I reached the limit of simulation steps
                if(q_val->comp.steps > MAX_MV){ 
                    mv_overflow = TRUE;
                    goto simulation_end;
                }

                if(!fork_flag){
                    // No transition can handle the input. 
                    // Check if I am in a final state
                    accepted = MT.states[q_val->comp.state].final;
                    // Then delete memory used by computation
                    free(q_val->comp.tape);
                    free(q_val);                    
                    if(accepted == TRUE) goto simulation_end; // If I am in a final state end simulation
                }
                else{
                    // The first transition I encounter is the evolution of the current one
                    // The other transitions are added to queue
                    t_temp = MT.states[q_val->comp.state].transitions;
                    // Find the first suitable transition
                    while(t_temp != NULL && t_temp->in != in) t_temp = t_temp->next;
                    // Back up the tape content and the cursor if I need to fork
                    if(fork_flag > 1){
                        memcpy(copy_helper, q_val->comp.tape, TAPE_LEN);
                        cursor_offset = q_val->comp.offset;
                    }

                    q_val->comp.tape[q_val->comp.offset] = t_temp->out; // Write tape
                    q_val->comp.offset += ((int)t_temp->move -1); // Move cursor
                    q_val->comp.state = t_temp->target; // Change state
                    q_val->comp.steps++; // Update computation steps
                    t_temp = t_temp->next; // Go to next transition

                    // Put computation back in queue
                    enqueue(&COMP_QUEUE, q_val); 

                    // Fork the remaining transitions
                    for(j=1; j<fork_flag; j++){ 
                        while(t_temp != NULL && t_temp->in != in) t_temp = t_temp->next; // Find suitable transition
                        
                        q_temp = (qn *)malloc(sizeof(qn)); // Create new computation
                        q_temp->comp.tape = (char *)malloc(TAPE_LEN * sizeof(char)); // Allocate memory for forked tape
                        memcpy(q_temp->comp.tape, copy_helper, TAPE_LEN); // Copy input tape
                        q_temp->comp.steps = q_val->comp.steps; // (Steps of father are already incremented by 1)
                        q_temp->comp.offset = cursor_offset; // Move cursor according to previous cursor offset

                        q_temp->comp.tape[q_temp->comp.offset] = t_temp->out; // Write to tape
                        q_temp->comp.offset += ((int)t_temp->move - 1); // Move cursor                        
                        q_temp->comp.state = t_temp->target; // Change state                        

                        // Put computation in queue
                        enqueue(&COMP_QUEUE, q_temp);
                        t_temp = t_temp->next; // Go to next transition
                    }                    
                }
            }
        }
        simulation_end:

        // Print output
        // I previously accepted the string:
        if(accepted) printf("1\n"); 
        // A branch couldn't end before the maximum number of steps or there are still some computations in queue:
        else if(COMP_QUEUE.size || mv_overflow) printf("U\n"); 
        // No computation is left in queue and all have ended before the limit:
        else printf("0\n");

        in = getchar();
    }

    // Free memory
    free(TAPE);
    free(copy_helper);
}

// Functions to manage queue
void enqueue(queue *Q, qn *item){
    item->prev = NULL;
    if(!Q->size) // empty queue
        Q->head = item;
    else
        Q->tail->prev = item;
    Q->tail = item; // Insert item as tail
    Q->size++; // Increase queue size
}

qn *dequeue(queue *Q){
    qn *item;
    if(!Q->size) return NULL;
    item = Q->head; // Get element at head
    Q->head = item->prev; // Update head
    Q->size--; // Decrease queue size
    return item;
}