#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONFIG_DIM 3

// Useful definitions
typedef enum
{
    FALSE, 
    TRUE
} bool;

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
    char *cursor;
    char *tape_start;
    int state;
    int steps;
} computation;

// Functions prototypes
void parse();
void simulate();
int listTransitions();
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
queue COMP_QUEUE;
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
            //printf("I need to reallocate the arry before proceeding\n");
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
    const int TAPE_LEN = 2*MAX_MV + 1;
    char in, TAPE[TAPE_LEN], *CENTER, copy_helper[TAPE_LEN];
    queue *Q_ADDR = &COMP_QUEUE;
    bool accepted, mv_overflow;  

    qn *q_temp, *q_val;
    transition *t_head, *t_temp;

    //TAPE = (char *)malloc(TAPE_LEN * sizeof(char)); // initialize tape
    //copy_helper = (char *)malloc(TAPE_LEN * sizeof(char)); // initialize copy helper
    CENTER = TAPE + MAX_MV;                             // Head that points to the center of tape

    in = getchar();
    while (in != EOF)
    {
        sys_clk = 0; // Initialize step counter

        // Initialize computations queue
        COMP_QUEUE.size = 0;
        COMP_QUEUE.head = NULL;
        COMP_QUEUE.tail = NULL;

        // Set all the tape cells as blank symbols
        for (i = 0; i < TAPE_LEN; i++)
            TAPE[i] = '_';

        // Copy the input to the tape
        i = 0;
        while (in != '\n' && i <= MAX_MV)
        {
            CENTER[i] = in;
            i++;
            in = getchar();
        }
        CENTER[i] = '\0'; // Add string terminator

        // Finish reading the string if there are trailing characters
        if (in != '\n')
            while (in != '\n')
                in = getchar();

        // Print tape content
        /*i = 0;
        while (i <= TAPE_LEN - 1)
            printf("%c", TAPE[i++]);
        
        printf("\n\n");
        */

        // Simulate
        // The head is T, which moves according to parsed transitions

        // Add first computation (default) to the computation queue
        q_temp = (qn *)malloc(sizeof(qn));
        q_temp->comp.tape_start = TAPE;
        q_temp->comp.cursor = CENTER;
        q_temp->comp.state = 0;
        q_temp->comp.steps = 0;
        enqueue(Q_ADDR, q_temp);

        accepted = FALSE;
        mv_overflow = FALSE;

        while(COMP_QUEUE.size && sys_clk < MAX_MV && !accepted){

            sys_clk++;
            //printf("Phase %d\n", sys_clk);
            // Evaluate all computations present in queue
            limit = COMP_QUEUE.size;
            for(i=0; i<limit; i++){      
                //printf("Analyzing computation %d, queue size: %d\n", i, COMP_QUEUE.size);
                q_val = dequeue(Q_ADDR); // Get current computation
                in = *(q_val->comp.cursor); // read tape content
                //printf("I'm currently at state %d reading %c\n", q_val->comp.state, in);
                //printf("The content of tape is %s\n", q_val->comp.tape_start);

                // Check if there is a fork
                fork_flag = 0;
                t_temp = t_head = MT.states[q_val->comp.state].transitions; // Get available transations                
                while(t_temp != NULL){
                    if(t_temp->in == in) fork_flag++;
                    t_temp = t_temp->next;
                }

                if(!fork_flag){
                    //printf("There are no exiting arcs\n");
                     // There are no arcs from this state, check if I am in a final state otherwise end computation 
                     // (it will remain in non-acceptance state forever)
                    accepted = MT.states[q_val->comp.state].final;
                    // Delete computation
                    //printf("I need to delete computation at %p, tape starts at %p\n", q_val, q_val->comp.tape_start);
                    //free(q_val->comp.tape_start);
                    free(q_val);
                    
                    if(accepted)
                        break;
                     
                }
                else{
                    // The first transition I encounter is the evolution of the current one
                    // The other transitions are added to queue
                    t_temp = t_head;
                    while(t_temp != NULL && t_temp->in != in) t_temp = t_temp->next;
                    //printf("Found first transition\n"); // find the first transition
                    // t_temp now points to the first transition

                    // Back up the tape content and the cursor if I need to fork
                    if(fork_flag>1){
                        memcpy(copy_helper, q_val->comp.tape_start, TAPE_LEN);
                        cursor_offset = q_val->comp.cursor - q_val->comp.tape_start;
                    }

                    *(q_val->comp.cursor) = t_temp->out;
                    q_val->comp.cursor += ((int)t_temp->move - 1);
                    q_val->comp.state = t_temp->target;
                    //printf("The transition led me to state %d\n", q_val->comp.state);
                    q_val->comp.steps++; // Update computation steps
                    t_temp = t_temp->next; // go ahead

                    enqueue(Q_ADDR, q_val);

                    j = 1; // count inserted transitions
                    while(j < fork_flag){

                        // DO NOT MODIFIY TAPE CONTENT BEFORE FORKING

                        // Fork the remaining transitions
                        while(t_temp != NULL && t_temp->in != in) t_temp = t_temp->next; // Find transition
                        // Fork computation
                        // Copy the tape
                        q_temp = (qn *)malloc(sizeof(qn)); // Create new computation
                        q_temp->comp.tape_start = (char *)malloc(TAPE_LEN * sizeof(char));
                        //printf("Created tape at %p\n", q_temp->comp.tape_start);
                        memcpy(q_temp->comp.tape_start, copy_helper, TAPE_LEN); // copy input tape
                        //printf("The cursor offset is %d\n", cursor_offset);
                        q_temp->comp.cursor = q_temp->comp.tape_start + cursor_offset + ((int)t_temp->move - 1); // calculate cursor offset and apply movement
                        q_temp->comp.steps = q_val->comp.steps; // steps of father are already incremented by 1
                        q_temp->comp.state = t_temp->target;
                        //printf("The trans led me to state %d\n", q_temp->comp.state);
                        enqueue(Q_ADDR, q_temp);
                        t_temp = t_temp->next;
                        j++;
                    }
                    
                }
            }
            //printf("\n");
        }

        //printf("Simulation ended - result: %s\n", accepted == TRUE ? "accepted" : "rejected");       
        

        // When copying, create a copy of the tape and calculate the offset of the cursor


        // U is print if no computation end before the maximum number of steps
        // If I have a queue of size 0 means:
        // - All the computations have rejected the input
        // - All the computations have reached maximum number of steps
        // I can check mv_overflow and then decide
        // If size==0 and mv_overflow==0 then the input has to be rejected
        // If size==0 and mv_overflow==1 then the input is undetermined
        if(accepted) printf("1\n");
        else if(!COMP_QUEUE.size && !mv_overflow) printf("0\n");
        else printf("U\n");
        
        // If the input is accepted I stop earlier so I do not consider the case

        in = getchar();

        // FREE MEMORY!
    }

    //free(&TAPE);
    //free(&copy_helper);

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