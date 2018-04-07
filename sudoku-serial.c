#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>


#define true 1
#define false 0

typedef struct {
   char* matrix;
   uint64_t* mask;
   int i;
} grid_struct;
typedef grid_struct* grid_t;

#define Curr_value(grid) (grid->matrix[grid->i])
#define idx(x, y) ( (x) * M + (y) )
#define valCount(x) ( __builtin_popcountll(x) )
#define valSmallest(x) ( __builtin_ctzll(x) + 1 )

typedef struct {
   grid_t* list;
   int list_size;
   int list_alloc;
}  stack_struct;
typedef stack_struct* stack_t;

typedef grid_t stackelem_t;

const int INIT_STACK_M = 128;
int M = 3, N = 9;

int soln_found = 0;
char* soln_matrix;
stack_t avail_stack;
stack_t allotment_stack;

// grid methods
void printMatrix(int **matrix);
int isValidGrid(grid_t grid);
void moveToNextUnsetCell(grid_t grid);
int getLeastUnsureCell(grid_t grid);
void Copy_grid(grid_t grid1, grid_t grid2);
void Populate_grid(grid_t grid, int **matrix);
grid_t Alloc_grid(stack_t avail);
void Free_grid(grid_t grid, stack_t avail);

// Stack Methods
stack_t Alloc_stack(void);
void Free_stack(stack_t stack);
int Empty_stack(stack_t stack);
void Push(stack_t stack, grid_t grid);
void Push_copy(stack_t stack, grid_t grid, stack_t avail);
grid_t Pop(stack_t stack);

// humanistic approach methods
void printPossibleValues(grid_t grid, int i);
int eliminate(grid_t grid);
void setCellPossibleValues(grid_t grid, int i);
void updatePeers(grid_t grid, int i);

// top level methods
int solveLogical(grid_t grid);
int Prepare_allotment_stack(grid_t grid);
int **solveSudoku(int **);


/*------------------------------------------------------------------*/

void printMatrix(int **matrix) {
   int i, j;
   for (i = 0; i < M; i++) {
      for (j = 0; j < M; j++) {
         printf("%2d ", matrix[i][j]);
      }
      printf("\n");
   }
   printf("\n");
}

void printGrid(grid_t grid) {
   int j;
   for(j = 0; j < M * M; j++) {
      if(j != grid->i){
         printf("%2d ", grid->matrix[j]);
      } else {
         printf("[%d]", grid->matrix[j]);
      }
      if(j % M == M-1) {
         printf("\n");
      }
   }
   printf("\n");
}

void printStack(stack_t stack) {
   printf("STACK: size = %d\n", stack->list_size);
   int i;
   for(i = 0; i<stack->list_size; i++)
      printGrid(stack->list[i]);
}

int isValidGrid(grid_t grid) {
   int i = grid->i;
   int x = i / M, y = i % M;
   int v = grid->matrix[i];

   int bx = (x / N) * N, by = (y / N) * N;
   int j, ox, oy;
   for (j = 0; j < M; j++) {
      ox = j / N;
      oy = j % N;
      // check row
      if (y != j && v == grid->matrix[x * M + j]) return false;

      // check column
      if (x != j && v == grid->matrix[j * M + y]) return false;

      // check box
      if (i != ((bx + ox) * M + by + oy) 
            && v == grid->matrix[(bx + ox) * M + by + oy]) 
         return false;
   }
   return true;
}

void moveToNextUnsetCell(grid_t grid){
   while(grid->i < M * M && Curr_value(grid)) {
      grid->i++;
   }
}

int getLeastUnsureCell(grid_t grid) {
   int idx = 0, min = M;
   int i;
   for(i = 0; i < M * M; i++) {
      if(grid->matrix[i] == 0 && valCount(grid->mask[i]) < min) {
         idx = i;
         min = valCount(grid->mask[idx]);
         // printf("min = %d\n", min);
      }
   }
   return idx;
}

void Copy_grid(grid_t grid1, grid_t grid2) {
   memcpy(grid2->matrix, grid1->matrix, M*M*sizeof(char));
   memcpy(grid2->mask, grid1->mask, M*M*sizeof(uint64_t));
   grid2->i = grid1->i;
}

void Populate_grid(grid_t grid, int **matrix) {
   int i;
   for(i = 0; i< M*M; i++) {
      grid->matrix[i] = matrix[i/M][i%M];
      if(grid->matrix[i] > 0) {
         grid->mask[i] = (1 << (grid->matrix[i] - 1));
      } else {
         // all values from 1 to M possible for now.
         grid->mask[i] = M == 64 ? ULLONG_MAX : ((1 << M) - 1); 
      }
   }
}


grid_t Alloc_grid(stack_t avail) {
   grid_t tmp;
   if (avail == NULL || Empty_stack(avail)) {
      tmp = (grid_t) malloc(sizeof(grid_struct));
      tmp->matrix = (char *)malloc(M*M*sizeof(char));
      tmp->mask = (uint64_t *)malloc(M*M*sizeof(uint64_t));
      return tmp;
   } else {
      return Pop(avail);
   }
}

void Free_grid(grid_t grid, stack_t avail) {
   
   if(grid != NULL) {
      if (avail == NULL) {
         free(grid->matrix);
         free(grid->mask);
         free(grid);
      } else {
         // push the grid in the avail stack only if it wouldn't cause
         // reallocation of the stack
         Push(avail, grid);
      }
   }
}

/* Stack Methods */
stack_t Alloc_stack() {
   
   stack_t tmp;
   tmp = (stack_t)malloc(sizeof(stack_struct));
   tmp->list = (grid_t *)malloc((INIT_STACK_M) * sizeof(grid_t));
   tmp->list_size = 0;
   tmp->list_alloc = INIT_STACK_M;
   return tmp;
}

void Free_stack(stack_t stack) {
   
   if(stack != NULL) {
      if(!Empty_stack(stack)) {
         int i = 0;
         for(; i<stack->list_size; i++) {
            Free_grid(stack->list[i], NULL);
         }
      }
      free(stack->list);
      free(stack);
   }
}

int Empty_stack(stack_t stack) {
   return stack == NULL || !stack->list_size;
}

void Push(stack_t stack, grid_t grid) {

   if(stack->list_size == stack->list_alloc) {
      grid_t *tmp = (grid_t *)malloc((stack->list_alloc) * 2 * sizeof(grid_t));
      memcpy(tmp, stack->list, (stack->list_size)*sizeof(grid_t));
      free(stack->list);
      stack->list = tmp;
      stack->list_alloc *= 2;
      // printf("stack->list_alloc = %d\n", stack->list_alloc);
   }

   stack->list[stack->list_size++] = grid;
}

void Push_copy(stack_t stack, grid_t grid, stack_t avail) {
   grid_t tmp = Alloc_grid(avail);
   Copy_grid(grid, tmp);
   Push(stack, tmp);
}

grid_t Pop(stack_t stack) {
   
   if(Empty_stack(stack))
      return NULL;
   stack->list_size--;
   return stack->list[stack->list_size];
}
/* Stack Methods */



int Prepare_allotment_stack(grid_t grid) {
   grid_t curr_grid = Alloc_grid(avail_stack);

   // // Temporary stacks for work assignment
   stack_t stack1 = Alloc_stack(), stack2 = Alloc_stack();
   stack_t allot_stack1, allot_stack2;

   grid->i = 0;
   Push_copy(stack1, grid, avail_stack);

   int j = 0, idx, v;
   uint64_t vals;
   do {
      if(j % 2) {
         allot_stack1 = stack2;
         allot_stack2 = stack1;
      } else {
         allot_stack1 = stack1;
         allot_stack2 = stack2;
      }

      // pop from allot_stack1, expand next level, push into allot_atack2
      while(!Empty_stack(allot_stack1)) {
         curr_grid = Pop(allot_stack1);

         if(0) printGrid(curr_grid);
         curr_grid->i = 0;
         while(curr_grid->i < M * M && Curr_value(curr_grid))
            curr_grid->i++;
         if(curr_grid->i == M * M) {
            // solution has been found
            soln_found = 1;
            soln_matrix = curr_grid->matrix;
            return 1;
         }

         idx = getLeastUnsureCell(curr_grid);
         vals = curr_grid->mask[idx];
         v = 1;
         do {
            if(vals % 2){
               curr_grid->i = idx;
               curr_grid->matrix[idx] = v;
               Copy_grid(curr_grid, grid);
               updatePeers(grid, idx);
               if(isValidGrid(grid)){
                  Push_copy(allot_stack2, grid, avail_stack);
               }
            }
            vals /= 2;
            v++;
         } while(vals > 0);

         Free_grid(curr_grid, avail_stack);

      }

      if(Empty_stack(allot_stack2)) {
         // no solution possible
         return -1;
      }
      j++;
   } while(allot_stack2->list_size < 1);

   if(grid != NULL) {
      Free_grid(grid, avail_stack);
   }
   if(curr_grid != NULL) {
      Free_grid(curr_grid, avail_stack);
   }

   allotment_stack = allot_stack2;
   return 0;
}

int solveLogical2(grid_t grid) {
   int changes = false;
   int i;
      	  printf("%d\n", !soln_found && changes);
   while(!soln_found && changes) {
	  printf("dsfsdfsdtid \n");
      for(i = 0; i < M * M; i++) {
         if(grid->matrix[i] == 0) {
            setCellPossibleValues(grid, i);

            if(grid->mask[i] == 0) return -1;
         }
         else 
            grid->mask[i] = (1 << (grid->matrix[i] - 1));
      }
      changes = eliminate(grid);
      // printGrid(grid);
   } 
   return 0;
}

int solveLogical(grid_t grid) {
   int changes = false;
   int i;
   
   do {
      // printf("Setting Possible Values\n");
      for(i = 0; i < M * M; i++) {
         if(grid->matrix[i] == 0) {
            setCellPossibleValues(grid, i);

            if(grid->mask[i] == 0) return -1;
         }
         else 
            grid->mask[i] = (1 << (grid->matrix[i] - 1));
      }
      changes = eliminate(grid);
      // printGrid(grid);

   } while(!soln_found && changes);
   return 0;
}
int **solveSudoku(int ** original_matrix) {
   avail_stack = Alloc_stack();

   grid_t init_grid = Alloc_grid(avail_stack);
   Populate_grid(init_grid, original_matrix);
   init_grid->i = 0;

   int r = solveLogical(init_grid);
   if(r < 0) {
      goto end;
   }

   r = Prepare_allotment_stack(init_grid);
   if(r != 0){
      goto end;
   }

   if(0) printStack(allotment_stack);
      int tid, i, nthrds;
      int idx, v;
      uint64_t vals;
      grid_t curr_grid, grid = Alloc_grid(avail_stack);
      stack_t avail_stack_local = Alloc_stack();
      stack_t search_stack_local = Alloc_stack();
      
      for(i = 0; i < allotment_stack->list_size && !soln_found; i++) {
         curr_grid = allotment_stack->list[i];

         do {
            
            if(solveLogical(curr_grid) < 0){
               goto next;
            }
            curr_grid->i = 0;
            while(curr_grid->i < M * M && Curr_value(curr_grid))
               curr_grid->i++;
            if(curr_grid->i == M*M) {
               soln_found = 1;
               soln_matrix = curr_grid->matrix;
               break;
            }
            
            /*****************find the next cell to expand****************/
            idx = getLeastUnsureCell(curr_grid);
            vals = curr_grid->mask[idx];
            if(0) printPossibleValues(curr_grid, idx);
            v = 1;
            do {
               if(vals % 2){
                  curr_grid->i = idx;
                  curr_grid->matrix[idx] = v;
                  Copy_grid(curr_grid, grid);
                  updatePeers(grid, idx);
                  if(isValidGrid(grid)){
                     Push_copy(search_stack_local, grid, avail_stack_local);
                  }
               }
               vals /= 2;
               v++;
            } while(vals > 0);
            /*****************find the next cell to expand****************/

            next:
               Free_grid(curr_grid, avail_stack_local);
               curr_grid = Pop(search_stack_local);
         } while(curr_grid != NULL && !soln_found);
      // Free_stack(search_stack_local);
      Free_stack(avail_stack_local);
   }
   // printMatrix(soln_matrix);
   
   end: ;
   //  ^ need to put a ';'as labels can be followed only by statements.
   // and declarations are not statements.
   int i2;
   int** ret_matrix = (int **)malloc(M * sizeof(int *));
   for(i = 0; i<M; i++) {
      ret_matrix[i] = (int *)malloc(M * sizeof(int));
   }

   // Free_stack(allotment_stack);
   // Free_stack(avail_stack);

   if(soln_found) {
      for(i = 0; i < M * M; i++) {
         ret_matrix[i/M][i%M] = soln_matrix[i];
      }
   }
   else {
      ret_matrix = original_matrix;
   }

   return ret_matrix;
}

int eliminate(grid_t grid) {
   uint64_t vals;
   int changes = false;
   int i;
   for(i = 0; i < M * M; i++) {
      vals = grid->mask[i];
      if(grid->matrix[i] == 0 && valCount(vals) == 1) {
         changes = true;
         grid->matrix[i] = valSmallest(vals);
         updatePeers(grid, i);
         if(0) printf("(%d, %d) set to %d\n", i/M, i%M, grid->matrix[i]);
      }
   }
   return changes;
}

// returns the possible values for current cell of grid.
void setCellPossibleValues(grid_t grid, int i) {
   if(grid->matrix[i] == 0) {
      // int i = grid->i;
      int x = i / M, y = i % M;
      // uint64_t possValues = grid->mask[i];
      // printPossibleValues(grid, i);
      int bx = (x / N) * N, by = (y / N) * N;
      int j, ox, oy, v;
      for(j = 0; j < M; j++) {
         ox = j / N;
         oy = j % N;
         v = grid->matrix[idx(x,j)];
         // check row
         if (y != j && v > 0) {
            v = grid->matrix[idx(x,j)];
            // printf("row : %d\n", v);
            grid->mask[i] &= ~(1 << (v - 1));
            // printPossibleValues(grid, i);
         }

         v = grid->matrix[idx(j,y)];
         // check column
         if (x != j && v > 0) {
            // printf("column : %d\n", v);
            grid->mask[i] &= ~(1 << (v - 1));
            // printPossibleValues(grid, i);
         }

         v = grid->matrix[idx(bx + ox, by + oy)];
         // check box
         if (i != idx(bx + ox, by + oy) && v > 0) {
            // printf("box : %d\n", v);
            grid->mask[i] &= ~(1 << (v - 1));
            // printPossibleValues(grid, i);
         }
      }
   } else {
      grid->mask[i] = (1 << (grid->matrix[i] - 1));
   }

   // return possValues;
}

void printPossibleValues(grid_t grid, int i) {
   printf("(%d, %d) has possible values : ", i/M, i%M);
   uint64_t vals = grid->mask[i];
   int v = 1;
   do {
      if(vals % 2)
         printf("%2d ", v);
      vals /= 2;
      v++;
   } while(vals > 0);
   printf("\n");
}

void updatePeers(grid_t grid, int i) {
   int v = grid->matrix[i];
   int x = i / M, y = i % M;
   int bx = (x / N) * N, by = (y / N) * N;
   int j, ox, oy;
   for (j = 0; j < M; j++) {
      ox = j / N;
      oy = j % N;
      // check row
      if (y != j) 
         grid->mask[idx(x, j)] &= ~(1 << (v - 1));

      if (x != j) 
         grid->mask[idx(j, y)] &= ~(1 << (v - 1));

      // check box
      if (i != idx(bx + ox, by + oy)) 
         grid->mask[idx(bx + ox, by + oy)] &= ~(1 << (v - 1));
   }
}


int **readInput2(char *filename) {
	FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    fp = fopen(filename, "r");
    if (fp == NULL) {
		printf("Could not find file\n");
        return 0;
	}
    if((read = getline(&line, &len, fp)) == -1) {
		printf("Could not read file\n");
		return 0;
	}

	sscanf(line, "%d", &N);
	M = N*N;
	char *tok;
	int **grid =(int**) malloc(sizeof(int*)*M);
    for (int i = 0;(read = getline(&line, &len, fp)) != -1 && i < M; i++) {
		grid[i] = (int*)malloc(sizeof(int)*M);
		tok = strtok(line, " ");
		for(int j = 0; j < M && tok != NULL; j++) {
			grid[i][j] = atoi(tok);
			tok = strtok(NULL, " ");
		}	
    }

    fclose(fp);
    if(line)
        free(line);
	return grid;
}

//returns a 2D array from a file containing the Sudoku in space separated format (empty cells are 0)
int ** readInput(char *filename){
   FILE *infile;
   infile = fopen(filename, "r");
   int i, j;
   char dummyline[M+1];
   char dummy;
   int value;
   int **sudokuGrid = (int**)malloc(sizeof(int*)*M);
   for (i=0;i<M;i++){
      sudokuGrid[i] = (int*)malloc(sizeof(int)*M);
      for (j=0;j<M;j++)
         sudokuGrid[i][j] = 0;
   }

   for (i = 0; i < M; i++){
      for (j = 0; j < M; j++){
         /* Checking if number of rows is less than M */
         if (feof(infile)){
            if (i != M){
               printf("The input puzzle has less number of rows than %d. Exiting.\n", M);
               exit(-1);
            }
            }

            fscanf(infile, "%d", &value);
            if(value >= 0 && value <= M)
            sudokuGrid[i][j] = value;
         else{
            printf("The input puzzle is not a grid of numbers (0 <= n <= %d) of size %dx%d. Exiting.\n", M, M, M);
            exit(-1);
         }
      }
      fscanf(infile, "%c", &dummy); /* To remove stray \0 at the end of each line */

      /* Checking if row has more elements than M */
      if (j > M){
         printf("Row %d has more number of elements than %d. Exiting.\n", i+1, M);
         exit(-1);
      }
   }
   return sudokuGrid;
}


/*checks if solution is a valid solution to original 
i.e. all originally filled cells match, and that solution is a valid grid*/
int isValid(int **original, int **solution){
   int valuesSeen[M],i,j,k;

   //check all rows
   for (i=0;i<M;i++){
      for (k=0;k<M;k++) valuesSeen[k] = 0;
      for (j=0;j<M;j++){
         if (solution[i][j]==0) return 0;
         if ((original[i][j])&&(solution[i][j] != original[i][j])) {
            printf("value mismatch at (%d, %d) \n", i, j);
            return 0;
         }
         int v = solution[i][j];
         if (valuesSeen[v-1]==1){
            printf("repeat of value %d in row at (%d, %d) \n", v, i, j);
            return 0;
         }
         valuesSeen[v-1] = 1;
      }
   }

   //check all columns
   for (i=0;i<M;i++){
      for (k=0;k<M;k++) valuesSeen[k] = 0;
      for (j=0;j<M;j++){
         int v = solution[j][i];
         if (valuesSeen[v-1]==1){
            printf("repeat of value %d in column at (%d, %d) \n", v, j, i);
            return 0;
         }
         valuesSeen[v-1] = 1;
      }
   }

   //check all minigrids
   //check all rows
   for (i=0;i<M;i=i+N){
      for (j=0;j<M;j=j+N){
         for (k=0;k<M;k++) valuesSeen[k] = 0;
         int r,c;
         for (r=i;r<i+N;r++)
            for (c=j;c<j+N;c++){
               int v = solution[r][c];
               if (valuesSeen[v-1]==1) {
                  printf("repeat of value %d in box at (%d, %d) \n", v, i, j);
                  return 0;
               }
               valuesSeen[v-1] = 1;
            }
      }
   }
   return 1;
}

int main(int argc, char *argv[]){
   if (argc<2){
      printf("Usage: ./sudoku <inputFile>\n");
      exit(0);
   }
   int **originalGrid = readInput2(argv[1]);
   int **gridToSolve = readInput2(argv[1]);
   int i,j;
   printf("************************INPUT GRID***********************\n");
   for (i=0;i<M;i++){
      for (j=0;j<M;j++){
         printf("%d ",originalGrid[i][j]);
      }
      printf("\n");
   }
   printf("*********************************************************\n");
   int **outputGrid = solveSudoku(originalGrid);

   if (isValid(originalGrid,outputGrid)){
   for (i=0;i<M;i++){
      for (j=0;j<M;j++)
         printf("%d ",outputGrid[i][j]);
      printf("\n");
   }
   }
   else{
      printf("No solution\n");
   }
}
