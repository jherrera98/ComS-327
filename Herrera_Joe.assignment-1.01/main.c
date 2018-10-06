#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

//The characteristic of a Cell
struct Cell{
 char character;
 int hardness;
 int immutable; //0 for false, 1 for true
};

//[rows,y][columns,x]
struct Cell dungeon[21][80];

//Generates the border of the dungeon and assigns spaces to the empty spaces
void generateBorder();

//Prints the Dungeon out to the console
void printDungeon();

//Draws the corridors on the Dungeon
void drawCorridors();

//Generates the rooms randomly throughout the Dungeon
void generateRooms();

//Checks to see if the given room coordinates overlap any existing rooms
//returns 1 if the given room overlaps, 0 otherwise
int roomOverlap(int start_y, int start_x, int room_y, int room_x);

//Array that contains the upper left coordinates of each room
int roomArray[10][2];

//Number of Rooms in the Dungeon
int numberOfRooms;

int main (int argc, char *argv[])
{
  int seed;
  seed = time(NULL);

  if(argc == 2){
    seed = atoi(argv[1]);
  }

  //printf("Seed = %d\n", seed);
  srand(seed);

  //Picks a random number of rooms from 5 to 10
  numberOfRooms = (rand() % 6) + 5;
  //printf("Number Of Rooms: %d\n", numberOfRooms);

  generateBorder();
  generateRooms();
  drawCorridors();
  printDungeon();
  return 0;
}

void generateBorder()
{
  int i, j;

  for (i =0; i < 21; i++){
    for(j=0; j < 80; j++){

      //Horizontal border
      if(i==0 || i == 20){
	dungeon[i][j].character = '-';
	dungeon[i][j].immutable = 1;
      }

      //Vertical Border
      else if((j ==0 || j == 79) && i < 20){
	dungeon[i][j].character = '|';
	dungeon[i][j].immutable = 1;	
      }
      else{
	dungeon[i][j].character = ' ';
	dungeon[i][j].immutable = 0;
      }
    }
  }
}

void printDungeon()
{
  int i, j; 

  //Prints the dungeon
  for (i =0; i < 21; i++){
    for(j=0; j < 80; j++){
      printf("%c", dungeon[i][j].character);
    }
    printf("\n");
  }
  
  //Print three extra lines
  for (i = 0; i < 3; i++){
    printf("\n");
  }
}

void generateRooms()
{
  int room_y;
  int room_x;
  int start_x;
  int start_y;

  
  //3 units in x and 2 units in y
  //For each room, generate
  int i;
  for(i = 0; i < numberOfRooms; i++){

    int validCoordinates = 0;

    while(!validCoordinates)
      {
	room_y = (rand()%8) + 2;//range = 2 - 9 inclusive
	room_x = (rand()%10) + 3;// range = 3 - 12 inclusive
	start_y = (rand()% 19) + 1;// range = 1-19 inclusive
	start_x = (rand()% 78) + 1;// range = 1 - 78 inclusive

	//checks to see if the room is out of the boundaries and
	// if the new room will overlap any existing rooms
	if((!(start_y + room_y > 20 || start_x + room_x > 79))
	   && roomOverlap(start_y, start_x, room_y, room_x) == 0){
	  validCoordinates = 1;
	}
      }
    
    

    int k;
    int l;
    for(k = start_y; k < (start_y + room_y); k++){
      for(l = start_x; l < (start_x + room_x); l++){
	dungeon[k][l].character = '.';
	dungeon[k][l].immutable = 0;
      }
    }
    //Saving the coordinates
    roomArray[i][0] = start_y;
    roomArray[i][1] = start_x;
  }
}

int roomOverlap(int start_y, int start_x, int room_y, int room_x)
{
  int i, j;
  
  for(i = start_y; i < (start_y + room_y); i++){
     for(j = start_x; j < (start_x + room_x); j++){
       
       //checks the point and the surrounding area(prevents rooms from being generated too closely)
       if(dungeon[i][j].character == '.' || 
	  dungeon[i-1][j].character == '.'||
	  dungeon[i+1][j].character == '.'||
	  dungeon[i][j-1].character == '.'||
	  dungeon[i][j+1].character == '.'){
	 return 1;
       }
     }
   }
  return 0;
}

void drawCorridors()
{
  //sort roomArray in asscending order by the x coordinate 
  int temp_x;
  int temp_y;

  int i, j;

  for(i = 0; i < numberOfRooms; i++){
    for(j= i + 1; j < numberOfRooms; j++){
	if(roomArray[i][1] > roomArray[j][1]){
	  temp_y = roomArray[j][0];
	  temp_x = roomArray[j][1];

	  roomArray[j][0] = roomArray[i][0];
	  roomArray[j][1] = roomArray[i][1];

	  roomArray[i][0] = temp_y;
	  roomArray[i][1] = temp_x;
	}
    }
  }


  //Connect all rooms except the last
  for(i =0; i< numberOfRooms-1; i++){
   
    int current_y = roomArray[i][0];
    int current_x = roomArray[i][1];
    int end_y = roomArray[i+1][0];
    int end_x = roomArray[i+1][1];
    int increment = 0;

    int currentChar;
    //int currentImmutable;
    //int currentHardness;
    if(!(end_x - current_x == 0))
      {
	increment = abs(end_x - current_x) / (end_x - current_x);
      }   
    while(current_x != end_x)
      {
	currentChar = dungeon[current_y][current_x].character;
	//currentImmutable = dungeon[current_y][current_x].immutable;
	//currentHardness = dungeon[current_y][current_x].hardness;
	if(!(currentChar == '.'))
	  {
	    dungeon[current_y][current_x].character = '#';
	  }
	current_x += increment;
      }

    //floating point exception 0/0?
    if(!(end_y - current_y == 0))
      {
	increment = abs(end_y - current_y) / (end_y - current_y);
      }
    
    while(current_y != end_y)
      {
	currentChar = dungeon[current_y][current_x].character;
	//currentImmutable = dungeon[current_y][current_x].immutable;
	//currentHardness = dungeon[current_y][current_x].hardness;
	if(!(currentChar == '.'))
	  {
	    dungeon[current_y][current_x].character = '#';
	  }
	current_y += increment;
      }
  }

}
