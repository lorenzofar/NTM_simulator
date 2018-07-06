#include<stdio.h>
#include<stdlib.h>

#define CONFIG_DIM 3

// Useful definitions
typedef enum{
    TRUE, FALSE
} bool;

// Transition representation
typedef struct transition{
    int target;
    char in;
    char out;
    char move;
    struct transition *next;
} transition;

// State representation
typedef struct state{
    bool final;
    transition *transitions;
} state;

// Table to store data
typedef struct{
    int size;
    state *states;
} ht;

// Functions prototypes
void parse();
void visualizeTransitions();

ht MT;

int main(){
    char temp_input[CONFIG_DIM];
    scanf("%s", temp_input);
    parse();
    visualizeTransitions();
    scanf("%s", temp_input);
    return 0;
}

void parse(){
    int t_start, t_end, scan_result, temp, temp_char, old_size;
    char t_in, t_out, t_mv;
    transition *temp_tr, *new_tr;
    // Initialize MT
    MT.size = 1;
    MT.states = (state *)malloc(sizeof(state));
    MT.states[0].transitions = NULL;

    // Read input until end
    scan_result = scanf("%d %c %c %c %d", &t_start, &t_in, &t_out, &t_mv, &t_end);    
    while(scan_result){

        // Check if the size of the ht is enough
        temp = t_start + 1;
        if(temp > MT.size){
            old_size = MT.size;
            MT.size = temp;
            MT.states = (state *)realloc(MT.states, temp * sizeof(state));
            // Initialize transitions to null
            for(old_size; old_size < MT.size; old_size++)
                MT.states[old_size].transitions = NULL;
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
        if(temp_tr != NULL);
            new_tr->next = temp_tr;

        // Save data
        MT.states[t_start].final = FALSE;
        MT.states[t_start].transitions = new_tr;
        
        // Read input
        scan_result = scanf("%d %c %c %c %d", &t_start, &t_in, &t_out, &t_mv, &t_end);   
    }
}

void visualizeTransitions(){
    int i, j;
    transition *temp;
    for(i=0; i<MT.size; i++){
        printf("State %d:\n", i);
        temp = MT.states[i].transitions;
        j = 0;
        while(temp != NULL){
            printf("tr%d: %c %c %c %d\n", j, temp->in, temp->out, temp->move, temp->target);
            temp = temp->next;
            j++;
        }
        printf("\n");
    }
}