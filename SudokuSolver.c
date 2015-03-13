#include <stdio.h>								//Standard I/O functions header file
#include <string.h>								//Manipulate strings and arrays of characters functions header file
#include <time.h>								//Date and time functions header file
#include <stdlib.h>								//General functions header file, like malloc
#include <windows.h>								//Windows API functions header file (only works on Windows machines), like Sleep
#include <unistd.h>								//Unix functions header file, like sleep

/*---C program to solve Sudoku puzzles
     by Jonathan Preston Cranford
     for Insight Data Engineering Coding Challenge
     November 3, 2014
     Note:  using rules of Sudoku at http://www.sudoku.name/rules/en
---*/


/*---Highlights of the solution method - also see 6) in the Summary below
	1)  Uses recursion to solve the puzzle
	2)  Allocates only bare minimum of memory need to run the program (see 2D array deadarr), and dynamically grows memory needed during execution of the program (for 2D array deadarr) using realloc command (memory reallocation)
---*/


/*---Summary of key program operations and functions used
	1)  In function main(), program gets data from input file and stores in 2D array using functions GetFilePointer and CreateGridArr
	2)  In function main(), program solves puzzle by calling function Solve
	3)  In function Solve, empty cells of the puzzle grid are analyzed one by one by calling function AnalyzeCell
	4)  In function AnalyzeCell, the value of the cell is changed to a value between 1 and 9 by calling function Change, and the new value is checked to see if it is in accordance with the rules of Sudoku (with respect to key values in rest of puzzle grid) via the function RuleCheckPass
	5)  In function RuleCheckPass, it is also checked if the current solution has been tried before, which eliminates potential of infinite loops developing
	6)  In function Change, if all possible values for current playable cell have been tried, then a dead end is reached, and the current solution is added to the list of dead end solutions in deadarr.  If more memory is needed for deadarr, it uses realloc to increase the memory available to deadarr by a little bit.  Finally, if a dead end is reached, Change recursively calls AnalyzeCell on the previous playable cell.
---*/
   
   
   
   
   

/*---Declaration of functions
	GetFilePointer:	Get name of CSV file with unsolved puzzle, open the file for reading, and return pointer to the file
	CreateGridArr:  Initializes 2D array of integers named "grid" representing the initial puzzle state collected from CSV file
	PrintPuzzle:	Prints the current state of puzzle to standard output
	Solve:		Solves the puzzle by visiting each individual playable cell and running the function AnalyzeCell on it
	AnalyzeCell:	Analyzes the fitness and adjusts value of the current playable cell in the puzzle by running a loop of these two functions:  1) RuleCheckPass, and 2)  Change.  Returns pointer to deadarr.
	RulecheckPass:	Checks the current state of the grid to see if all rules of Sudoku have been satisfied (1), otherwise returns 0
	Change:		Changes value of current playable cell in grid, and if cycled through all possible numbers 1-9, it initiates recursion by calling function AnalyzeCell again on the previous playable cell.  Returns pointer to deadarr.
	RelMem2DArr:	Releases memory reserved for a 2D array
	RelMem1DArr:	Releases memory reserved for a 1D array
	WritePuzzle:	Writes to file SolvedSudokuPuzzle.csv the solved state of the Sudoku puzzle
---*/
FILE *GetFilePointer( void );
int **CreateGridArr( FILE *fp, int *numrows, int *numcols, int griddim, int *numplayable );
void PrintPuzzle( int **grid, int numrows, int numcols );
void Solve( int **grid, int numrows, int numcols, int numcells, int numplayable);
int **AnalyzeCell( int **grid, int *rowplay, int *colplay, int indplay, int numcols, int numrows, int **deadarr, int *badrowsnum, int numcells, int *badrowsmem );
int RuleCheckPass( int **grid, int *rowplay, int *colplay, int indplay, int numcols, int numrows, int **deadarr, int *badrowsnum );
int **Change( int **grid, int *rowplay, int *colplay, int indplay, int numcols, int numrows, int start, int **deadarr, int *badrowsnum, int numcells, int *badrowsmem );
void RelMem2DArr( int **myarr, int numrows );
void RelMem1DArr( int *myarr );
void WritePuzzle( int **grid, int numrows, int numcols);



/*---main():	Start of main function---*/
int main()
{
	/*---Define variables in function main---*/
	FILE *fp;							//Pointer to file with unsolved Sudoku puzzle
	int griddim = 9;						//Number of cells in one dimension along grid array
	int **grid = (int **)malloc( griddim*sizeof(int *) );		//The puzzle grid 2D array of integers
	int numcells = 81;						//Number of cells in Sudoku puzzle grid
	int numrows, numcols;						//Number of rows (numrows) and columns (numcols) in puzzle grid
	int numplayable = 0;						//Number of empty cells (the 0's) in initial state of puzzle
	int ind, row, col;						//Generic use index in loops (ind), row in 2D array (row), and column in 2D array (col)
	char usrinp[100];						//Generic use string to get input from user from command line
	
	
	
	/*---Changing some system level properties to make it nicer for the user, this only works if using Windows machine, but does not crash if not using Windows---*/
	system( "mode con: cols=150 lines=1000" );
	
	
	
	/*---Checking for memory allocation errors---*/
	if ( grid == NULL )
	{
		fprintf(stderr,"\n\n\nERROR - unable to allocate required memory for puzzle grid, double check your input CSV file represents a 9x9 puzzle grid ... or try freeing up some memory by closing other programs and try again.  Now exiting\n\n\n");
		exit(1);
	}
	
	

	/*---Allocate memory for columns in grid, and initialize to 0---*/
	for ( ind=0; ind<griddim; ind++ )
	{
		grid[ind] = (int *)malloc( griddim*sizeof(int) );	//Checking for memory allocation errors
		if ( grid[ind] == NULL )
		{
			fprintf(stderr,"\n\n\nERROR - unable to allocate required memory for puzzle grid, double check your input CSV file represents a 9x9 puzzle grid ... or try freeing up some memory by closing other programs and try again.  Now exiting\n\n\n");
			exit(1);
		}
	}
	for ( row=0; row<griddim; row++ )	
	{
		for ( col=0; col<griddim; col++ )
			grid[row][col] = 0;
	}
	
	
	
	/*---Call functions to proceed with solving the puzzle---*/
	fp = GetFilePointer();						//Get name of CSV file with unsolved puzzle, open the file for reading, and establish pointer fp to the file
	
	grid = CreateGridArr( fp, &numrows, &numcols, griddim, &numplayable );	//Initializes 2D array of integers named "grid" representing the initial puzzle state collected from CSV file
	
	fclose( fp );							//Close the file fp
	
	printf("\n\n\nOriginal puzzle state is:\n\n");			//Print original state of puzzle to standard output
	PrintPuzzle( grid, numrows, numcols );	
	printf("\n\n\n");					
	
	Solve( grid, numrows, numcols, numcells, numplayable);		//Solves the puzzle by visiting each individual playable cell and running the function AnalyzeCell on it, the 2D array grid from function main() is updated automatically because it is passed into Solve as a pointer
	
	//Print solved state of puzzle to standard output	
	printf("\n\n\nSolved puzzle state is below, which is also found in output file \"SolvedSudokuPuzzle.csv\":\n\n");
	PrintPuzzle( grid, numrows, numcols );
	
	WritePuzzle( grid, numrows, numcols );				//Writes solved state of puzzle to file "SolvedSudokuPuzzle.csv"		
	
	RelMem2DArr( grid, griddim);					//Release memory used for 2D array grid
	
	printf( "\n\n\nOkay, all done, puzzle is solved, have a nice day!  Press enter to exit program  " );
	gets( usrinp );
	
	
	return 0;
}






/*------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------My Functions---------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------------------------------------*/


/*---GetFilePointer:	Get name of CSV file with unsolved puzzle, open the file for reading, and return pointer to the file---*/
FILE *GetFilePointer( void )
{
	/*---Define variables in function GetFilePointer---*/
	FILE *fp;									//Pointer to file with unsolved Sudoku puzzle
	char filename[100];								//String for the name of the file (filename) with unsolved Sudoku puzzle
	
	
	
	do										//Loop to ask for user to enter file until file name exists or user exits program
	{
		printf( "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n|--------------------------------------------------INSTRUCTIONS AND NOTES--------------------------------------------------|\n\n" );
		printf( "A few instructions, notes, and comments about SudokuSolver before we get started:\n1)  SudokuSolver follows rules of Sudoku found at http://www.sudoku.name/rules/en\n2)  Input must be a comma separated file (CSV) of the initial Sudoku puzzle state\n3)  The CSV file format is 9 lines, with each line containing comma-separated numbers 0-9 (no spaces or blank lines in file)\n4)  A 0 in the CSV file represents a blank, playable cell in puzzle\n5)  Here is an example of the first few lines of a file with proper format (there is a blank line below, but this is not part of the file):\n\n0,3,5,2,9,0,8,6,4\n0,8,2,4,1,0,7,0,3\n...and so on up to line 9\n\n6)  The max file name size is 100 characters\n\n\n\n\n\n");
		printf( "|--------------------------------------------------ENTER FILE WITH UNSOLVED SUDOKU PUZZLE--------------------------------------------------|\n\n" );
		printf( "Okay, now please enter name of CSV file with unsolved Sudoku Puzzle and press enter\n\n" );
		printf("File name (e.g., puzzles/input.csv), or type 0 and press enter to exit SudokuSolver:  ");
		gets( filename );
		
		if ( strlen( filename ) == 1 && filename[0] == '0' )			//Exit program if user enters a 0
		{
			printf("\n\n\nGoodbye, enjoy!\n");
			exit(0);
		}
		fp = fopen( filename,"r" );
		if ( fp == NULL )							//User must enter another file name if file does not exist
		{
			printf( "\n\n\t\tOOPS!  That file does not exist, check you spelling and try again\n" );
			Sleep( 5000 );
//			sleep( 5 );
			printf( "\n\n\n\n\n\n" );
		}
	}
	while ( fp == NULL );
	
	return fp;
}



/*---CreateGridArr:	Initializes 2D array of integers named "grid" representing the initial puzzle state collected from CSV file---*/
int **CreateGridArr( FILE *fp, int *numrows, int *numcols, int griddim, int *numplayable )
{
	/*---Define variables in function CreateGridArr---*/
	int **grid = (int **)malloc( griddim*sizeof(int *) );				//The puzzle grid 2D array of integers
	char filerow[30];								//String for a row from the file (filerow) with unsolved puzzle
	int ind, row, col;								//Generic use index in loops (ind), row in 2D array (row), and column in 2D array (col)
	
	
	
	/*---Checking for memory allocation errors---*/
	if ( grid == NULL )
	{
		fprintf(stderr,"\n\n\nERROR - unable to allocate required memory for puzzle grid, double check your input CSV file represents a 9x9 puzzle grid ... or try freeing up some memory by closing other programs and try again.  Now exiting\n\n\n");
		exit(1);
	}
	
	
	
	/*---Allocate memory for columns in grid, and initialize to 0---*/
	for ( ind=0; ind<griddim; ind++ )
	{
		grid[ind] = (int *)malloc(griddim*sizeof(int));
		if ( grid[ind] == NULL )						//Checking for memory allocation errors
		{
			fprintf(stderr,"\n\n\nERROR - unable to allocate required memory for puzzle grid, double check your input CSV file represents a 9x9 puzzle grid ... or try freeing up some memory by closing other programs and try again.  Now exiting\n\n\n");
			exit(1);
		}
	}
		
	for ( row=0; row<griddim; row++ )	
	{
		for ( col=0; col<griddim; col++ )
			grid[row][col] = 0;
	}
	
	
	
	row = 0;
	while ( fscanf(fp, "%s", filerow) != EOF )					//Loop to read file until end of file
	{
		if ( strlen(filerow) != 17 )						//If CSV file is not correct format, ask user to correct format for CSV file, and exit
		{
			printf( "\n\n\nERROR - something is wrong with the format of your CSV file, it should contain 9 rows of 9 numbers, with numbers in each row separated by commas, and no spaces, i.e.\n0,1,0,0,5,2,0,0,9\n0,0,0,0,1,0,0,2,8\n...and so on.  Check the format of your CSV file and retry.  Goodbye, enjoy\n\n\n" );
			exit(0);
		}
		
		col = 0;
				
		for ( ind=0; ind<strlen(filerow); ind++ )				//For loop to build a row of the 2D array grid
		{
			if ( ind==0 || (ind%2==0) )
			{
				grid[row][col] = (int)(filerow[ind]) - (int)('0');
				if ( grid[row][col] == 0 )
					(*numplayable)++;
				col++;
			}
		}
		
		row++;
	}
	
	*numrows = row;
	*numcols = col;
	
	if ( *numrows!=griddim || *numcols!=griddim )					//If puzzle does not have 9 rows and 9 columns, ask user to correct CSV file, and exit		
	{
		printf("\n\nERROR - Puzzle must be 9x9 grid of integers, check that the CSV file has these dimensions and try again.  Goodbye, enjoy\n\n\n");
		exit(0);
	}
	
	return grid;
}


/*---PrintPuzzle:	Prints the current state of puzzle to standard output---*/
void PrintPuzzle( int **grid, int numrows, int numcols )
{
	/*---Define variables in function GetFilePointer---*/
	int row, col;									//Generic use row in 2D array (row) and column in 2D array (col)
	
	
	
	for ( row=0; row<numrows; row++ )						//Loop through rows and columns printing out cells to standard output
	{
		for ( col=0; col<numcols; col++ )
			printf( "%d\t", grid[row][col] );
		printf( "\n" );
	}
}



/*---Solve:	Solves the puzzle by visiting each individual playable cell and running the function AnalyzeCell on it---*/
void Solve( int **grid, int numrows, int numcols, int numcells, int numplayable )
{
	/*---Define variables in function GetFilePointer---*/
	int badrowsmem = numcells;							//Number of rows in deadarr allocated in memory (can change dynamically during execution)
	int badrowsnum = 0;								//Number of rows in deadarr, which is always <= badrowsmem
	int **deadarr = (int **)malloc( badrowsmem*sizeof(int *) );			//2D array, where each row is a combination of numbers of playable cells in the Sudoku puzzle grid that leads to a dead end.  Memory is allocated so as too not use more memory than needed, but can be reallocated during execution if more space is needed
	int *rowplay = (int *)malloc( numcells*sizeof(int) ); 				//1D arrays containing the row numbers of playable cells in puzzle grid (cells that are initially 0)
	int *colplay = (int *)malloc( numcells*sizeof(int) );				//1D arrays containing the column numbers of playable cells in puzzle grid (cells that are initially 0)
	int indplay = 0;								//The index in rowplay and colplay arrays of the current playable cell which is under consideration
	int ind, row, col; 								//Generic use index in loops (ind), row in 2D array (row), and column in 2D array (col)
	
	
		
	/*---Checking for memory allocation errors---*/
	if ( deadarr == NULL || rowplay == NULL || colplay == NULL)
	{
		fprintf(stderr,"\n\n\nERROR - unable to allocate required memory, try freeing up some memory by closing other programs and try again.  It may also be that the puzzle is unsolvable, so try double checking your input CSV file of the original puzzle state.  Now exiting\n\n\n");
		exit(1);
	}
	
	
	
	/*---Allocate memory for columns in 2D array deadarr and initialize to 0---*/
	for ( ind=0; ind<badrowsmem; ind++ )
	{
		deadarr[ind] = (int *)malloc( numcells*sizeof(int) );
		if ( deadarr[ind] == NULL )						//Checking for memory allocation errors
		{
			fprintf(stderr,"\n\n\nERROR - unable to allocate required memory, try freeing up some memory by closing other programs and try again.  It may also be that the puzzle is unsolvable, so try double checking your input CSV file of the original puzzle state.  Now exiting\n\n\n");
			exit(1);
		}
	}
	for ( row=0; row<badrowsmem; row++ )	
	{
		for ( col=0; col<numcells; col++ )
			deadarr[row][col] = 0;
	}



printf( "\n\n\n|--------------------------------------------------SOLVING PUZZLE--------------------------------------------------|\n\n" );
	
	/*---This is the beginning of the real meat of the program, where it actually solves the puzzle, and continues through functions AnalyzeCell, RuleCheckPass, Change---*/
	for ( row=0; row<numrows; row++ )						//Loop through rows of 2D puzzle array
	{
		for ( col=0; col<numcols; col++ )					//Loop through columns of 2D puzzle array
		{
			if (grid[row][col] == 0)					//If found empty cell, start looking for correct value of it
			{
				rowplay[indplay] = row;					//Add row of cell to list of row values of playable cells
				colplay[indplay] = col;					//Add column of cell to list of column values of playable cells
								
				/*---Call function AnalyzeCell to look for correct value of cell, and sends new pointer to deadarr back to function Solve in case the memory for deadarr is reallocated during the call to AnalyzeCell---*/
				deadarr = AnalyzeCell( grid, rowplay, colplay, indplay, numcols, numrows, deadarr, &badrowsnum, numcells, &badrowsmem );
				
				indplay++;						//Increase index counter of number of playable cells (represents next playable cell)
				
				if ( indplay/numplayable == 1 )
					fprintf( stderr, "\rPercent solved:  %2.0f%% ... okay, all done, see below for the solution", ((float)indplay/(float)numplayable)*100  );
				else
					fprintf( stderr, "\rPercent solved:  %2.0f%% ... just hold on, I'll solve it soon!", ((float)indplay/(float)numplayable)*100  );
			}
		}
	}
	
	
	
	/*---Release memory for arrays after done with them---*/
	RelMem2DArr( deadarr, badrowsmem );
	RelMem1DArr( rowplay );
	RelMem1DArr( colplay );
}



/*---AnalyzeCell:	Analyzes the fitness and adjusts value of the current playable cell in the puzzle by running a loop of these two functions:  1) RuleCheckPass, and 2)  Change.  Returns pointer to deadarr.---*/
int** AnalyzeCell( int **grid, int *rowplay, int *colplay, int indplay, int numcols, int numrows, int **deadarr, int *badrowsnum, int numcells, int *badrowsmem )
{
	/*---Define variables in function AnalyzeCell---*/
	int start; 									//Starting value of current cell
	


	if ( grid[rowplay[indplay]][colplay[indplay]] == 0 )				//Starting value of cell is 1 if cell was previously 0
		start = 1;
	else										//Starting value of cell is current value if AnalyzeCell called recursively
		start = grid[rowplay[indplay]][colplay[indplay]];
	
	if ( grid[rowplay[indplay]][colplay[indplay]] == 9 )				//Change current value of cell so not testing same value twice, special case if current value is 9 (must wrap back to 1)
		grid[rowplay[indplay]][colplay[indplay]] = 1;
	else										//Change current value of cell so not testing same value twice
		grid[rowplay[indplay]][colplay[indplay]]++;
	
	/*---Loop through possible values of current cell via function Change while the rules of Sudoku are not satisfied, checked via function RuleCheckPass---*/
	while ( !( RuleCheckPass( grid, rowplay, colplay, indplay, numcols, numrows, deadarr, badrowsnum ) ) )
	{		
		/*---Function Change changes value of current playable cell in grid, and if cycled through all possible numbers 1-9, it initiates recursion by calling function AnalyzeCell again on the previous playable cell---*/
		deadarr = Change( grid, rowplay, colplay, indplay, numcols, numrows, start, deadarr, badrowsnum, numcells, badrowsmem );
	}
		
	return deadarr;									//Must return pointer to deadarr to original calling function, just in case memory is reallocated during a recursive call to AnalyzeCell
}



/*---RuleCheckPass:	Checks the current state of the grid to see if all rules of Sudoku have been satisfied (returns 1), and if not, returns 0---*/
int RuleCheckPass( int **grid, int *rowplay, int *colplay, int indplay, int numcols, int numrows, int **deadarr, int *badrowsnum )
{
	/*---Define variables in function RuleCheckPass---*/
	int ind, ind2, row, col;							//Generic use indices in loops (ind and ind2), row in 2D array (row), and column in 2D array (col)
	int multrow, multcol;								//Used in determining what region of cells a given cell falls within
	int val = grid[rowplay[indplay]][colplay[indplay]];				//Value of current cell
	int regdim = 3;									//1D dimension of a  3x3 region of the grid
	int numnonrepeat=0;								//In comparing current solution to rows of deadarr (array of solutions that lead to dead end), the number of rows that are not the same as current solution
	
	
	
	/*---Check if no repetition column-wise, if not, return 0---*/
	for ( col=0; col<numcols; col++ )
	{
		if ( (grid[rowplay[indplay]][col] == val) && (col != colplay[indplay]) )
			return 0;
	}
	
	
	
	/*---Check if no repetition row-wise, if not, return 0---*/
	for ( row=0; row<numrows; row++ )
	{
		if ( (grid[row][colplay[indplay]] == val) && (row != rowplay[indplay]) )
			return 0;
	}
	
	
	
	/*---Check if no repetition region-wise (3x3 grid is a region)---*/
	multrow = rowplay[indplay]/regdim;						//Row region of current cell (0-2)
	multcol = colplay[indplay]/regdim;						//Column region of current cell (0-2)
	
	for ( row=multrow*regdim; row<=multrow*regdim+(regdim-1); row++ )		//Loop to cycle through all cells in current region to make sure value of current cell is not repeated in the region
	{
		for ( col=multcol*regdim; col<=multcol*regdim+(regdim-1); col++ )
		{
			if ( (grid[row][col] == val) && (row != rowplay[indplay]) && (col != colplay[indplay]) )
				return 0;
		}
	}
	
	/*---Check to make sure not already tried this solution, and if did, means solution is a dead end and does not check out ---*/
	for ( ind=0; ind<*badrowsnum; ind++ )						//Loop over rows of deadarr
	{
		for ( ind2=0; ind2<=indplay; ind2++ )					//Loop over playable cells that have been changed from 0
		{
			if ( grid[rowplay[ind2]][colplay[ind2]] != deadarr[ind][ind2] )
			{
				numnonrepeat++;
				break;
			}
				
		}
	}
	
	if ( numnonrepeat == *badrowsnum )						//If current solution not found in array of dead end solutions, the value in this cell checks out (at least for now), else, solution does not work and Sudoku rules are effectively not satisfied
		return 1;
	else
		return 0;
}



/*---Change:	Changes value of current playable cell in grid, and if cycled through all possible numbers 1-9, it initiates recursion by calling function AnalyzeCell again on the previous playable cell.  Returns pointer to deadarr.---*/
int ** Change( int **grid, int *rowplay, int *colplay, int indplay, int numcols, int numrows, int start, int **deadarr, int *badrowsnum, int numcells, int *badrowsmem ) 
{
	/*---Define variables in function Change---*/
	int val = grid[rowplay[indplay]][colplay[indplay]];				//Value of current cell
	int ind, row, col;								//Generic use index in loops (ind), row in 2D array (row), and column in 2D array (col)
	int **tmp;									//Temporary array used in reallocation of memory for deadarr, in order to make memory allocation bigger



	/*---Changes the value of the current cell---*/
	if ( val == 9 )
		val = 1;
	else
		val++;
	
	
	
	/*---If cycled through all values for cell, then add this solution to the list of dead end solutions in deadarr (reallocate memory in deadarr if needed), and initiate recursive call of AnalyzeCell on the previous playable cell.  In other words, there is a problem further back, we must reach it to correct it---*/
	if ( val == start )
	{
		(*badrowsnum)++;

		/*---Reallocate memory for deadarr if needed---*/
		if ( *badrowsnum>*badrowsmem )
		{
			*badrowsmem = *badrowsmem + numcells;
			tmp = (int **)realloc(deadarr, *badrowsmem*sizeof(int *));
			if ( tmp == NULL )						//Checking for memory allocation errors
			{
				fprintf(stderr,"\n\n\nERROR - unable to allocate required memory, try freeing up some memory by closing other programs and try again.  It may also be that the puzzle is unsolvable, so try double checking your input CSV file of the original puzzle state.  Now exiting\n\n\n");
				exit(1);
			}
			for ( ind=0; ind<(*badrowsmem); ind++ )
			{
				if ( ind >= (*badrowsnum-1) )
				{
					tmp[ind] = (int *)malloc(numcells*sizeof(int));
					if ( tmp == NULL )				//Checking for memory allocation errors
					{
						fprintf(stderr,"\n\n\nERROR - unable to allocate required memory, try freeing up some memory by closing other programs and try again.  It may also be that the puzzle is unsolvable, so try double checking your input CSV file of the original puzzle state.  Now exiting\n\n\n");
						exit(1);
					}
				}
			}
			deadarr = tmp;
		}

		
		
		/*---Adding dead end solution to list of dead end solutions in deadarr---*/
		for ( ind=0; ind<=indplay; ind++ )
			deadarr[(*badrowsnum)-1][ind] = grid[rowplay[ind]][colplay[ind]];

		grid[rowplay[indplay]][colplay[indplay]] = 0;				//Setting value of current cell back to zero, before recursion is called, so doesn't mess up assignment of numbers to previous playable cells



		/*---Recursive call on AnalyzeCell for playable cell (indplay-1)---*/
		deadarr = AnalyzeCell(grid,rowplay,colplay,indplay-1, numcols, numrows, deadarr, badrowsnum, numcells, badrowsmem);
	}	
	
	grid[rowplay[indplay]][colplay[indplay]] = val;
		
	return deadarr;									//Must return pointer to deadarr in case memory is reallocated

}



/*---RelMem2DArr:	Releases memory reserved for a 2D array---*/
void RelMem2DArr( int **myarr, int numrows )
{
	/*---Define variables in function RelMem2DArr---*/
	int row;									//Generic use row in 2D array (row)
	
	
	
	for ( row=0; row<numrows; row++ )
 		 free( myarr[row] );
	free(myarr);
}



/*---RelMem1DArr:	Releases memory reserved for a 1D array---*/
void RelMem1DArr( int *myarr )
{
	free(myarr);
}



/*---WritePuzzle:	Writes to file SolvedSudokuPuzzle.csv the solved state of the Sudoku puzzle---*/
void WritePuzzle( int **grid, int numrows, int numcols)
{
	/*---Define variables in function WritePuzzle---*/
	FILE *fp = fopen( "SolvedSudokuPuzzle.csv", "w" );				//Output file for solved Sudoku puzzle
	int row, col;									//Generic use row in 2D array (row), and column in 2D array (col)
	
	
	
	for ( row=0; row<numrows; row++ )
	{
		for ( col=0; col<numcols; col++ )
		{
			fputc( grid[row][col]+(int)('0'), fp );
			if ( col != numcols-1 )
				fputs( ",", fp );
		}
		if ( row != numrows-1 )
			fputs( "\n", fp );
	}
	
	fclose(fp);
}
