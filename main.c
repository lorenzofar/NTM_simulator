#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONFIG_DIM 3

// Useful definitions
typedef enum
{
    TRUE,
    FALSE
} bool;

// Transition representation
typedef struct transition
{
    int target;
    char in;
    char out;
    char move;
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
    char *tape_start;
    char *tape_end;
    char *cursor;
    int state;
    int steps;
} computation;

// Functions prototypes
void parse();
void simulate();
int listTransitions();

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
            printf("I need to reallocate the arry before proceeding\n");
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
        new_tr->move = t_mv;
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
        printf("State %d%s:\n", i, MT.states[i].final == TRUE ? " - FINAL" : "");
        temp = MT.states[i].transitions;
        j = 0;
        while (temp != NULL)
        {
            count++;
            printf("tr%d: %c %c %c %d\n", j, temp->in, temp->out, temp->move, temp->target);
            temp = temp->next;
            j++;
        }
        printf("\n");
    }
    return count;
}

void simulate()
{
    int state_curr = 0, scan_result, i, current_state; // Set the starting state
    char in, *TAPE, *CENTER, *END, *T;
    const queue *Q_ADDR = &COMP_QUEUE;

    const int transitions_count = listTransitions();
    printf("There are %d transitions\n", transitions_count);

    TAPE = (char *)malloc((2 * MAX_MV + 1) * sizeof(char)); // initialize tape
    T = CENTER = TAPE + MAX_MV;                             // Head that points to the center of tape
    END = TAPE + 2 * MAX_MV;

    printf("The tape starts at %p, the center is at %p, the end is at %p\n", TAPE, CENTER, END);

    in = getchar();
    while (in != EOF)
    {
        // Set the current state to 0 (default starting state)
        current_state = 0;

        // Initialize computations queue
        COMP_QUEUE.size = 0;
        COMP_QUEUE.head = NULL;
        COMP_QUEUE.tail = NULL;

        // Set all the tape cells as blank symbols
        for (i = 0; i <= 2 * MAX_MV; i++)
            TAPE[i] = '_';

        // Copy the input to the tape
        i = 0;
        while (in != '\n' && i <= MAX_MV)
        {
            CENTER[i++] = in;
            in = getchar();
        }

        // Finish reading the string if there are trailing characters
        if (in != '\n')
            while (in != '\n')
                in = getchar();

        // Print tape content
        /*i = 0;
        while (i <= 2 * MAX_MV)
            printf("%c", TAPE[i++]);
        
        printf("\n\n");*/

        // Simulate
        // The head is T, which moves according to parsed transitions



        in = getchar();
    }
}

void enqueue(queue *Q, qn *item){
    if(!Q->size) // empty queue
        Q->head = item;
    else
        Q->tail->prev = item;
    Q->tail = item; // Insert item as tail
    Q->size++; // Increase queue size
    printf("Added item to queue, size is now %d\n", Q->size);
}

qn *dequeue(queue *Q){
    qn *item;
    if(!Q->size) return NULL;
    item = Q->head; // Get element at head
    Q->head = item->prev; // Update head
    Q->size--; // Decrease queue size
    return item;
}