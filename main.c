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

//TODO: Check if I still need to consider the MT size

int main()
{
    char temp_input[CONFIG_DIM]; // Buffer to recognize input delimiters
    int scan_result;
    int acc, temp;
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
    while (finals != NULL)
    {
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
    transition *new_tr;

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
        if (temp > MT.size) // I need to expand the MT
        {
            old_size = MT.size;
            MT.size = temp;
            MT.states = (transition **)realloc(MT.states, temp * sizeof(transition *));
            // Initialize transitions to null
            for (old_size; old_size < MT.size; old_size++)
                MT.states[old_size] = NULL;
        }

        // Create new transition according to input
        new_tr = (transition *)malloc(sizeof(transition));
        new_tr->in = t_in;
        new_tr->out = t_out;
        new_tr->move = t_mv == 'L' ? 0 : t_mv == 'S' ? 1 : 2;
        new_tr->target = t_end;
        new_tr->next = NULL;

        new_tr->next = MT.states[t_start]; // Link transition with list
        MT.states[t_start] = new_tr;       // Insert transition at top

        // Read input
        scan_result = scanf("%d %c %c %c %d", &t_start, &t_in, &t_out, &t_mv, &t_end);
    }
}

void simulate()
{
    int i, j, cursor_helper, sys_clk, limit, fork_flag, current_state;
    char in;
    queue COMP_QUEUE;
    bool accepted, mv_overflow;
    qn *forked_comp, *currrent_comp;
    transition *t_temp;

    tape_cell *cell, *cursor;

    in = getchar();
    while (in != EOF)
    {
        // Initialize computations queue
        COMP_QUEUE.size = 0;
        COMP_QUEUE.head = COMP_QUEUE.tail = NULL;

        // Add first computation (root) to the computations queue
        currrent_comp = (qn *)malloc(sizeof(qn));                                // Allocate memory to store computation
        currrent_comp->comp.tape = (tape_cell *)malloc(sizeof(tape_cell));       // Allocate memory to store tape (just one tape)
        currrent_comp->comp.tape->left = currrent_comp->comp.tape->right = NULL; // Set tape bounds
        currrent_comp->comp.state = 0;                                           // Set starting state
        currrent_comp->comp.steps = 0;                                           // Initialize steps counter

        // While reading tape, every time i read a character allocate a new tape cell
        // If I exceed the maximum number of moves, stop reading (read trailing characters then)
        i = 0;                                        // Track tape size
        cursor = currrent_comp->comp.tape;            // Set cursor to tape start
        while (in != '\n' && in != -1 && i <= MAX_MV) // Limit the tape size to the maximum number of moves
        {
            cursor->val = in;                              // Write the current character to tape
            cell = (tape_cell *)malloc(sizeof(tape_cell)); // Create a new cell
            cell->val = BLANK;                             // Initialize cell with blank character
            cell->right = NULL;                            // Set cell bounds
            cell->left = cursor;                           // Point to the current cursor
            cursor->right = cell;                          // Link tape with cell
            cursor = cell;                                 // Update cursor
            i++;                                           // Increase size counter
            in = getchar();                                // Read next character
        }

        // Finish reading the string if there are trailing characters
        if (in != '\n' && in != -1)
            while (in != '\n' && in != -1)
                in = getchar();

        enqueue(&COMP_QUEUE, currrent_comp); // Add computation to queue

        accepted = false;    // Initialize accepted flag
        mv_overflow = false; // Initialize computation overflow flag
        sys_clk = 0;         // Initialize system clock

        // Repeat until no computation is left in queue and the moves limit has been reached
        while (COMP_QUEUE.size && sys_clk < MAX_MV)
        {
            sys_clk++; // Increase system clock

            // Store the current queue size to avoid issues when forking or removing
            limit = COMP_QUEUE.size;
            for (i = 0; i < limit; i++)
            {
                currrent_comp = dequeue(&COMP_QUEUE); // Get a computation from queue
                current_state = currrent_comp->comp.state;
                // Check if I am in a final state
                accepted = check_final(current_state);
                if (accepted)
                {
                    // Delete tape
                    delete_tape(currrent_comp->comp.tape);
                    free(currrent_comp);
                    goto simulation_end;
                }

                in = currrent_comp->comp.tape->val; // Get the character under the cursor

                fork_flag = 0; // Reset fork flag
                if (current_state >= MT.size)
                    t_temp = NULL;
                else
                    t_temp = MT.states[current_state]; // Get state transations

                // Count how many transitions can handle the input

                for (t_temp; t_temp != NULL; t_temp = t_temp->next)
                    if (t_temp->in == in)
                        fork_flag++;

                // Check if I reached the limit of simulation steps
                // If I find an overflow here, it means that in the previous step no computation accepted the string
                // So it's safe to end the simulation and print U
                if (currrent_comp->comp.steps > MAX_MV)
                {
                    mv_overflow = true;
                    delete_tape(currrent_comp->comp.tape);
                    free(currrent_comp);
                    goto simulation_end;
                }

                if (!fork_flag)
                {
                    // No transition can handle the input.
                    delete_tape(currrent_comp->comp.tape); // Delete tape of computation
                    free(currrent_comp);                   // Delete computation
                }
                else
                {
                    // The first transition I encounter is the evolution of the current one
                    // The other transitions are added to queue
                    t_temp = MT.states[current_state]; // Point to the first transition
                    // Find the first suitable transition
                    while (t_temp != NULL && t_temp->in != in)
                        t_temp = t_temp->next;
                    t_temp = t_temp->next; // Do not consider first transition (it will be simulated later)

                    // Fork the remaining transitions
                    for (j = 1; j < fork_flag; j++)
                    {
                        while (t_temp != NULL && t_temp->in != in)
                            t_temp = t_temp->next; // Find a suitable transition

                        forked_comp = (qn *)malloc(sizeof(qn));                          // Create new computation
                        forked_comp->comp.tape = (tape_cell *)malloc(sizeof(tape_cell)); // Allocate memory for forked tape (one cell)
                        copy_tape(forked_comp->comp.tape, currrent_comp->comp.tape);     // Copy input tape

                        forked_comp->comp.tape->val = t_temp->out; // Write to tape

                        forked_comp->comp.steps = currrent_comp->comp.steps; // (Steps of father are already incremented by 1

                        // Move cursor, and also check wheter I need to allocate new memory
                        if (t_temp->move == 0)
                        { // LEFT
                            if (forked_comp->comp.tape->left == NULL)
                            {
                                cell = (tape_cell *)malloc(sizeof(tape_cell));
                                cell->left = NULL;
                                cell->val = BLANK;
                                cell->right = forked_comp->comp.tape;
                                forked_comp->comp.tape->left = cell;
                            }
                            forked_comp->comp.tape = forked_comp->comp.tape->left; // Move cursor
                        }
                        else if (t_temp->move == 2)
                        { // RIGHT
                            if (forked_comp->comp.tape->right == NULL)
                            {
                                cell = (tape_cell *)malloc(sizeof(tape_cell));
                                cell->right = NULL;
                                cell->val = BLANK;
                                cell->left = forked_comp->comp.tape;
                                forked_comp->comp.tape->right = cell;
                            }
                            forked_comp->comp.tape = forked_comp->comp.tape->right; // Move cursor
                        }

                        forked_comp->comp.state = t_temp->target; // Change state

                        // Put computation in queue
                        enqueue(&COMP_QUEUE, forked_comp);
                        t_temp = t_temp->next; // Go to next transition
                    }

                    // Get transitions root
                    t_temp = MT.states[currrent_comp->comp.state];
                    // Find the first suitable transition
                    while (t_temp != NULL && t_temp->in != in)
                        t_temp = t_temp->next;

                    currrent_comp->comp.tape->val = t_temp->out; // Write tape

                    // Check direction
                    // If I remain still, do nothing
                    // Check if I am at the edge of the tape
                    // In case allocate memory to store a new cell
                    if (t_temp->move == 0)
                    { // LEFT
                        if (currrent_comp->comp.tape->left == NULL)
                        {
                            cell = (tape_cell *)malloc(sizeof(tape_cell));
                            cell->right = currrent_comp->comp.tape;
                            cell->left = NULL;
                            cell->val = BLANK;
                            currrent_comp->comp.tape->left = cell;
                        }
                        currrent_comp->comp.tape = currrent_comp->comp.tape->left; // Move cursor
                    }
                    else if (t_temp->move == 2)
                    { // RIGHT
                        if (currrent_comp->comp.tape->right == NULL)
                        {
                            cell = (tape_cell *)malloc(sizeof(tape_cell));
                            cell->left = currrent_comp->comp.tape;
                            cell->right = NULL;
                            cell->val = BLANK;
                            currrent_comp->comp.tape->right = cell;
                        }
                        currrent_comp->comp.tape = currrent_comp->comp.tape->right; // Move cursor
                    }

                    currrent_comp->comp.state = t_temp->target; // Change state
                    currrent_comp->comp.steps++;                // Update computation steps

                    // Put computation back in queue
                    enqueue(&COMP_QUEUE, currrent_comp);
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
            forked_comp = dequeue(&COMP_QUEUE);
            delete_tape(forked_comp->comp.tape);
            free(forked_comp);
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