#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define DEBUG 0
#if DEBUG
#define DUMP_DBL(x) printf("line %d: %s = %.5f\n", __LINE__, #x, x)
#else
#define DUMP_DBL(x)
#endif

#define INITIAL 2 /*initial size of number array and block array*/
#define SEPERATOR "=======================================\n"
#define MINI_SEPERATOR "------------------------------------------------\n"
typedef struct {
    int row;
    int col;
} cell_t;

typedef cell_t data_t;
typedef struct node node_t;

struct node {
	data_t data;
	/*for queue counter*/
	int num;
	node_t *next;
	node_t *prev;
};
typedef struct {
	node_t *head;
	node_t *foot;
} list_t;

int mygetchar(void);
/*alistairs fns*/
list_t* make_empty_list(void);
int is_empty_list(list_t *list);
list_t* insert_at_foot(list_t *list, data_t value);
list_t* insert_at_head(list_t *list, data_t value) ;
data_t get_head(list_t *list);
data_t get_foot(list_t *list);
list_t* get_tail(list_t *list);
void free_list(list_t *list);
/*stage 0 fns*/
void get_dimensions(int dimension[]);
void exit_if_null(void *ptr, char *msg);
cell_t get_cell(int r_limit, int c_limit);
int get_number(int limit);
int block_grid(int **grid, int r_lim, int c_lim);
int** construct_grid(int row_len, int col_len);
void print_cell(cell_t cell);
void print_route(list_t* list);
list_t* get_route(list_t* list, int r_lim, int c_lim);
int is_valid_move(cell_t cell_1, cell_t cell_2);
int determine_status(cell_t i_cell, cell_t g_cell, list_t *list, int **grid);
void print_info(int row_lim, int col_lim, int n_blocks, cell_t I, cell_t G);
void print_status(int status);
void put_route_on_grid(int** grid, list_t *list, 
    int r_lim, int c_lim, int repair_mode);
void print_grid(int **grid, int r_lim, int c_lim);
/*stage 1 fns*/
cell_t find_block(int** grid, list_t* R);
int add_adj_cells(list_t *q, int row, int col, int r_lim, int c_lim,
    int counter, cell_t cell, int** grid, int**q_grid);
list_t* make_queue(list_t *q, cell_t start, int** grid, int r_lim, 
    int c_lim, int* repairable);
int add_cell(list_t *q, int x, char* name, cell_t cell, 
    int counter, int valid, int** grid, int** q_grid);
int cell_cmp(cell_t c1, cell_t c2);
list_t* make_bridge(list_t *dest, list_t *q);
list_t* repair_route(list_t* R, list_t* B, cell_t dead_end);
/*stage 2 fns*/
int** reset_grid(int** grid, int r_lim, int c_lim);
list_t *reset_list(list_t *list);
int prioritise_cell(cell_t curr, cell_t valid, cell_t node);

int
main(int argc, char *argv[]) {
    int dim[2], nblocks, **grid, status, *repairable, i, repairmode = 0,
    processing_input = 1, first_time = 1;
    cell_t i_cell, goal_cell, q_cell;
    list_t *route, *queue, *bridge;
    
    route = make_empty_list();
    queue = make_empty_list();
    bridge = make_empty_list();
    /*get the dimensions of the grid and store them in dim*/
    get_dimensions(dim);
    i_cell = get_cell(dim[0], dim[1]);
    goal_cell = get_cell(dim[0], dim[1]);
    grid = construct_grid(dim[0], dim[1]);
    
    /*let 2 denote I, let 3 denote G*/
    grid[i_cell.row][i_cell.col] = 2; grid[goal_cell.row][goal_cell.col] = 3; 
  
    /*input can go be as long as it wants*/
    while (processing_input) {
        assert(is_empty_list(queue));
        nblocks = block_grid(grid, dim[0], dim[1]);
        if (first_time) {
            /*stage 0 information*/
            print_info(dim[0], dim[1], nblocks, i_cell, goal_cell);
            route = get_route(route, dim[0], dim[1]);
            print_route(route);
        }
        else {
            /*keep resetting the route on the grid after the first time*/
            repairmode = 1;
        }
        status = determine_status(i_cell, goal_cell, route, grid);
        
        if (first_time) {
            print_status(status);
            printf("==STAGE 1%s", SEPERATOR);    
        }
        put_route_on_grid(grid, route, dim[0], dim[1], repairmode);
        print_grid(grid, dim[0], dim[1]);
        repairable = (int*)malloc(sizeof(int));
        *repairable = 1;
       
        if (status == 4) {
            /*for stage 2, we want to keep trying to repair it until we do or
            it fails*/
            while (status == 4) {
                q_cell = find_block(grid, route);
                DUMP_DBL(4.0);fflush(stdout);
                queue = make_queue(queue, q_cell, grid, dim[0], dim[1], repairable);
                DUMP_DBL(4.0);fflush(stdout);
                if (!*repairable) {
                    break;    
                }
                /*route is definitely repairable*/
                DUMP_DBL(4.0);fflush(stdout);
                bridge = make_bridge(bridge, queue);
                /*attach bridge to route to complete repair*/
                repair_route(route, bridge, q_cell);
                put_route_on_grid(grid, route, dim[0], dim[1], repairmode);
                queue = reset_list(queue);
                bridge->head = NULL;
                bridge->foot = NULL;
                status = determine_status(i_cell, goal_cell, route, grid);
            }
            if (*repairable) {
                printf("%s", MINI_SEPERATOR);
                print_grid(grid, dim[0], dim[1]);
                printf("%s", MINI_SEPERATOR);
                print_route(route);
                print_status(status = 5);
            } else {
                printf("%s", MINI_SEPERATOR);
                print_grid(grid, dim[0], dim[1]);
                printf("%s", MINI_SEPERATOR);
                printf("The route cannot be repaired!\n");
            }
        }
        if (feof(stdin)) {
            processing_input = 0;    
        }
        if (first_time && processing_input) {
            printf("==STAGE 2=======================================");  
            first_time = 0;
        }
        else {
            printf("================================================");
        }
        if (processing_input) {
            printf("\n");    
        }
        grid = reset_grid(grid, dim[0], dim[1]);
    }
    /*free all the stuff you've malloc'ed*/
    free(repairable);
    free_list(route);
    free(bridge);
    free_list(queue);
    for (i=0;i<dim[0];i++) {
        free(grid[i]);
    }
    free(grid);
	return 0;
}
/*======================================================================*/


/*STAGE 0*/
/*prints stage0 information*/
void
print_info(int row_lim, int col_lim, int n_blocks, cell_t I, cell_t G) {
    printf("==STAGE 0%s", SEPERATOR);
    printf("The grid has %d rows and %d columns.\n", row_lim, col_lim);
    printf("The grid has %d block(s).\n", n_blocks);
    printf("The initial cell in the grid is [%d,%d].\n", 
        I.row, I.col);
    printf("The goal cell in the grid is [%d,%d].\n",
        G.row, G.col);
    printf("The proposed route in the grid is:\n");
}
/*function designed by Alistair*/
int
mygetchar(void) {
	int c;
	while ((c=getchar())=='\r') {
	}
	return c;
}
/*mallocs space for the dimension of grid*/
void
get_dimensions(int dimension[]) {
    int i, j;
    char c;
    char* number;
    size_t current_size;
    
    for (i = 0; i<2; i++) {
        j = 0;
        /*malloc space independently for flexible dimensions*/
        number = (char*)malloc(INITIAL*sizeof(*number));
        exit_if_null(number, "dimension allocation");
        
        current_size = INITIAL;
        while (isdigit(c = mygetchar())) {
            if (j+1 == current_size) {
                current_size *= 2;
                number = realloc(number, current_size*sizeof(*number));
                exit_if_null(number, "dimension reallocation");
            }
            number[j] = c;
            j += 1;
        }
        number[j] = '\0';
        dimension[i] = atoi(number);
        free(number);
    }
}

/*takes dimensions of a grid and returns the next cell from stdin*/
cell_t
get_cell(int r_limit, int c_limit) {
    cell_t cell;
    cell.row = get_number(r_limit);
    cell.col = get_number(c_limit);
    return cell;
}

void
exit_if_null(void *ptr, char *msg) {
    if (!ptr) {
        printf("unexpected null pointer: %s\n", msg);
        exit(EXIT_FAILURE);
    }
}

/*gets a string of digits from stdin and converts it to an integer*/
int
get_number(int limit) {
    char number[limit+1], c;
    int j = 0;
    while (!isdigit(c = mygetchar())) {
        /*skip these characters*/
    }
    while (isdigit(c)) {
        /*fill up the coord until you reach a non-numeric character*/
        number[j] = c;
        j += 1;
        c = mygetchar();
    }
    number[j] = '\0';
    /*discard the '\n' character after the cell*/
    return atoi(number);;
}
/*initialises grid with empty cells (0 value)*/
int**
construct_grid(int row_len, int col_len) {
    int i, j, **grid;

    grid = (int**)malloc(row_len*sizeof(*grid));
    exit_if_null(grid, "grid allocation");
    for (i=0;i<row_len;i++){
        grid[i] = (int*)malloc(col_len*sizeof(int));
        for (j=0;j<col_len;j++) {
            /*let 0 denote empty cell*/
            grid[i][j] = 0;    
        }
    }
    return grid;
}
/*adds blocks to the grid and returns the number of blocks received*/
int
block_grid(int **grid, int r_lim, int c_lim) {
    cell_t curr_cell;
    char c;
    int i = 0;
    /*in case we have a no-block scenario*/
    if ((c = mygetchar()) == '\n') {
        c = mygetchar();    
    }
    while (c != '$' && c != EOF) {
        /*get the next cell denoting a block*/
        curr_cell = get_cell(r_lim, c_lim);
        i += 1;
        assert(curr_cell.row <= r_lim && curr_cell.col <= c_lim);
        /*let 1 denote blocked cells*/
        grid[curr_cell.row][curr_cell.col] = 1;
        /*calling 2 helps to go over newline characters*/
        c = mygetchar(); c = mygetchar();
        if (c == '$' || c == EOF) {
            break;    
        }
    }
    assert(c == '$' || c == EOF);
    return i;
}
/*print cell to stdout*/
void
print_cell(cell_t cell) {
    printf("[%d,%d]", cell.row, cell.col);
}
/*print input route to stdout*/
void
print_route(list_t* list) {
    cell_t curr_cell;
    node_t *tmp;
    int count = 0;
    tmp = (node_t*)malloc(sizeof(*tmp));
    assert(list != NULL);
    tmp = list->head;
    
    while (tmp) {
        curr_cell = tmp->data;
        count += 1;
        print_cell(curr_cell);
        /*formatting requirements*/
        if (tmp->next == NULL) {
            printf(".\n");
            while (tmp) {
            tmp = tmp->next;    
            }
            /*end of route*/
            break;
        }
        else {
            printf("->");    
        }
        if (count == 5) {
            printf("\n");
            count = 0;
        }
        tmp = tmp->next;
    }
    /*if tmp is not NULL, it somehow messes up other lists which use tmp to
    iterate over*/
    assert(tmp == NULL);
    free(tmp);
}
/*creates route received from stdin*/
list_t*
get_route(list_t* list, int r_lim, int c_lim) {
    char c;
    cell_t curr_cell;
    c = mygetchar();
    /*route is seperated by $ or EOF*/
    while ((c != EOF) && (c != '$')) {
        curr_cell = get_cell(r_lim, c_lim);
        list = insert_at_foot(list, curr_cell);
        /*debugging reasons*/
        list->foot->num = 0;
        assert(c != EOF);
        while ((c = mygetchar()) == '\n') {
            /*do nothing more*/   
        }
    }
    return list;
}
/*returns 1 if the move between cells is legal, 0 otherwise*/
int
is_valid_move(cell_t cell_1, cell_t cell_2) {
    int row_diff, col_diff;
    row_diff = abs(cell_1.row - cell_2.row);
    col_diff = abs(cell_1.col - cell_2.col);
    /*cover jumps bigger than two in any direction*/
    if (row_diff >= 2 || col_diff >= 2) {
        return 0;    
    }
    /*cover diagonal moves*/
    if (row_diff == 1 && col_diff == 1) {
        return 0;    
    }
    /*moving to the same cell is not valid*/
    if (!(row_diff) && !(col_diff)) {
        return 0;    
    }
    else {
        return 1;    
    }
}
/*takes the grid and undergoes various status checks in order of precedence*/
int
determine_status(cell_t i_cell, cell_t g_cell, list_t *list, int **grid) {
    cell_t route_i, route_g, curr_cell, old_cell = {-1,-1};
    int blocked = 0;
    node_t *tmp;
    tmp = (node_t*)malloc(sizeof(*tmp));
    
    tmp = list->head;
    route_i = get_head(list);
    route_g = get_foot(list);
    
    if (route_i.row != i_cell.row || route_i.col != i_cell.col) {
        while (tmp) {
            tmp = tmp->next;    
        }
        free(tmp);
        return 1;
    }
    else if (route_g.row != g_cell.row || route_g.col != g_cell.col) {
        while (tmp) {
            tmp = tmp->next;    
        }
        free(tmp);
        return 2;
    }
    else {
        while (tmp) {
            curr_cell = tmp->data;
            if (old_cell.row == -1) {
            }
            else {
                /*status 3 is invalid move*/
                if (!(is_valid_move(curr_cell, old_cell))) {
                    while (tmp) {
                        tmp = tmp->next;    
                    }
                    free(tmp);
                    return 3;
                }    
            }
            /*do the block checks, but only if a block hasn't been found*/
            if (!blocked) {
                if (grid[curr_cell.row][curr_cell.col] == 1) {
                    blocked = 1;    
                }
            }
            old_cell = curr_cell;
            tmp = tmp->next;
        }
    /*we do not need to check if the last cell in list is blocked, as
    it will be the goal cell due to the previous status check*/
    }
    assert(tmp == NULL);
    free(tmp);
    if (blocked) {
        return 4;
    } 
    /*status 5 is valid*/
    assert(list != NULL);
    return 5;
}

void
print_status(int status) {
    if (status == 1) {
        printf("Initial cell in the route is wrong!\n");
    }
    else if (status == 2) {
        printf("Goal cell in the route is wrong!\n");
    }
    else if (status == 3) {
        printf("There is an illegal move in this route!\n");
    }
    else if (status == 4) {
        printf("There is a block on this route!\n");
    }
    else if (status == 5) {
        printf("The route is valid!\n");
    }
}
/*adds a route to the grid or re-adds it by using the markers '4' and '5'*/
void
put_route_on_grid(int** grid, list_t *list,
    int r_lim, int c_lim, int repair_mode) {

    node_t *tmp;
    cell_t curr_cell;
    int r, c, i, j, broken = 0;
    tmp = (node_t*)malloc(sizeof(*tmp));
    tmp = list->head;
    /*repairmode for when we want the route on the grid to be reset after
    repairing a route*/
    if (repair_mode) {
        for (i=0;i<r_lim;i++) {
            for (j=0;j<c_lim;j++) {
                if (grid[i][j] == 4 || grid[i][j] == 5) {
                    grid[i][j] = 0;    
                }
            }
        }    
    }
    while (tmp) {
        curr_cell = tmp->data;
        r = curr_cell.row;
        c = curr_cell.col;
        if (grid[r][c] == 1) {
            /*flag that the route is now broken*/
            broken = 1;    
        }
        if (!(grid[r][c])) {
            /*let 4 denote route path (asterisk)*/
            if (!broken) {
                grid[r][c] = 4;
            } else {
                /*let 5 denote route path after first block in path*/
                grid[r][c] = 5;    
            }
        }
        tmp = tmp->next;
    }
    assert(tmp == NULL);
    free(tmp);
}
/*print the contents of a grid*/
void
print_grid(int **grid, int r_lim, int c_lim) {
    int i, j, num;
    
    printf(" ");
    for (j=0;j<c_lim;j++) {
        printf("%d", j%10);    
    }
    printf("\n");
    for (i=0;i<r_lim;i++) {
        printf("%d", i%10);
        for (j=0;j<c_lim;j++) {
            num = grid[i][j];
            if (!num) {
                printf("%c", 32);    
            }
            else if (num == 1) {
                printf("%c", 35);    
            }
            else if (num == 2) {
                printf("%c", 73);
            }
            else if (num == 3) {
                printf("%c", 71);
            }
            else if (num == 4 || num == 5) {
                printf("%c", 42);    
            }
        }
        printf("\n");
    }
}
/*STAGE 1*/
/*takes the grid and a route and returns the cell before the first block seen*/
cell_t
find_block(int** grid, list_t* R) {
    node_t *tmp;
    cell_t curr_cell, start;
    int r, c;
    tmp = (node_t*)malloc(sizeof(*tmp));
    tmp = R->head;
    
    while (tmp) {
        curr_cell = tmp->data;
        r = curr_cell.row;
        c = curr_cell.col;
        /*there must be at least one block on the route if in status 4*/
        if (grid[r][c] == 1) {
            start = tmp->prev->data;
            while (tmp) {
                tmp = tmp->next;    
            }
            free(tmp);
            return start;
        }
        tmp = tmp->next;
    }
    /*returning something here avoids warning messages when compiling*/
    return get_head(R);
}
/*takes the grid, a starting cell and constructs a queue from it*/
list_t
*make_queue(list_t *q, cell_t start, int** grid, int r_lim,
    int c_lim, int *repairable) {
    node_t *tmp;
    cell_t curr_cell;
    int r, c, stop, count, **q_grid, i;
    tmp = (node_t*)malloc(sizeof(*tmp));
    q = insert_at_foot(q, start);
    /*counter associated with each node, starts at 0*/
    q->head->num = 0;
    /*declare memory*/
    q_grid = (int**)malloc(r_lim*sizeof(*q_grid));
    for (i=0;i<r_lim;i++) {
        q_grid[i] = (int*)malloc(c_lim*sizeof(int));    
    }
    /*this q_grid is used to avoid adding duplicate cells to the queue through
    a flag*/
    q_grid = construct_grid(r_lim, c_lim);
    q_grid[start.row][start.col] = 1;
    tmp = q->head;
    
    while (tmp) {
        curr_cell = tmp->data;
        count = tmp->num;
        r = curr_cell.row;
        c = curr_cell.col;
        
        /*add elements to queue*/
        
        stop = add_adj_cells(q, r, c, r_lim, c_lim, count, curr_cell, 
            grid, q_grid);
       
        /*break if the route can be repaired*/
        if (stop) {
            break;    
        }
        /*traverse queue*/
        tmp = tmp->next;
    }
    /*tmp is NULL if the queue was exhausted with no solution*/
    if (tmp == NULL) {
        *repairable = 0;
        free(tmp); 
    }
    else {
        while (tmp) {
            tmp = tmp->next;    
        }
        free(tmp);
    }
    for (i=0;i<r_lim;i++) {
        free(q_grid[i]);    
    }
    free(q_grid);
    return q;
}

int
add_adj_cells(list_t *q, int row, int col, int r_lim, int c_lim,
    int counter, cell_t cell, int** grid, int** q_grid) {

    int valid_up, valid_down, valid_left, valid_right, found;
    /*make sure the adjacent cell is within the dimensions of the grid*/
    valid_up = (row-1>=0)&&(grid[row-1][col] != 1);
    /*add cells if they are valid according to rules*/
    found = add_cell(q, -1, "row", cell, counter, valid_up, grid, q_grid);
    /*the last cell added will fix the route*/
    if (found) {return 1;}
    
    valid_down = (row+1<r_lim)&&(grid[row+1][col] != 1);
    found = add_cell(q, 1, "row", cell, counter, valid_down, grid, q_grid);
    if (found) {return 1;}
    
    valid_left = (col-1>=0)&&(grid[row][col-1] != 1);
    found = add_cell(q, -1, "col", cell, counter, valid_left, grid, q_grid);
    if (found) {return 1;}
    
    valid_right = (col+1<c_lim)&&(grid[row][col+1] != 1);
    found = add_cell(q, 1, "col", cell, counter, valid_right, grid, q_grid);
    if (found) {return 1;}
    
    return 0;
}
/*add a cell to associated queue and return 1 if the cell added is part of 
the other half of the broken route*/
int
add_cell(list_t *q, int x, char* name, cell_t cell, int counter, 
    int valid, int** grid, int** q_grid) {
    cell_t tmp_cell;
    if (valid) {
        tmp_cell = cell;
        if (!(strcmp(name, "row"))) {
            tmp_cell.row += x;
        }
        else {
            tmp_cell.col += x;    
        }
        if (!q_grid[tmp_cell.row][tmp_cell.col]) {
            /*spaces with value 1 are in the queue, this avoids duplicates*/
            q_grid[tmp_cell.row][tmp_cell.col] = 1;
            q = insert_at_foot(q, tmp_cell);
            q->foot->num = counter + 1;
            /*5 is part of the broken route segment and 3 is the goal cell*/
            if (grid[tmp_cell.row][tmp_cell.col] == 5 || 
                grid[tmp_cell.row][tmp_cell.col] == 3) {
                
                /*this will tell buddy functions to finish making the queue*/
                return 1;    
            }
        }
    }
    return 0;
}
/*takes the queue in specified assignment and picks out relevant cells to 
construct a 'bridge' for repair*/
list_t
*make_bridge(list_t *dest, list_t *q) {
    assert(q != NULL);
    
    node_t *tmp;
    cell_t curr_cell, valid_cell = {-1,-1};
    int counter, max, start_val;
    tmp = (node_t*)malloc(sizeof(*tmp));
    
    tmp = q->foot;
    curr_cell = tmp->data;
    dest = insert_at_foot(dest, curr_cell);
    /*initialise the num to 0 to avoid problems*/
    dest->foot->num = 0;
    assert(tmp != NULL);
    max = tmp->num;
    start_val = max;
    /*go through queue backwards*/
    tmp = tmp->prev;
    /*iterate over list*/
    while (tmp) {
        curr_cell = tmp->data;
        
        if (tmp->prev != NULL) {
            counter = tmp->prev->num;
        }
        /*check if this cell can be part of the bridge*/
        if (is_valid_move(curr_cell, get_head(dest))) {
            /*has to be adjacent to the next cell*/
            if (prioritise_cell(curr_cell, valid_cell, get_head(dest)) ||
                valid_cell.row == -1) {
                valid_cell = curr_cell;            
            }
        }
        /*ensure that the earliest move is added to the list*/
        if (max-counter) {
            if (valid_cell.row != -1 && max != start_val) {
                dest = insert_at_head(dest, valid_cell);
                dest->head->num = 0;
                valid_cell.row = -1;
            }
            max = counter;
        }
        tmp = tmp->prev;
    }
    free(tmp);
    /*the counter must have reached 0 by the end of iteration*/
    assert(!max);
    return dest;
}

/*takes a route and a bridge and appends the bridge to the broken segment of
the route*/
list_t
*repair_route(list_t* R, list_t* B, cell_t dead_end) {
    node_t *tmp, *discard;
    list_t *curr;
    cell_t bridge_end, curr_cell;
    tmp = (node_t*)malloc(sizeof(*tmp));
    discard = (node_t*)malloc(sizeof(*discard));
    curr = make_empty_list();
    /*important to keep in mind that any changes made to tmp are made to R*/
    tmp = R->head;
    bridge_end = get_foot(B);
    while (tmp) {
        curr_cell = tmp->data;
        DUMP_DBL(4.0);fflush(stdout);
        if (cell_cmp(curr_cell, dead_end)) {
            /*get the broken segment*/
            discard = tmp->next;
            /*discard the broken fragment of the route, including the
            bridge_end, as this cell is included in B*/
            curr->head = discard;
            while (!cell_cmp(discard->prev->data, bridge_end)) {
                discard = discard->next;
                DUMP_DBL(4.0);fflush(stdout);
            }
            curr->foot = discard->prev->prev;
            curr->foot->next = NULL;
            free_list(curr);
            /*attach head of bridge to start of broken segment*/
            tmp->next = B->head;
            B->head->prev = tmp;
            /*attach end of bridge to route to complete repair*/
            discard->prev = B->foot;
            B->foot->next = discard;
            /*discard the variables*/
            while (tmp) {
                tmp = tmp->next;    
            }
            while (discard) {
                discard = discard->next;    
            }
        }
        else {
            tmp = tmp->next;    
        }
    }
    assert(tmp == NULL && discard == NULL);
    free(tmp);
    free(discard);
    return R;
}
/*tests for equality for two cells*/
int
cell_cmp(cell_t c1, cell_t c2) {
    return (c1.row == c2.row && c1.col == c2.col);    
}

/*STAGE 3*/
int
**reset_grid(int** grid, int r_lim, int c_lim) {
    int i, j;
    for (i=0;i<r_lim;i++) {
        for (j=0;j<c_lim;j++) {
            if (grid[i][j] == 1) {
                grid[i][j] = 0;    
            }
        }
    }
    return grid;
}
/*deletes the contents of a list from memory and turns it back into an empty
list for*/
list_t
*reset_list(list_t *list) {
    node_t *curr, *p;
	assert(list!=NULL);
	curr = list->head->next;
	while (curr != list->foot) {
		p = curr;
		curr = curr->next;
		free(p);
	}
	list->head = list->foot = NULL;
	return list;
}
/*takes two cells and a central cell, and returns 1 if the 'c' cell should
be prioritised*/
int
prioritise_cell(cell_t c, cell_t v, cell_t node) {
    int prefer;
    /*prefer cell above*/
    prefer = ((c.row == node.row-1) && (c.col == node.col));
    if (prefer) {return 1;}
    /*prefer cell below if the valid cell is not above*/
    prefer = ((c.row == node.row+1) && (c.col == node.col)) &&
        !((v.row == node.row-1) && (v.col == node.col));
    if (prefer) {return 1;}
    /*prefer cell to left if the valid cell is to the right*/
    prefer = ((c.row == node.row) && (c.col == node.col-1)) &&
        ((v.row == node.row) && (v.col == node.col+1));
    if (prefer) {return 1;}
    return 0;
}

/*LISTOPS*/

/*initialise a linked list*/
list_t
*make_empty_list(void) {
	list_t *list;
	list = (list_t*)malloc(sizeof(*list));
	assert(list!=NULL);
	list->head = list->foot = NULL;
	return list;
}
int
is_empty_list(list_t *list) {
	assert(list!=NULL);
	return list->head==NULL;
}
void
free_list(list_t *list) {
	node_t *curr, *p;
	assert(list!=NULL);
	curr = list->head;
	while (curr) {
		p = curr;
		curr = curr->next;
		free(p);
	}
	free(list);
}
list_t
*insert_at_foot(list_t *list, data_t value) {
	node_t *new;
	new = (node_t*)malloc(sizeof(*new));
	assert(list!=NULL && new!=NULL);
	new->data = value;
	new->next = NULL;
	if (list->foot==NULL) {
		/* this is the first insertion into the list */
		new->prev = NULL;
		list->head = list->foot = new;
	} else {
	    new->prev = list->foot;
		list->foot->next = new;
		list->foot = new;
	}
	return list;
}
list_t
*insert_at_head(list_t *list, data_t value) {
	node_t *new;
	new = (node_t*)malloc(sizeof(*new));
	assert(list!=NULL && new!=NULL);
	new->data = value;
	new->next = list->head;
	new->prev = NULL;
	
	if (list->head != NULL) {
	    list->head->prev = new;
	}
	list->head = new;
	if (list->foot==NULL) {
		/* this is the first insertion into the list */
		list->foot = new;
	}
	
	return list;
}
data_t
get_head(list_t *list) {
	assert(list!=NULL && list->head!=NULL);
	return list->head->data;
}
data_t
get_foot(list_t *list) {
    assert(list != NULL && list->head != NULL);
    return list->foot->data;
}
list_t
*get_tail(list_t *list) {
	node_t *oldhead;
	assert(list!=NULL && list->head!=NULL);
	oldhead = list->head;
	list->head = list->head->next;
	
	if (list->head==NULL) {
		/* the only list node just got deleted */
		list->foot = NULL;
	}
	else {
	    list->head->prev = NULL;
	}
	free(oldhead);
	return list;
}