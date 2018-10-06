#include <stdio.h>

//The board
int board[5][5]= {
  {1, 2, 3, 4, 5},
  {6, 7, 8, 9, 10},
  {11,12,13,14,15},
  {16,17,18,19,20},
  {21,22,23,24,25}
};

//The Number Of Rows in the board
int numberOfRows = sizeof(board)/sizeof(board[0]);

//The Number of Columns in the board
int numberOfColumns = sizeof(board[0])/sizeof(board[0][0]);

//Array that contains the visited values
int visited[25];

//Size of the visited Array
int visitedSize = sizeof(visited) / sizeof(visited[0]);

//Keeps track of the size of current path
int size;

//Number of tours found
int numberOfTours;

//Finds all the possible tours
void tour(int startI, int startJ);

//Gets the value of the corresponding value, returns -1 if the value is invalid
int getCoordinateValue (int i, int j);

//Checks to see if the number is in the arrary, returns 1 for true, -1 otherwise
int InVisitedArray(int number);

//Prints out one solution
void printSolution();

int main (int argc, char *argv[])
{	
  numberOfTours = 0; 
  size = 0;

  //finds the tours of each square
  int i, j;
  for (i = 0; i < numberOfRows; i++){
    for (j = 0; j <numberOfColumns; j++) 
      {	
	tour(i,j);
      }
  }   
  
  return 0;
}

void tour(int startI, int startJ)
{
  visited [size] = getCoordinateValue(startI, startJ);
  size++;

  //array containing all the possible coordinates
  int possibleCoordinates[8][2];
  possibleCoordinates[0][0] = startI - (2);
  possibleCoordinates[0][1] = startJ - (1);
  possibleCoordinates[1][0] = startI - (2);
  possibleCoordinates[1][1] = startJ - (-1);
  possibleCoordinates[2][0] = startI - (1);
  possibleCoordinates[2][1] = startJ - (2);
  possibleCoordinates[3][0] = startI - (1);
  possibleCoordinates[3][1] = startJ - (-2);
  possibleCoordinates[4][0] = startI - (-1);
  possibleCoordinates[4][1] = startJ - (2);
  possibleCoordinates[5][0] = startI - (-1);
  possibleCoordinates[5][1] = startJ - (-2);
  possibleCoordinates[6][0] = startI - (-2);
  possibleCoordinates[6][1] = startJ - (1);
  possibleCoordinates[7][0] = startI - (-2);
  possibleCoordinates[7][1] = startJ - (-1);

  //Checks each possible coordinate to see if it is a valid position to move to
  int i = 0;
  for (i = 0; i < 8; i++)
    {
      int valueOfPossibleCoordinate = getCoordinateValue(possibleCoordinates[i][0], possibleCoordinates[i][1]);

      //checks to see if the possible coordinates are within the board and has not been visited
      if (valueOfPossibleCoordinate!= -1 && InVisitedArray(valueOfPossibleCoordinate) !=-1 )
	{
	  tour(possibleCoordinates[i][0], possibleCoordinates[i][1]);
	}
    }

  //When the size of visited array reaches 25, the path is printed out
  if(size == 25)
    {
      numberOfTours++;
      printSolution();
    }
  size--;
  visited[size] = -1;
}

int getCoordinateValue(int i, int j)
{
  if(i < 0 || i > 4 || j < 0 || j > 4) 
    {
      return -1;
    }
  return board[i][j];
}

int InVisitedArray(int number)
{
  int i;
  for (i =0; i < visitedSize ; i++)
    {
      if (visited[i] == number)
	{
	  return -1;
	}
    }
  return 1;
}

void printSolution()
{
  //printf ("%d: ", numberOfTours);
  int i;
  for (i = 0; i < visitedSize; i++)
    {
      printf("%d ", visited[i]);
    }
  printf("\n");
}

