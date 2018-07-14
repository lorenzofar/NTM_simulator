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
int listTransitions();

void print_tape(char *tape, int tape_len);

void simulate();
void fork_computation();

// Queue management 
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

ht MT;
int MAX_MV;

int main()
{
    char temp_input[CONFIG_DIM];
    int scan_result, acc, old_size, temp;
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
            ////printf("I need to reallocate the arry before proceeding\n");
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
        if (temp_tr != NULL)
            new_tr->next = temp_tr;

        // Save data
        MT.states[t_start].final = FALSE;
        MT.states[t_start].transitions = new_tr;

        // Read input
        scan_result = scanf("%d %c %c %c %d", &t_start, &t_in, &t_out, &t_mv, &t_end);
    }
}

int listTransitions()
{
    int i, j;
    int count = 0;
    transition *temp;
    for (i = 0; i < MT.size; i++)
    {
        //printf("State %d%s:\n", i, MT.states[i].final == TRUE ? " - FINAL" : "");
        temp = MT.states[i].transitions;
        j = 0;
        while (temp != NULL)
        {
            count++;
            //printf("tr%d: %c %c %d %d\n", j, temp->in, temp->out, temp->move, temp->target);
            temp = temp->next;
            j++;
        }
        //printf("\n");
    }
    return count;
}

void simulate()
{
    int i, j, sys_clk, fork_flag, cursor_offset, limit; // Set the starting state
    const int TAPE_LEN = 2*MAX_MV + 2;
    char in, *TAPE, *CENTER, *copy_helper;
    queue COMP_QUEUE;
    bool accepted, mv_overflow;  

    qn *q_temp, *q_val;
    transition *t_temp;

    COMP_QUEUE.size = 0;
    COMP_QUEUE.head = COMP_QUEUE.tail = NULL;

    TAPE = (char *)malloc(TAPE_LEN * sizeof(char)); // initialize tape
    copy_helper = (char *)malloc(TAPE_LEN * sizeof(char)); // initialize copy helper
    CENTER = TAPE + MAX_MV;                             // Head that points to the center of tape

    in = getchar();
    while (in != EOF)
    {
        // Initialize computations queue
        while(COMP_QUEUE.size){
            q_temp = dequeue(&COMP_QUEUE);
            free(q_temp->comp.tape);
            free(q_temp);
        }
        COMP_QUEUE.size = 0;
        COMP_QUEUE.head = COMP_QUEUE.tail = NULL;
        
        // Set all the tape cells as blank symbols
        for (i = 0; i < TAPE_LEN; i++)
            TAPE[i] = '_';

        // Copy the input to the tape
        i = 0;
        while (in != '\n' && i <= MAX_MV)
        {
            if(in != -1) CENTER[i] = in; // avoid putting invalid characters in tape
            i++;
            in = getchar();
        }
        CENTER[TAPE_LEN-1] = '\0'; // Add string terminator

        // Finish reading the string if there are trailing characters
        if (in != '\n' && in != -1) while (in != '\n' && in != -1) in = getchar();        

        accepted = FALSE;
        mv_overflow = FALSE;

        // Add first computation (default) to the computation queue
        q_temp = (qn *)malloc(sizeof(qn)); // Allocate memory to store computation
        q_temp->comp.tape = (char *)malloc(TAPE_LEN * sizeof(char)); // Allocate memory to store tape
        memcpy(q_temp->comp.tape, TAPE, TAPE_LEN); // Copy tape content
        q_temp->comp.offset = MAX_MV; // Initialize cursor
        q_temp->comp.state = 0;
        q_temp->comp.steps = 0;

        //print_tape(q_temp->comp.tape, TAPE_LEN);

        enqueue(&COMP_QUEUE, q_temp); // Add computation to queue

        sys_clk = 0; // Initialize step counter
        // Repeat until there are some computations in queue, the string has not been accepted and the moves limit has not been reached
        while(COMP_QUEUE.size && sys_clk < MAX_MV){

            sys_clk++; // Increase system clock

            limit = COMP_QUEUE.size; // Store the current size to avoid issues when forking or removing
            for(i=0; i<limit; i++){   
                
                q_val = dequeue(&COMP_QUEUE); // Get a computation from the queue
                in = q_val->comp.tape[q_val->comp.offset]; // read tape content
                
                fork_flag = 0; // reset fork flag
                t_temp = MT.states[q_val->comp.state].transitions; // Get state transations
                j=0;
                while(t_temp != NULL){
                    if(t_temp->in == in) fork_flag++; 
                    j++;
                    t_temp = t_temp->next;                    
                }
                //printf("I evaluated %d transitions in state %d, %d can handle the input %c\n", j, q_val->comp.state, fork_flag, in);

                if(q_val->comp.steps > MAX_MV){ // Check if I reached the maximum simulation steps
                    mv_overflow = TRUE;
                    goto simulation_end;
                }

                if(!fork_flag){
                     // There are no arcs from this state, check if I am in a final state otherwise end computation 
                     // Then delete memory used by computation
                    accepted = MT.states[q_val->comp.state].final;

                    if(COMP_QUEUE.size == 0){
                        //printf("this is the last computation, the tape is:\n");
                        //print_tape(q_val->comp.tape,TAPE_LEN);
                        //printf("and I'm reading: %c (%d)\n",q_val->comp.tape[q_temp->comp.offset] ,in);
                    }

                    free(q_val->comp.tape);
                    free(q_val);                    
                    if(accepted == TRUE) goto simulation_end;
                }
                else{
                    // The first transition I encounter is the evolution of the current one
                    // The other transitions are added to queue
                    t_temp = MT.states[q_val->comp.state].transitions;
                    while(t_temp != NULL && t_temp->in != in) t_temp = t_temp->next;
                    // t_temp now points to the first transition
                    // Back up the tape content and the cursor if I need to fork
                    if(fork_flag > 1){
                        memcpy(copy_helper, q_val->comp.tape, TAPE_LEN);
                        cursor_offset = q_val->comp.offset;
                    }

                    //printf("Simulating tr: %d->%d - %c->%c - mv: %d\n", q_val->comp.state,t_temp->target,q_val->comp.tape[q_val->comp.offset],t_temp->out,t_temp->move-1);
                    q_val->comp.tape[q_val->comp.offset] = t_temp->out;
                    q_val->comp.offset += ((int)t_temp->move -1);
                    q_val->comp.state = t_temp->target;
                    q_val->comp.steps++; // Update computation steps
                    t_temp = t_temp->next; // go ahead

                    //print_tape(q_val->comp.tape,TAPE_LEN);

                    enqueue(&COMP_QUEUE, q_val); // put computation back in queue

                    for(j=1; j<fork_flag; j++)  { // Fork the remaining transitions
                        while(t_temp != NULL && t_temp->in != in) t_temp = t_temp->next; // Find transition
                        
                        q_temp = (qn *)malloc(sizeof(qn)); // Create new computation
                        q_temp->comp.tape = (char *)malloc(TAPE_LEN * sizeof(char)); // Allocate memory for forked tape
                        memcpy(q_temp->comp.tape, copy_helper, TAPE_LEN); // copy input tape
                        q_temp->comp.steps = q_val->comp.steps; // steps of father are already incremented by 1
                        q_temp->comp.offset = cursor_offset; // move cursor according to previous cursor offset


                        q_temp->comp.tape[q_temp->comp.offset] = t_temp->out; // Write to tape
                        q_temp->comp.offset += ((int)t_temp->move - 1); // Move cursor                        
                        q_temp->comp.state = t_temp->target; // Move to new state                        

                        // MA DIO PORCO, FACCIO IL FORK MA NON SCRIVO UN CAZZO SUL NASTRO
                        //printf("Simulating tr: %d->%d - %c->%c - mv: %d\n", q_val->comp.state,t_temp->target,q_val->comp.tape[q_val->comp.offset],t_temp->out,t_temp->move-1);
                        //print_tape(q_temp->comp.tape,TAPE_LEN);

                        enqueue(&COMP_QUEUE, q_temp);
                        t_temp = t_temp->next;
                    }                    
                }
            }
            ////printf("\n");
        }
        simulation_end:

        ////printf("Simulation ended - result: %s\n", accepted == TRUE ? "accepted" : "rejected");       
        
        // U is print if no computation end before the maximum number of steps
        // If I have a queue of size 0 means:
        // - All the computations have rejected the input
        // - All the computations have reached maximum number of steps
        // I can check mv_overflow and then decide
        // If size==0 and mv_overflow==0 then the input has to be rejected
        // If size==0 and mv_overflow==1 then the input is undetermined
        //printf("Final situation:\nqueue_size: %d\naccepted: %d\nmv_overflow: %d\n",COMP_QUEUE.size, accepted, mv_overflow);
        if(accepted) printf("1\n");
        else if(COMP_QUEUE.size || mv_overflow) printf("U\n");
        else printf("0\n");
        // If the input is accepted I stop earlier so I do not consider the case

        in = getchar();

        // FREE MEMORY!
    }

    free(TAPE);
    free(copy_helper);

}

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


void print_tape(char *tape, int tape_len){
    int i = 0;
    while (i < tape_len){
        //printf("%c", tape[i]);
        i++;
    }    
    //printf("\n\n");
}