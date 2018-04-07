#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <limits.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>

#define true 1
#define false 0
#define TRUE 1
#define FALSE 0

typedef struct {
    int* mask;
} mask_struct;
typedef mask_struct* Mask;

typedef struct {
   int* matrix;
   int** mask;
   int i;
} grid_struct1;
typedef grid_struct1* Grid;

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

typedef struct {
   Grid* list;
   int list_size;
   int list_alloc;
}  stack_struct1;
typedef stack_struct1* Stack;

const int INIT_STACK_M = 128;
int M = 9, N = 3;
extern int thread_count;



int soln_found = 0;
char* soln_matrix;

int* soln_matrix2;

stack_t avail_stack;
stack_t allotment_stack;

Stack avail_stack2;
Stack allotment_stack2;

int get(Mask m, int i) {}

int valCount2(int* vals) {
	int count = 0;
	for(int i = 0; i < M;i++) {
		if(vals[i] == 1) count++;
	}
	return count;
}

int valSmallest2(int* vals) {
	int count = 0;
	for(int i = 0; i < M;i++) {
		if(vals[i] == 1) return i+1;
	}
	return 0;
}

// grid methods
void printMatrix(int **matrix);
int isValidGrid(grid_t grid);
int getLeastUnsureCell(grid_t grid);
void Copy_grid(grid_t grid1, grid_t grid2);
void Populate_grid(grid_t grid, int **matrix);
grid_t Alloc_grid(stack_t avail);
Grid Alloc_grid2(Stack avail);
void Free_grid(grid_t grid, stack_t avail);

// Stack Methods
stack_t Alloc_stack(void);
void Free_stack(stack_t stack);
int Empty_stack(stack_t stack);
int Empty_stack2(Stack stack);
void Push(stack_t stack, grid_t grid);
void Push2(Stack stack, Grid grid);
void Push_copy(stack_t stack, grid_t grid, stack_t avail);
grid_t Pop(stack_t stack);
Grid Pop2(Stack stack);

// humanistic approach methods
void printPossibleValues(grid_t grid, int i);
void printPossibleValues2(Grid grid, int i);
int eliminate(grid_t grid);
int eliminate2(Grid grid);
int eliminate3(grid_t grid);
int eliminate4(Grid grid);
void setCellPossibleValues(grid_t grid, int i);
void setCellPossibleValues3(grid_t grid, int i);
void setCellPossibleValues2(Grid grid, int i);
void setCellPossibleValues4(Grid grid, int i);
void updatePeers(grid_t grid, int i);
void updatePeers2(Grid grid, int i);
void updatePeers3(grid_t grid, int i);
void updatePeers4(Grid grid, int i);

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

void printGrid2(Grid grid) {
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

void printStack2(Stack stack) {
   printf("STACK: size = %d\n", stack->list_size);
   int i;
   for(i = 0; i<stack->list_size; i++)
      printGrid2(stack->list[i]);
}

int isValidGrid2(Grid grid) {
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
      if (i != ((bx + ox) * M + by + oy) && v == grid->matrix[(bx + ox) * M + by + oy]) 
         return false;
   }
   return true;
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
      if (i != ((bx + ox) * M + by + oy) && v == grid->matrix[(bx + ox) * M + by + oy]) 
         return false;
   }
   return true;
}



int getLeastUnsureCell2(Grid grid) {
   int idx = 0, min = M;
   int i;
   for(i = 0; i < M * M; i++) {          
		//printf("valcount = %d matrix_i = %d\n", valCount2(grid->mask[i]), grid->matrix[i]);
		
      if(grid->matrix[i] == 0 && valCount2(grid->mask[i]) < min) {
         idx = i;
         min = valCount2(grid->mask[idx]);
         //printf("min = %d\n", min);
      }
   }
   return idx;
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

void Copy_grid2(Grid grid1, Grid grid2) {
    memcpy(grid2->matrix, grid1->matrix, M*M*sizeof(int));
    memcpy(grid2->mask, grid1->mask, M*M*sizeof(int*));
    grid2->i = grid1->i;
    for(int i = 0; i < M*M; i++) {
		memcpy(grid2->mask[i],grid1->mask[i], M*sizeof(int));
	}
}
void Copy_grid(grid_t grid1, grid_t grid2) {
   memcpy(grid2->matrix, grid1->matrix, M*M*sizeof(char));
   memcpy(grid2->mask, grid1->mask, M*M*sizeof(uint64_t));
   grid2->i = grid1->i;
}

void Populate_grid2(Grid grid, int **matrix) {
   int i;
   for(i = 0; i< M*M; i++) {
      grid->matrix[i] = matrix[i/M][i%M];
      if(grid->matrix[i] > 0) {
		 for(int j = 0; j < M; j++) {
			grid->mask[i][j] = FALSE;
		 } 
         grid->mask[i][grid->matrix[i]-1] = TRUE;
      } else {
         // all values from 1 to M possible for now.
		 for(int j = 0; j < M; j++) {
			grid->mask[i][j] = TRUE;
		 } 
      }
   }
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
		//printf("ULONG %lu\n",grid->mask[i]);
      }
   }
}

Grid Alloc_grid2(Stack avail) {
	Grid temp;
    if (avail == NULL || Empty_stack2(avail)) {
 		temp = (Grid) malloc(sizeof(grid_struct1));
        temp->matrix = (int *)malloc(M*M*sizeof(int));
        temp->mask = (int **)malloc(M*M*sizeof(int*));
    	for(int i = 0; i < M*M; i++) {
			temp->mask[i] = (int *)malloc(M*sizeof(int));
    		for(int j = 0; j < M; j++) {
				//temp->mask[i][j] = FALSE;
			}
		}
    	return temp;
	} else {
		return Pop2(avail);
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

void Free_grid2(Grid grid, Stack avail) {
   
	if(grid != NULL) {
    	if (avail == NULL) {
        	free(grid->matrix);
    		/*for(int i = 0; i < M*M; i++) {
				free(grid->mask[i]);
			}*/
        	free(grid->mask);
       		free(grid);
      	} else {
        	// push the grid in the avail stack only if it wouldn't cause
        	// reallocation of the stack
            Push2(avail, grid);
      	}
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

Stack Alloc_stack2() {
   
   Stack tmp;
   tmp = (Stack)malloc(sizeof(stack_struct));
   tmp->list = (Grid *)malloc((INIT_STACK_M) * sizeof(Grid));
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


void Free_stack2(Stack stack) {
   
   if(stack != NULL) {
      if(!Empty_stack2(stack)) {
         int i = 0;
         for(; i<stack->list_size; i++) {
            Free_grid2(stack->list[i], NULL);
         }
      }
      free(stack->list);
      free(stack);
   }
}


int Empty_stack(stack_t stack) {
   return stack == NULL || !stack->list_size;
}

int Empty_stack2(Stack stack) {
   return stack == NULL || !stack->list_size;
}

void Push2(Stack stack, Grid grid) {

   if(stack->list_size == stack->list_alloc) {
      Grid *tmp = (Grid *)malloc((stack->list_alloc) * 2 * sizeof(Grid));
      memcpy(tmp, stack->list, (stack->list_size)*sizeof(Grid));
      free(stack->list);
      stack->list = tmp;
      stack->list_alloc *= 2;
      // printf("stack->list_alloc = %d\n", stack->list_alloc);
   }

   stack->list[stack->list_size++] = grid;
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

void Push_copy2(Stack stack, Grid grid, Stack avail) {
   Grid tmp = Alloc_grid2(avail);
   Copy_grid2(grid, tmp);
   Push2(stack, tmp);
}

void Push_copy(stack_t stack, grid_t grid, stack_t avail) {
   grid_t tmp = Alloc_grid(avail);
   Copy_grid(grid, tmp);
   Push(stack, tmp);
}

Grid Pop2(Stack stack) {
   
   if(Empty_stack2(stack))
      return NULL;
   stack->list_size--;
   return stack->list[stack->list_size];
}

grid_t Pop(stack_t stack) {
   
   if(Empty_stack(stack))
      return NULL;
   stack->list_size--;
   return stack->list[stack->list_size];
}
/* Stack Methods */


int Prepare_allotment_stack2(Grid grid) { // erro propaga para aqui update
	Grid curr_grid = Alloc_grid2(avail_stack2);

   	// // Temporary stacks for work assignment
   	Stack stack1 = Alloc_stack2(), stack2 = Alloc_stack2();
   	Stack allot_stack1, allot_stack2;

   	grid->i = 0;
    printPossibleValues2(avail_stack2->list[0], 0);
   	Push_copy2(stack1, grid, avail_stack2);
	printf("aqui\n");
    printPossibleValues2(stack1->list[0], 0);
   	int j = 0, idx, v;
   	int* vals;
   	do {
   		if(j % 2) {
		    allot_stack1 = stack2;
   	        allot_stack2 = stack1;
   	   } else {
   	      	allot_stack1 = stack1;
   	      	allot_stack2 = stack2;
   	   }

   	   // pop from allot_stack1, expand next level, push into allot_atack2
   	   while(!Empty_stack2(allot_stack1)) {
   	      	curr_grid = Pop2(allot_stack1);
	
   	      	if(0) printGrid2(curr_grid);
   	      	curr_grid->i = 0;
   	      	while(curr_grid->i < M * M && Curr_value(curr_grid))
   	     	    curr_grid->i++;
        	if(curr_grid->i == M * M) {
        		// solution has been found
            	printf("solution has been found in allotment\n");
            	soln_found = 1;
            	soln_matrix2 = curr_grid->matrix;
            	return 1;
        	}

        	idx = getLeastUnsureCell2(curr_grid);
        	vals = curr_grid->mask[idx];
        	v = 1;
			for(int i = 0;i < M; i++) {
				if(vals[i] == 1) {
					curr_grid->i = idx;
               		curr_grid->matrix[idx] = v;
               		Copy_grid2(curr_grid, grid);
               		updatePeers2(grid, idx);
               		if(isValidGrid2(grid)){
                 		Push_copy2(allot_stack2, grid, avail_stack2);
               		}
				}
				v++;
			}

	        Free_grid2(curr_grid, avail_stack2);
		}

      	if(Empty_stack2(allot_stack2)) {
      	   // no solution possible
           printf("no solution possible found in allotment\n");
      	   return -1;
      	}
      	j++;
   	} while(allot_stack2->list_size < thread_count);

   	if(grid != NULL) {
   		Free_grid2(grid, avail_stack2);
   	}
   	if(curr_grid != NULL) {
    	Free_grid2(curr_grid, avail_stack2);
   	}

   	allotment_stack2 = allot_stack2;
   	return 0;
}


int Prepare_allotment_stack(grid_t grid) {
   grid_t curr_grid = Alloc_grid(avail_stack);

   // // Temporary stacks for work assignment
   stack_t stack1 = Alloc_stack(), stack2 = Alloc_stack();
   stack_t allot_stack1, allot_stack2;

   grid->i = 0;
   printPossibleValues(avail_stack->list[0], 0);
   printGrid(grid);
   Push_copy(stack1, grid, avail_stack);
   printPossibleValues(stack1->list[0], 0);
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
            printf("solution has been found in allotment\n");
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
   } while(allot_stack2->list_size < thread_count);

   if(grid != NULL) {
      Free_grid(grid, avail_stack);
   }
   if(curr_grid != NULL) {
      Free_grid(curr_grid, avail_stack);
   }

   allotment_stack = allot_stack2;
   return 0;
}

int solveLogical1(grid_t grid) {
   int changes = false;
   int i;
   int onetime = 0;
   	  printf("%d\n", !soln_found && changes || onetime);
	while(!soln_found && changes || !onetime) {
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

int solveLogical2(Grid grid) {
   int changes = false;
   int i;
   
   do {
      //printf("Setting Possible Values\n");
      for(i = 0; i < M * M; i++) {
         if(grid->matrix[i] == 0) {
            setCellPossibleValues2(grid, i);
            if(grid->mask[i] == 0) return -1;
         }
         else{ 
			for(int j = 0; j < M; j++){
            	grid->mask[i][j] = FALSE;
			}
            grid->mask[i][grid->matrix[i]-1] = TRUE;
		}
      }
	  //for(int k = 0; k < M; k++) {
	//	printf(" %d ", grid->mask[i]);
	  //}
      changes = eliminate2(grid);
      //printGrid2(grid);

   } while(!soln_found && changes);
   return 0;
}
int solveLogical4(Grid grid) { // problema nesta func
   int changes = false;
   int i;
   
   do {
      //printf("Setting Possible Values\n");
      for(i = 0; i < M * M; i++) {
         if(grid->matrix[i] == 0) {
            setCellPossibleValues4(grid, i);
            if(grid->mask[i] == 0) return -1;
         }
         else{ 
			for(int j = 0; j < M; j++){
            	grid->mask[i][j] = FALSE;
			}
            grid->mask[i][grid->matrix[i]-1] = TRUE;
		}
      }
	  //for(int k = 0; k < M; k++) {
	//	printf(" %d ", grid->mask[i]);
	  //}
      changes = eliminate4(grid);
	  printf("changes = %d\n", changes);
      //printGrid2(grid);

   } while(!soln_found && changes);
   return 0;
}
int solveLogical3(grid_t grid) {
   int changes = false;
   int i;
   
   do {
      //printf("Setting Possible Values\n");
      for(i = 0; i < M * M; i++) {
         if(grid->matrix[i] == 0) {
            setCellPossibleValues3(grid, i);

            if(grid->mask[i] == 0) return -1;
         }
         else 
            grid->mask[i] = (1 << (grid->matrix[i] - 1));
      }
      changes = eliminate3(grid);
      // printGrid(grid);
   } while(!soln_found && changes);
	  printf("asdasd fsdf !! %d\n", grid->mask[0]);
   return 0;
}
int solveLogical(grid_t grid) {
   int changes = false;
   int i;
   
   do {
      //printf("Setting Possible Values\n");
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


int **solveSudoku2(int ** original_matrix) {
   avail_stack2 = Alloc_stack2();
   Grid init_grid = Alloc_grid2(avail_stack2);
   Populate_grid2(init_grid, original_matrix);
   init_grid->i = 0;


   int r = solveLogical4(init_grid);
   if(r < 0) {
      goto end;
   }
   for(int k = 0; k < M; k++) {
		printf("ASFASFAS %d ", init_grid->mask[0][k]);	
	} 
	printf("\n");
   r = Prepare_allotment_stack2(init_grid);
   if(r != 0){
		printf("1\n ASFASFAS 2323 \n");	
      goto end;
   }
	
		printf("1\n ASFASFAS PASSSOU NICE %d \n");	
   printPossibleValues2(allotment_stack2->list[0], 0);
   //printStack2(allotment_stack2);

   #pragma omp parallel shared(soln_found)
   {
      int tid, i, nthrds;
      int idx, v;
      int* vals;
      Grid curr_grid, grid = Alloc_grid2(avail_stack2);
      Stack avail_stack_local = Alloc_stack2();
      Stack search_stack_local = Alloc_stack2();
      
      
	  tid = omp_get_thread_num();
      nthrds = omp_get_num_threads();
      for(i = tid; i < allotment_stack2->list_size && !soln_found; i+=nthrds) {
		 printf("tid %d\n", tid);
         curr_grid = allotment_stack2->list[i];

         do {
            
            if(solveLogical2(curr_grid) < 0){
               goto next;
            }
            curr_grid->i = 0;
            while(curr_grid->i < M * M && Curr_value(curr_grid))
               curr_grid->i++;
            if(curr_grid->i == M*M) {
               	#pragma omp critical (soln)
                {
                	soln_found = 1;
                    soln_matrix2 = curr_grid->matrix;
                }
            	break;
            }
            
            /*****************find the next cell to expand****************/
            idx = getLeastUnsureCell2(curr_grid);
            vals = curr_grid->mask[idx];
            printPossibleValues2(curr_grid, idx);
            v = 1;
			for(int p = 0; p < M; p++) {
				if(vals[p] == 1) {
                  	curr_grid->i = idx;
                	curr_grid->matrix[idx] = v;
                	Copy_grid2(curr_grid, grid);
                  	updatePeers2(grid, idx);
                  	if(isValidGrid2(grid)){
                     	Push_copy2(search_stack_local, grid, avail_stack_local);
                  	}
				}
				v++;
			}
            /*****************find the next cell to expand****************/

            next:
               Free_grid2(curr_grid, avail_stack_local);
               curr_grid = Pop2(search_stack_local);
         } while(curr_grid != NULL && !soln_found);
      }
	  printf("FINAL tid %d\n", tid);
      // Free_stack(search_stack_local);
      Free_stack2(avail_stack_local);
   }
   // printMatrix(soln_matrix);
   
   end: ;
   //  ^ need to put a ';'as labels can be followed only by statements.
   // and declarations are not statements.
   int i;
   int** ret_matrix = (int **)malloc(M * sizeof(int *));
   for(i = 0; i<M; i++) {
      ret_matrix[i] = (int *)malloc(M * sizeof(int));
   }

   // Free_stack(allotment_stack);
   // Free_stack(avail_stack);

   if(soln_found) {
      for(i = 0; i < M * M; i++) {
         ret_matrix[i/M][i%M] = soln_matrix2[i];
      }
   }
   else {
      ret_matrix = original_matrix;
   }

   return ret_matrix;
}

int **solveSudoku(int ** original_matrix) {
   avail_stack = Alloc_stack();

   grid_t init_grid = Alloc_grid(avail_stack);
   Populate_grid(init_grid, original_matrix);
   init_grid->i = 0;

   int r = solveLogical3(init_grid);
   if(r < 0) {
      goto end;
   }

   printf("sdasd %d ", init_grid->mask[0]);	

   r = Prepare_allotment_stack(init_grid);
   if(r != 0){
      goto end;
   }

   printPossibleValues(allotment_stack->list[0], 0);

   if(0) printStack(allotment_stack);
   // exit(0);
   #pragma omp parallel shared(soln_found)
   {
      int tid, i, nthrds;
      int idx, v;
      uint64_t vals;
      grid_t curr_grid, grid = Alloc_grid(avail_stack);
      stack_t avail_stack_local = Alloc_stack();
      stack_t search_stack_local = Alloc_stack();

      tid = omp_get_thread_num();
      nthrds = omp_get_num_threads();
      for(i = tid; i < allotment_stack->list_size && !soln_found; i+=nthrds) {
         curr_grid = allotment_stack->list[i];
         do {
            
            if(solveLogical(curr_grid) < 0){
               goto next;
            }
            curr_grid->i = 0;
            while(curr_grid->i < M * M && Curr_value(curr_grid))
               curr_grid->i++;
            if(curr_grid->i == M*M) {
               #pragma omp critical (soln)
               {
                  soln_found = 1;
                  soln_matrix = curr_grid->matrix;
               }
               break;
            }
            
            /*****************find the next cell to expand****************/
            idx = getLeastUnsureCell(curr_grid);
            vals = curr_grid->mask[idx];
            printPossibleValues(curr_grid, idx);
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
      }
      // Free_stack(search_stack_local);
      Free_stack(avail_stack_local);
   }
   // printMatrix(soln_matrix);
   
   end: ;
   //  ^ need to put a ';'as labels can be followed only by statements.
   // and declarations are not statements.
   int i;
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

int eliminate4(Grid grid) { //problema propaga para qui
   int* vals;
   int changes = false;
   int i;
   for(i = 0; i < M * M; i++) {
      vals = grid->mask[i];
	  printf("(%d, %d) would set to %d , valcount = %d\n", i/M, i%M, grid->matrix[i], valCount2(vals));
      if(grid->matrix[i] == 0 && valCount2(vals) == 1) {
         changes = true;
         grid->matrix[i] = valSmallest2(vals);
	     printf("smallest %d\n", grid->matrix[i]);
         updatePeers4(grid, i);
		 //printGrid2(grid);
         //printf("(%d, %d) set to %d\n", i/M, i%M, grid->matrix[i]);
      }
   }
   return changes;
}

int eliminate3(grid_t grid) {
   uint64_t vals;
   int changes = false;
   int i;
   for(i = 0; i < M * M; i++) {
      vals = grid->mask[i];
	  printf("(%d, %d) would set to %d , valcount = %d \n", i/M, i%M, grid->matrix[i], valCount(vals));
      if(grid->matrix[i] == 0 && valCount(vals) == 1) {
         changes = true;
         grid->matrix[i] = valSmallest(vals);
	     printf("smallest %d\n", grid->matrix[i]);
         updatePeers3(grid, i);
		 //printGrid(grid);
         if(0) printf("(%d, %d) set to %d\n", i/M, i%M, grid->matrix[i]);
      }
   }
   return changes;
}

int eliminate2(Grid grid) { //problema propaga para qui
   int* vals;
   int changes = false;
   int i;
   for(i = 0; i < M * M; i++) {
      vals = grid->mask[i];
      if(grid->matrix[i] == 0 && valCount2(vals) == 1) {
         changes = true;
         grid->matrix[i] = valSmallest2(vals);
         updatePeers2(grid, i);
         if(0) printf("(%d, %d) set to %d\n", i/M, i%M, grid->matrix[i]);
      }
   }
   return changes;
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
void setCellPossibleValues2(Grid grid, int i) {
   if(grid->matrix[i] == 0) {
      int x = i / M, y = i % M;
	  //printGrid2(grid);
      // printPossibleValues2(grid, i);
      int bx = (x / N) * N, by = (y / N) * N;
      int j, ox, oy, v;
      for(j = 0; j < M; j++) {
         ox = j / N;
         oy = j % N;
         v = grid->matrix[idx(x,j)];
		 //printf("%d\n",v);
         // check row
         if (y != j && v > 0) {
            v = grid->matrix[idx(x,j)];
            //printf("row : %d\n", v);
            grid->mask[i][v-1] = FALSE;
            //printPossibleValues2(grid, i);
         }

         v = grid->matrix[idx(j,y)];
         // check column
         if (x != j && v > 0) {
            // printf("column : %d\n", v);
            grid->mask[i][v-1] = FALSE;
             //printPossibleValues2(grid, i);
         }

         v = grid->matrix[idx(bx + ox, by + oy)];
         // check box
         if (i != idx(bx + ox, by + oy) && v > 0) {
            // printf("box : %d\n", v);
            grid->mask[i][v-1] = FALSE;
             //printPossibleValues2(grid, i);
         }
      }
   } else {
	  for(int k = 0; k < M;k++) {
      	grid->mask[i][k] = 0;
	  }
      grid->mask[i][grid->matrix[i]-1] = TRUE;
   }

   // return possValues;
}
void setCellPossibleValues4(Grid grid, int i) {
   if(grid->matrix[i] == 0) {
      int x = i / M, y = i % M;
	  //printGrid2(grid);
       printPossibleValues2(grid, i);
      int bx = (x / N) * N, by = (y / N) * N;
      int j, ox, oy, v;
      for(j = 0; j < M; j++) {
         ox = j / N;
         oy = j % N;
         v = grid->matrix[idx(x,j)];
		 //printf("%d\n",v);
         // check row
         if (y != j && v > 0) {
            v = grid->matrix[idx(x,j)];
            //printf("row : %d\n", v);
            grid->mask[i][v-1] = FALSE;
            //printPossibleValues2(grid, i);
         }

         v = grid->matrix[idx(j,y)];
         // check column
         if (x != j && v > 0) {
            // printf("column : %d\n", v);
            grid->mask[i][v-1] = FALSE;
             //printPossibleValues2(grid, i);
         }

         v = grid->matrix[idx(bx + ox, by + oy)];
         // check box
         if (i != idx(bx + ox, by + oy) && v > 0) {
            // printf("box : %d\n", v);
            grid->mask[i][v-1] = FALSE;
             //printPossibleValues2(grid, i);
         }
      }
   } else {
	  for(int k = 0; k < M;k++) {
      	grid->mask[i][k] = 0;
	  }
      grid->mask[i][grid->matrix[i]-1] = TRUE;
   }

   // return possValues;
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
			//printf("%d\n",v);
         // check row
         if (y != j && v > 0) {
            v = grid->matrix[idx(x,j)];
            // printf("row : %d\n", v);
            grid->mask[i] &= ~(1 << (v - 1));
            //printPossibleValues(grid, i);
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
void setCellPossibleValues3(grid_t grid, int i) {
   if(grid->matrix[i] == 0) {
      // int i = grid->i;
      int x = i / M, y = i % M;
      // uint64_t possValues = grid->mask[i];
       printPossibleValues(grid, i);
      int bx = (x / N) * N, by = (y / N) * N;
      int j, ox, oy, v;
      for(j = 0; j < M; j++) {
         ox = j / N;
         oy = j % N;
         v = grid->matrix[idx(x,j)];
			//printf("%d\n",v);
         // check row
         if (y != j && v > 0) {
            v = grid->matrix[idx(x,j)];
            // printf("row : %d\n", v);
            grid->mask[i] &= ~(1 << (v - 1));
            //printPossibleValues(grid, i);
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
void printPossibleValues2(Grid grid, int i) {
    printf("(%d, %d) has possible values : ", i/M, i%M);
    int* vals = grid->mask[i];
	for(int i = 0; i < M; i++) {
		if(vals[i] == 1) {
			printf("%2d ", i+1);	
	 	}
		else {
			//printf("zero %d ", i+1);	
	 	}
    }
    printf("\n");
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

void updatePeers4(Grid grid, int i) {
   int v = grid->matrix[i];
   printf("matrix[%d] %d\n", i,v);
   int x = i / M, y = i % M;
   int bx = (x / N) * N, by = (y / N) * N;
   int j, ox, oy;
   for (j = 0; j < M; j++) {
      ox = j / N;
      oy = j % N;
      // check row
      if (y != j) 
         grid->mask[idx(x, j)][v-1] = FALSE;

      if (x != j) 
         grid->mask[idx(j, y)][v-1] = FALSE;

      // check box
      if (i != idx(bx + ox, by + oy)) 
         grid->mask[idx(bx + ox, by + oy)][v-1] = FALSE;
   }
}

void updatePeers3(grid_t grid, int i) {
   int v = grid->matrix[i];
   printf("matrix[%d] %d\n",i, v);
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

void updatePeers2(Grid grid, int i) {
   int v = grid->matrix[i];
   int x = i / M, y = i % M;
   int bx = (x / N) * N, by = (y / N) * N;
   int j, ox, oy;
   for (j = 0; j < M; j++) {
      ox = j / N;
      oy = j % N;
      // check row
      if (y != j) 
         grid->mask[idx(x, j)][v-1] = FALSE;

      if (x != j) 
         grid->mask[idx(j, y)][v-1] = FALSE;

      // check box
      if (i != idx(bx + ox, by + oy)) 
         grid->mask[idx(bx + ox, by + oy)][v-1] = FALSE;
   }
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


int thread_count = 4;

int **readInput(char *filename) {
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
   if (argc<3){
      printf("Usage: ./sudoku <thread_count> <inputFile>\n");
      exit(0);
   }
   thread_count = atoi(argv[1]);
   if (thread_count<=0){
      printf("Usage: ./sudoku <thread_count> <inputFile>\n");
      printf("<thread_count> should be positive\n");
   }
   int **originalGrid = readInput(argv[2]);
   int **gridToSolve = readInput(argv[2]);
   omp_set_num_threads(thread_count);
   int i,j;
   printf("************************INPUT GRID***********************\n");
   for (i=0;i<M;i++){
      for (j=0;j<M;j++){
         printf("%d ",originalGrid[i][j]);
      }
      printf("\n");
   }
   printf("*********************************************************\n");

   int **outputGrid = solveSudoku2(originalGrid);

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
