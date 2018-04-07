all:
	gcc sudoku-serial.c -o sudoku -fopenmp
	gcc sudoku-omp.c -o sudoku-omp -fopenmp
clean:
	rm sudoku
	rm sudoku-omp
