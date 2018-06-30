#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define CONFIG_DIM 3

// Define structures to represent states
typedef enum{
    L, R, S
} direction;

typedef struct state{
    short n;
    char in;
    char out;
    direction move;
    short target;
    struct state *children;
    struct state *sibling;
} state;

//Define structures to represent tape
typedef struct tape_cell{
    char val;
    struct tape_cell *prev;
    struct tape_cell *next;
} tape_cell;

tape_cell TAPE; 

void parse();

int main(){
    //Initialize tape
    
    //Start reading input
    //Because the different parts are delimited by tr / acc / max, I just use a temporary string of lenght 3
    char temp[CONFIG_DIM];
    scanf("%s", temp);
    printf("%s", temp);
    parse();
    scanf("%s", temp);
    printf("%s", temp);
    return 0;
}

void parse(){
    int t_start, t_end, i, scan_result;
    char t_in, t_out, t_mv;
    printf("PARSING\n");
    scan_result = scanf("%d %c %c %c %d", &t_start, &t_in, &t_out, &t_mv, &t_end);
    while(scan_result){
        printf("%d %c %c %c %d\t scanf result: %d\n", t_start, t_in, t_out, t_mv, t_end, scan_result);
        scan_result = scanf("%d %c %c %c %d", &t_start, &t_in, &t_out, &t_mv, &t_end);   
    }
}
