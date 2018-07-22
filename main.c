#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define CONFIG_DIM 3
#define BLANK '_'

// Define directions as enum in order to use them in sums
typedef enum
{
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

// Table to store data
typedef struct
{
    int size;
    transition **states;
} ht;

// Tape representation
typedef struct tape_cell
{
    struct tape_cell *left;
    struct tape_cell *right;
    char val;
} tape_cell;

// Representation of a computation
typedef struct computation
{
    tape_cell *tape;
    int state;
    int steps;
} computation;

// Functions prototypes
void parse();
void simulate();
void copy_tape(tape_cell *dest, tape_cell *src);
void delete_tape(tape_cell *tape);
bool check_final(int state);
void fork_computation();

// Computation queue
typedef struct qn
{
    computation comp;
    struct qn *prev;
} qn;

typedef struct queue
{
    qn *head;
    qn *tail;
    int size;
} queue;

typedef struct f_state
{
    int val;
    struct f_state *next;
} f_state;

qn *dequeue(queue *Q);
void enqueue(queue *Q, qn *item);

ht MT;                  // Define the machine as a global variable
int MAX_MV;             // Container for maximum steps
f_state *finals = NULL; // List of final states

int main()
{
    char temp_input[CONFIG_DIM]; // Buffer to recognize input delimiters
    int scan_result;
    int acc, old_size, temp;
    transition *t_a, *t_b;
    f_state *temp_final;

    // Start reading input and parse transitions
    scanf("%s", temp_input);
    parse();

    // Read final states
    scanf("%s", temp_input);
    scan_result = scanf("%d", &acc);
    while (scan_result)
    {
        temp_final = (f_state *)malloc(sizeof(f_state));
        temp_final->val = acc;
        temp_final->next = finals;
        finals = temp_final;
        scan_result = scanf("%d", &acc);
    }

    // Read maximum number of moves
    scanf("%s", temp_input);
    scanf("%d", &MAX_MV);

    //Start reading strings
    scanf("%s\n", temp_input);
    simulate();

    // Free memory used by transitions
    temp = 0;
    while (temp < MT.size)
    {
        t_a = MT.states[temp];
        while (t_a != NULL)
        {
            t_b = t_a->next;
            free(t_a);
            t_a = t_b;
        }
        temp++;
    }
    // Free memory used by states
    free(MT.states);
    while(finals != NULL){
        temp_final = finals->next;
        free(finals);
        finals = temp_final;
    } // Free memory used to store final states
    return 0;
}

void parse()
{
    int t_start, t_end, temp, old_size, scan_result;
    char t_in, t_out, t_mv;
    transition *temp_tr, *new_tr;

    // Initialize MT
    MT.size = 1;
    MT.states = (transition **)malloc(sizeof(transition *));
    MT.states[0] = NULL;

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
            MT.states = (transition **)realloc(MT.states, temp * sizeof(transition *));
            // Initialize transitions to null
            while (old_size < MT.size)
            {
                MT.states[old_size] = NULL;
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
        temp_tr = MT.states[t_start];
        // Insert the new state at the head
        new_tr->next = temp_tr;

        // Save data in ht
        MT.states[t_start] = new_tr;

        // Read input
        scan_result = scanf("%d %c %c %c %d", &t_start, &t_in, &t_out, &t_mv, &t_end);
    }
}

void simulate()
{
    int i, j, cursor_helper, sys_clk, limit, fork_flag;
    char in;
    queue COMP_QUEUE;
    bool accepted, mv_overflow;
    qn *q_temp, *q_val;
    transition *t_temp;

    tape_cell *cell, *cursor;

    in = getchar();
    while (in != EOF)
    {
        // Initialize computations queue
        COMP_QUEUE.size = 0;
        COMP_QUEUE.head = COMP_QUEUE.tail = NULL;

        // Add first computation (root) to the computations queue
        q_temp = (qn *)malloc(sizeof(qn));                          // Allocate memory to store computation
        q_temp->comp.tape = (tape_cell *)malloc(sizeof(tape_cell)); // Allocate memory to store tape (just one tape)
        q_temp->comp.tape->left = q_temp->comp.tape->right = NULL;
        q_temp->comp.state = 0;       // Set starting state
        q_temp->comp.steps = 0;       // Initialize steps counter
        enqueue(&COMP_QUEUE, q_temp); // Add computation to queue

        // While reading tape, every time i read a character allocate a new tape cell
        // If I exceed the maximum number of moves, stop reading (read trailing characters then)
        i = 0;
        cursor = q_temp->comp.tape;
        while (in != '\n' && in != -1 && i <= MAX_MV)
        {
            cursor->val = in;
            cell = (tape_cell *)malloc(sizeof(tape_cell));
            cell->val = BLANK;
            cell->right = NULL;
            cell->left = cursor; // Point to the current cursor
            cursor->right = cell;
            cursor = cell;
            i++;
            in = getchar();
        }

        // Finish reading the string if there are trailing characters
        if (in != '\n' && in != -1)
            while (in != '\n' && in != -1)
                in = getchar();

        accepted = false;
        mv_overflow = false;

        sys_clk = 0; // Initialize system clock
        // Repeat until there are some computations in queue and the moves limit has not been reached
        while (COMP_QUEUE.size && sys_clk < MAX_MV)
        {

            sys_clk++; // Increase system clock

            // Store the current queue size to avoid issues when forking or removing
            limit = COMP_QUEUE.size;
            for (i = 0; i < limit; i++)
            {
                q_val = dequeue(&COMP_QUEUE); // Get a computation from queue

                // Check if I am in a final state
                accepted = check_final(q_val->comp.state);
                if (accepted)
                {
                    // Delete tape
                    delete_tape(q_val->comp.tape);
                    free(q_val);
                    goto simulation_end;
                }

                in = q_val->comp.tape->val; // Get the character under the cursor

                fork_flag = 0; // Reset fork flag
                if (q_val->comp.state >= MT.size)
                    t_temp = NULL;
                else
                    t_temp = MT.states[q_val->comp.state]; // Get state transations
                // Count how many transitions can handle the input
                while (t_temp != NULL)
                {
                    if (t_temp->in == in)
                        fork_flag++;
                    t_temp = t_temp->next;
                }

                // Check if I reached the limit of simulation steps
                if (q_val->comp.steps > MAX_MV)
                {
                    mv_overflow = true;
                    delete_tape(q_val->comp.tape);
                    free(q_val);
                    goto simulation_end;
                }

                if (!fork_flag)
                {
                    // No transition can handle the input.
                    // Check if I am in a final state
                    delete_tape(q_val->comp.tape); // Delete tape of computation
                    free(q_val);                   // Delete computation
                }
                else
                {
                    // The first transition I encounter is the evolution of the current one
                    // The other transitions are added to queue
                    t_temp = MT.states[q_val->comp.state];
                    // Find the first suitable transition
                    while (t_temp != NULL && t_temp->in != in)
                        t_temp = t_temp->next;
                    t_temp = t_temp->next; // Do not consider first transition

                    // Fork the remaining transitions
                    for (j = 1; j < fork_flag; j++)
                    {
                        while (t_temp != NULL && t_temp->in != in)
                            t_temp = t_temp->next; // Find suitable transition

                        q_temp = (qn *)malloc(sizeof(qn));                          // Create new computation
                        q_temp->comp.tape = (tape_cell *)malloc(sizeof(tape_cell)); // Allocate memory for forked tape
                        copy_tape(q_temp->comp.tape, q_val->comp.tape);             // Copy input tape

                        q_temp->comp.tape->val = t_temp->out; // Write to tape

                        q_temp->comp.steps = q_val->comp.steps; // (Steps of father are already incremented by 1

                        // Move cursor, and also check wheter I need to allocate new memory
                        if (t_temp->move == 0)
                        { // LEFT
                            if (q_temp->comp.tape->left == NULL)
                            {
                                cell = (tape_cell *)malloc(sizeof(tape_cell));
                                cell->left = NULL;
                                cell->val = BLANK;
                                cell->right = q_temp->comp.tape;
                                q_temp->comp.tape->left = cell;
                            }
                            q_temp->comp.tape = q_temp->comp.tape->left; // Move cursor
                        }
                        else if (t_temp->move == 2)
                        { // RIGHT
                            if (q_temp->comp.tape->right == NULL)
                            {
                                cell = (tape_cell *)malloc(sizeof(tape_cell));
                                cell->right = NULL;
                                cell->val = BLANK;
                                cell->left = q_temp->comp.tape;
                                q_temp->comp.tape->right = cell;
                            }
                            q_temp->comp.tape = q_temp->comp.tape->right; // Move cursor
                        }

                        q_temp->comp.state = t_temp->target; // Change state

                        // Put computation in queue
                        enqueue(&COMP_QUEUE, q_temp);
                        t_temp = t_temp->next; // Go to next transition
                    }

                    // Get transitions root
                    t_temp = MT.states[q_val->comp.state];
                    // Find the first suitable transition
                    while (t_temp != NULL && t_temp->in != in)
                        t_temp = t_temp->next;

                    q_val->comp.tape->val = t_temp->out; // Write tape

                    // Check direction
                    // If I remain still, do nothing
                    // Check if I am at the edge of the tape
                    // In case allocate memory to store a new cell
                    if (t_temp->move == 0)
                    { // LEFT
                        if (q_val->comp.tape->left == NULL)
                        {
                            cell = (tape_cell *)malloc(sizeof(tape_cell));
                            cell->right = q_val->comp.tape;
                            cell->left = NULL;
                            cell->val = BLANK;
                            q_val->comp.tape->left = cell;
                        }
                        q_val->comp.tape = q_val->comp.tape->left; // Move cursor
                    }
                    else if (t_temp->move == 2)
                    { // RIGHT
                        if (q_val->comp.tape->right == NULL)
                        {
                            cell = (tape_cell *)malloc(sizeof(tape_cell));
                            cell->left = q_val->comp.tape;
                            cell->right = NULL;
                            cell->val = BLANK;
                            q_val->comp.tape->right = cell;
                        }
                        q_val->comp.tape = q_val->comp.tape->right; // Move cursor
                    }

                    q_val->comp.state = t_temp->target; // Change state
                    q_val->comp.steps++;                // Update computation steps

                    // Put computation back in queue
                    enqueue(&COMP_QUEUE, q_val);
                }
            }
        }
    simulation_end:

        // Print output
        // I previously accepted the string:
        if (accepted)
            printf("1\n");
        // A branch couldn't end before the maximum number of steps or there are still some computations in queue:
        else if (COMP_QUEUE.size || mv_overflow)
            printf("U\n");
        // No computation is left in queue and all have ended before the limit:
        else
            printf("0\n");

        // Free memory used by previous computations queue
        while (COMP_QUEUE.size)
        {
            q_temp = dequeue(&COMP_QUEUE);
            delete_tape(q_temp->comp.tape);
            free(q_temp);
        }

        in = getchar();
    }
}

bool check_final(int state)
{
    f_state *temp;
    temp = finals;
    while (temp != NULL)
    {
        if (temp->val == state)
            return true;
        temp = temp->next;
    }
    return false;
}

void copy_tape(tape_cell *dest, tape_cell *src)
{
    tape_cell *temp_s, *temp_d, *new;
    dest->val = src->val;
    dest->left = dest->right = NULL;
    temp_s = src->left;
    temp_d = dest;
    while (temp_s != NULL)
    {
        new = (tape_cell *)malloc(sizeof(tape_cell)); // Allocate new cell
        new->left = NULL;                             // Set left / right pointers
        new->right = temp_d;
        new->val = temp_s->val; // Write content
        temp_d->left = new;
        temp_d = temp_d->left; // Move
        temp_s = temp_s->left;
    }
    temp_s = src->right;
    temp_d = dest;
    while (temp_s != NULL)
    {
        new = (tape_cell *)malloc(sizeof(tape_cell)); // Allocate new cell
        new->right = NULL;                            // Set left / right pointers
        new->left = temp_d;
        new->val = temp_s->val; // Write content
        temp_d->right = new;
        temp_d = temp_d->right; // Move
        temp_s = temp_s->right;
    }
}

void delete_tape(tape_cell *tape)
{
    tape_cell *temp, *helper;
    temp = tape;
    while (temp->left != NULL)
        temp = temp->left; // Get the leftmost element
    while (temp != NULL)
    {
        helper = temp->right;
        free(temp);
        temp = helper;
    }
}

// Functions to manage queue
void enqueue(queue *Q, qn *item)
{
    item->prev = NULL;
    if (!Q->size) // empty queue
        Q->head = item;
    else
        Q->tail->prev = item;
    Q->tail = item; // Insert item as tail
    Q->size++;      // Increase queue size
}

qn *dequeue(queue *Q)
{
    qn *item;
    if (!Q->size)
        return NULL;
    item = Q->head;       // Get element at head
    Q->head = item->prev; // Update head
    Q->size--;            // Decrease queue size
    return item;
}