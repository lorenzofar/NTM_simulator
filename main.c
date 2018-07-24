#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define CONFIG_DIM 3  // Buffer to store input section bounds
#define BLOCK_SIZE 48 // Make blocks of 47 chars in order to have a struct that takes up 64 bytes
#define BLANK '_'     // Blank character according to specifications

typedef struct transition
{
    int target;              // The state the transition points to
    char in;                 // Input character
    char out;                // Character written to tape
    char move;               // Direction of movement
    struct transition *next; // List implementation
} transition;

typedef struct
{
    int size;            // Number of states
    transition **states; // List of transitions for each state
} ht;

typedef struct tape_block
{
    struct tape_block *left;  // Pointer to left block
    struct tape_block *right; // Pointer to right block
    char cells[BLOCK_SIZE];   // Array of characters (tape block)
} tape_block;

typedef struct computation
{
    tape_block *tape; // Own tape of computation
    char block_pos;   // Position inside block
    int state;        // Current state
    int steps;        // Steps performed by computation
} computation;

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

void parse();
void simulate();
void copy_tape(tape_block *dest, tape_block *src);
void delete_tape(tape_block *tape);
void fill_tape(char *tape);
void fork_computation();
qn *dequeue(queue *Q);
void enqueue(queue *Q, qn *item);

ht MT;           // Define the machine as a global variable
int MAX_MV;      // Container for maximum steps
bool *finals;    // List of final states
int finals_size; // Size of final states

int main()
{
    char temp_input[CONFIG_DIM]; // Buffer to recognize input delimiters
    int acc, temp, old_size;
    transition *t_a, *t_b;

    scanf("%s", temp_input); // Start reading input
    parse();                 // Parse transitions

    scanf("%s", temp_input); // Parse delimiter

    finals_size = MT.size;                               // Get number of parsed states
    finals = (bool *)malloc(finals_size * sizeof(bool)); // Create array of final states
    for (temp = 0; temp < finals_size; temp++)           // Initialize final states to false
        finals[temp] = false;

    temp = scanf("%d", &acc); // Get first final state
    while (temp)              // Continue reading until there are final states
    {
        if (acc >= finals_size)                                           // Check if I need to expand array
        {                                                                 // I need to expand the array
            old_size = finals_size;                                       // Get current size
            finals_size = acc + 1;                                        // Update size
            finals = (bool *)realloc(finals, finals_size * sizeof(bool)); // Reallocate and expand the array
            for (old_size; old_size < finals_size; old_size++)            // Set new states to false
                finals[old_size] = false;
        }
        finals[acc] = true;
        temp = scanf("%d", &acc);
    }

    scanf("%s", temp_input); // Parse delimiter
    scanf("%d", &MAX_MV);    // Read maximum number of moves

    scanf("%s\n", temp_input); // Start reading strings
    simulate();                                 // Start simulation

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

    free(MT.states); // Free memory used by states
    free(finals);    // Free memory used by final states

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

        temp = t_start + 1;
        if (temp > MT.size)                                                             // Check if the size of the ht is enough
        {                                                                               // I need to expand the MT
            old_size = MT.size;                                                         // Get current size
            MT.size = temp;                                                             // Update size
            MT.states = (transition **)realloc(MT.states, temp * sizeof(transition *)); // Reallocate and expand the array
            for (old_size; old_size < MT.size; old_size++)                              // Initialize transitions to null
                MT.states[old_size] = NULL;
        }

        new_tr = (transition *)malloc(sizeof(transition)); // Create new transition
        new_tr->in = t_in;                                 // Set input character
        new_tr->out = t_out;                               // Set output character
        new_tr->move = t_mv;                               // Set move direction
        new_tr->target = t_end;                            // Set target state
        new_tr->next = MT.states[t_start];                 // Link transition with list
        MT.states[t_start] = new_tr;                       // Insert transition at top

        scan_result = scanf("%d %c %c %c %d", &t_start, &t_in, &t_out, &t_mv, &t_end); // Read input
    }
}

void simulate()
{
    int i, j, cursor_helper, sys_clk, limit, fork_flag, current_state;
    char block_pos;
    char in;
    queue COMP_QUEUE;
    bool accepted, mv_overflow;
    qn *forked_comp, *current_comp;
    transition *t_temp;

    tape_block *block, *current_block;

    in = getchar();
    while (in != EOF)
    {
        printf("first char: %c - %d\n", in, in);
        // Initialize computations queue
        COMP_QUEUE.size = 0;
        COMP_QUEUE.head = COMP_QUEUE.tail = NULL;

        // Check if I have an empty string
        if (in == '\n')
            goto simulation_end;

        // Add first computation (root) to the computations queue
        current_comp = (qn *)malloc(sizeof(qn));                               // Allocate memory to store computation
        current_comp->comp.tape = (tape_block *)malloc(sizeof(tape_block));    // Allocate memory to store tape (just one tape)
        current_comp->comp.tape->left = current_comp->comp.tape->right = NULL; // Set tape bounds
        current_comp->comp.state = 0;                                          // Set starting state
        current_comp->comp.steps = 0;                                          // Initialize steps counter

        // While reading tape, every time i read a character allocate a new tape cell
        // If I exceed the maximum number of moves, stop reading (read trailing characters then)
        i = 0;
        block_pos = 0;                                // Track tape size
        current_block = current_comp->comp.tape;      // Set cursor to tape start
        fill_tape(current_block->cells);              // Initialize tape
        while (in != '\n' && in != -1 && i <= MAX_MV) // Limit the tape size to the maximum number of moves
        {
            current_block->cells[block_pos] = in; // Write the current character to tape
            block_pos++;                          // Increase position inside block
            if (block_pos == BLOCK_SIZE)          // Check if I reached the end of the block
            {
                block = (tape_block *)malloc(sizeof(tape_block)); // Create a new block
                block->right = NULL;                              // Set right of block to NULL
                block->left = current_block;                      // Link new block with current block
                current_block->right = block;                     // Link current block with new block
                fill_tape(block->cells);                          // Initialize tape with blank characters
                current_block = block;                            // Update current block
                block_pos = 0;                                    // Set cursor at beginning of block
            }
            i++;
            in = getchar(); // Read next character
        }
        block_pos = current_comp->comp.block_pos = 0; // Set position in block of computation

        // Finish reading the string if there are trailing characters
        if (in != '\n' && in != -1)
            while (in != '\n' && in != -1)
                in = getchar();

        enqueue(&COMP_QUEUE, current_comp); // Add computation to queue

        accepted = false;    // Initialize accepted flag
        mv_overflow = false; // Initialize computation overflow flag
        sys_clk = 0;         // Initialize system clock

        // Repeat until no computation is left in queue and the moves limit has been reached
        while (COMP_QUEUE.size && sys_clk < MAX_MV)
        {
            sys_clk++; // Increase system clock

            limit = COMP_QUEUE.size; // Store the current queue size to avoid issues when forking or removing
            for (i = 0; i < limit; i++)
            {
                current_comp = dequeue(&COMP_QUEUE);      // Get a computation from queue
                current_state = current_comp->comp.state; // Get current state

                accepted = current_state < finals_size && finals[current_state]; // Check if I am in a final state
                if (accepted)
                {
                    delete_tape(current_comp->comp.tape); // Delete tape
                    free(current_comp);                   // Free memory used by computation
                    goto simulation_end;                  // Exit cycle
                }

                block_pos = current_comp->comp.block_pos;       // Get the position inside block
                in = current_comp->comp.tape->cells[block_pos]; // Get the character under the cursor

                fork_flag = 0; // Reset fork flag
                if (current_state >= MT.size)
                    t_temp = NULL;
                else
                    t_temp = MT.states[current_state]; // Get state transitions

                for (t_temp; t_temp != NULL; t_temp = t_temp->next) // Count how many transitions can handle the input
                    if (t_temp->in == in)
                        fork_flag++;

                // Check if I reached the limit of simulation steps
                // If I find an overflow here, it means that in the previous step no computation accepted the string
                // So it's safe to end the simulation and print U
                if (current_comp->comp.steps > MAX_MV)
                {
                    mv_overflow = true;
                    delete_tape(current_comp->comp.tape);
                    free(current_comp);
                    goto simulation_end;
                }

                if (!fork_flag)
                {
                    // No transition can handle the input.
                    delete_tape(current_comp->comp.tape); // Delete tape of computation
                    free(current_comp);                   // Delete computation
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

                    for (j = 1; j < fork_flag; j++) // Fork the remaining transitions
                    {
                        block_pos = current_comp->comp.block_pos;
                        while (t_temp != NULL && t_temp->in != in)
                            t_temp = t_temp->next; // Find a suitable transition

                        forked_comp = (qn *)malloc(sizeof(qn));                            // Create new computation
                        forked_comp->comp.tape = (tape_block *)malloc(sizeof(tape_block)); // Allocate memory for forked tape (one cell)
                        copy_tape(forked_comp->comp.tape, current_comp->comp.tape);        // Copy input tape

                        forked_comp->comp.tape->cells[block_pos] = t_temp->out; // Write to tape

                        forked_comp->comp.steps = current_comp->comp.steps; // (Steps of father are already incremented by 1)
                        // Move cursor, and also check wheter I need to allocate new memory
                        if (t_temp->move == 'L')
                            if (block_pos == 0)
                            { // I need to change block
                                if (forked_comp->comp.tape->left == NULL)
                                {
                                    block = (tape_block *)malloc(sizeof(tape_block)); // Create new block
                                    fill_tape(block->cells);                          // Initialize block with blank characters
                                    block->left = NULL;                               // Set block left bound
                                    block->right = current_block;                     // Link block with tape
                                    forked_comp->comp.tape->left = block;             // Link tape with block
                                }
                                forked_comp->comp.tape = forked_comp->comp.tape->left; // Update current block
                                block_pos = BLOCK_SIZE - 1;                            // Set position at the end of block
                            }
                            else
                                block_pos--;
                        else if (t_temp->move == 'R')
                        {
                            if (block_pos == BLOCK_SIZE - 1) // I need to change block
                            {
                                if (forked_comp->comp.tape->right == NULL)
                                {
                                    block = (tape_block *)malloc(sizeof(tape_block)); // Create new block
                                    fill_tape(block->cells);                          // Initialize block with blank characters
                                    block->right = NULL;                              // Set block right bound
                                    block->left = current_block;                      // Link block with tape
                                    forked_comp->comp.tape->right = block;            // Link tape with block
                                }
                                forked_comp->comp.tape = forked_comp->comp.tape->right; // Update current block
                                block_pos = 0;                                          // Set position at beginning of block
                            }
                            else
                                block_pos++;
                        }

                        forked_comp->comp.state = t_temp->target; // Change state
                        forked_comp->comp.block_pos = block_pos;  // Update position in block
                        enqueue(&COMP_QUEUE, forked_comp);        // Put computation in queue
                        t_temp = t_temp->next;                    // Go to next transition
                    }

                    t_temp = MT.states[current_comp->comp.state]; // Get transitions root

                    while (t_temp != NULL && t_temp->in != in) // Find the first suitable transition
                        t_temp = t_temp->next;

                    block_pos = current_comp->comp.block_pos;                // Get position inside block
                    current_comp->comp.tape->cells[block_pos] = t_temp->out; // Write tape

                    // Check direction
                    // If I remain still, do nothing
                    // Check if I am at the edge of a block, in case allocate memory to store a new block
                    if (t_temp->move == 'L')
                        if (block_pos == 0)
                        {
                            if (current_comp->comp.tape->left == NULL)
                            {
                                block = (tape_block *)malloc(sizeof(tape_block)); // Create new block
                                fill_tape(block->cells);                          // Initialize block with blank characters
                                block->left = NULL;                               // Set block left bound
                                block->right = current_comp->comp.tape;           // Link block with tape
                                current_comp->comp.tape->left = block;            // Link tape with block
                            }
                            current_comp->comp.tape = current_comp->comp.tape->left; // Update current block
                            block_pos = BLOCK_SIZE - 1;                              // Set position at end of block
                        }
                        else
                            block_pos--; // Update block position
                    else if (t_temp->move == 'R')
                    {
                        if (block_pos == BLOCK_SIZE - 1) // I need to change block
                        {
                            if (current_comp->comp.tape->right == NULL)
                            {
                                block = (tape_block *)malloc(sizeof(tape_block)); // Create new block
                                fill_tape(block->cells);                          // Initialize block with blank characters
                                block->right = NULL;                              // Set block right bound
                                block->left = current_comp->comp.tape;            // Link block with tape
                                current_comp->comp.tape->right = block;           // Link tape with block
                            }
                            current_comp->comp.tape = current_comp->comp.tape->right; // Update current block
                            block_pos = 0;                                            // Set position at beginning of block
                        }
                        else
                            block_pos++; // Update block positions
                    }

                    current_comp->comp.state = t_temp->target; // Change state
                    current_comp->comp.steps++;                // Update computation steps
                    current_comp->comp.block_pos = block_pos;  // Update position in block

                    enqueue(&COMP_QUEUE, current_comp); // Put computation back in queue
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

void copy_cells(char *dest, char *src)
{
    char i;
    for (i = 0; i < BLOCK_SIZE; i++)
        dest[i] = src[i];
}

void copy_tape(tape_block *dest, tape_block *src)
{
    tape_block *temp_s, *temp_d, *new;

    // Copy the stuff in central block
    copy_cells(dest->cells, src->cells);

    dest->left = dest->right = NULL;
    temp_s = src->left;
    temp_d = dest;
    while (temp_s != NULL)
    {
        new = (tape_block *)malloc(sizeof(tape_block)); // Allocate new cell
        new->left = NULL;                               // Set left / right pointers
        new->right = temp_d;
        copy_cells(new->cells, temp_s->cells);
        temp_d->left = new;
        temp_d = temp_d->left; // Move
        temp_s = temp_s->left;
    }
    temp_s = src->right;
    temp_d = dest;
    while (temp_s != NULL)
    {
        new = (tape_block *)malloc(sizeof(tape_block)); // Allocate new cell
        new->right = NULL;                              // Set left / right pointers
        new->left = temp_d;
        copy_cells(new->cells, temp_s->cells);
        temp_d->right = new;
        temp_d = temp_d->right; // Move
        temp_s = temp_s->right;
    }
}

void delete_tape(tape_block *tape)
{
    tape_block *temp, *helper;
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

void fill_tape(char *tape)
{
    char i;
    for (i = 0; i < BLOCK_SIZE; i++)
        tape[i] = BLANK;
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