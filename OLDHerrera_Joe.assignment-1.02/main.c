#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

//The characteristic of a Cell
struct Cell{
 char character;
 uint8_t hardness;
 int immutable; //0 for false, 1 for true
};

//The characteristic of a room
struct Room{
  uint8_t xPosUpperLeft;
  uint8_t yPosUpperLeft;
  uint8_t xSize;
  uint8_t ySize;
};

//Enum containing the types of actions for file
enum action{
  action_load,//0
  action_save,//1
  action_load_save//2
};

//[rows,y][columns,x]
struct Cell dungeon[21][80];

//Pointer for the Room array
struct Room *roomArr = NULL;

//Generates the border of the dungeon and assigns spaces to the empty spaces
void generateBorder();

//Prints the Dungeon out to the console
void printDungeon();

//Draws the corridors on the Dungeon
void drawCorridors();

//Generates the rooms randomly throughout the Dungeon
void generateRooms();

//Loads/Saves the Dungeon 
void load_Save(enum action a);

//Checks to see if the given room coordinates overlap any existing rooms
//returns 1 if the given room overlaps, 0 otherwise
int roomOverlap(int start_y, int start_x, int room_y, int room_x);

//Array that contains the upper left coordinates of each room
int roomArray[10][2];

//Number of Rooms in the Dungeon
int numberOfRooms;


int main (int argc, char *argv[])
{
  //Declaring the room array with an unknown space
  roomArr = malloc(sizeof(struct Room)* 20);
  
  int seed;
  seed = time(NULL);
  srand(seed);
  //The types of actions for a file
  enum action action;
  
  //check if there are arugments
  if (argc >= 2)
    {
      int i;

      char saveStr[7] = "--save";
      char loadStr[7] = "--load";

      int fileArguments = 0;

      for(i = 1; i !=  argc; i++){
	//save
	if(strcmp(argv[i], saveStr) == 0){
	    action = action_save;
	    fileArguments++;
	  }

	//load
	else if(strcmp(argv[i], loadStr) == 0){
	    action = action_load;
	    fileArguments++;
	  }

	//invalid arguments
	else{
	  //ends the program due to invalid arguments
	  printf("invalid arguments\n");
	  return -1;
	    }
      }
      //There should only be at most 2 fileArguments, otherwise invalid arguments
      if(fileArguments == 2)
	{
	  action = action_load_save;
	}
      //calls the load_Save with the appropriate action 
      load_Save(action);
    }
  else{
    generateBorder();
    generateRooms();
  }
  //drawCorridors();
  printDungeon();

  free(roomArr);
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
  //Picks a random number of rooms from 5 to 10
  numberOfRooms = (rand() % 6) + 5;
  
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
    
    

    int k, l;
    //Assigns the the cells in the Dungeon
    for(k = start_y; k < (start_y + room_y); k++){
      for(l = start_x; l < (start_x + room_x); l++){
	dungeon[k][l].character = '.';
	dungeon[k][l].immutable = 0;
      }
    }
    //Saving the coordinates
    roomArray[i][0] = start_y;
    roomArray[i][1] = start_x;

    //Save the values in the RoomArray
    roomArr[i].xPosUpperLeft = startx;
    roomArr[i].yPosUpperLeft = starty;
    roomArr[i].xSize = room_x;
    roomArr[i].ySize = room_y;
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
void load_Save(enum action a)
{
  //Opening up a file in read binary mode
  FILE *f;
  char *filename =strcat(getenv("HOME"), "/test_dungeon_files/101.little-endian.rlg327");
  f = fopen(filename, "rb");

  //checks to see if the file exist
  if(!f){
    printf("The file could not be opened\n");
    exit(1);
  }

  //Variables for the file (file type marker, file version, and file size)
  char fileTypeMarker[12];
  uint32_t fileVersion;
  uint32_t  fileSize;

  //Reading  information about the file (file type marker, file version, and file size)
  fread(&fileTypeMarker, sizeof(fileTypeMarker), 1, f);//reading bytes 0-11
  fread(&fileVersion, sizeof(fileVersion), 1, f);//reading bytes 12-15
  fread(&fileSize, sizeof(fileSize), 1, f);//reading bytes 16-19

  //getting the number of rooms
  numberOfRooms = (fileSize - 1702) / 4;

  //Declaring and reading information about the x and y coordinates of the player
  uint8_t xCoor;
  uint8_t yCoor;
  fread(&xCoor, sizeof(xCoor), 1, f);//reading bytes 20
  fread(&yCoor, sizeof(yCoor), 1, f);//reading bytes 21

  //Assigning the hardness values to each cell in the Dungeon
  int i, j;
  for(i = 0; i < 21; i++){
    for(j=0; j < 80; j++){
      fread(&dungeon[i][j].hardness, sizeof(dungeon[i][j].hardness), 1, f);//reading bytes 22-1701
    }
  }
  
  //Reading information about each room, storing the info in the Room array
  for(i = 0; i < numberOfRooms; i++)
    {
      fread(&roomArr[i].xPosUpperLeft, sizeof(roomArr[i].xPosUpperLeft), 1, f);
      fread(&roomArr[i].yPosUpperLeft, sizeof(roomArr[i].yPosUpperLeft), 1, f);
      fread(&roomArr[i].xSize, sizeof(roomArr[i].xSize), 1, f);
      fread(&roomArr[i].ySize, sizeof(roomArr[i].ySize), 1, f);
    }
 
  /*
  //Printing out the values of the file
  printf("File Type Marker: %s\n", fileTypeMarker);
  printf("Version type: %d\n", fileVersion);
  printf("File Size: %d\n", fileSize);
  printf("X coordinate: %u\n", xCoor);
  printf("Y coordinate: %u\n\n", yCoor);
  */
  /*  
  for(i = 0; i < 21; i++){
    printf("\n");
    for(j=0; j < 80; j++){
      printf("%u", dungeon[i][j].hardness);
    }
  }
  */

  /*
  //Print out each room
  for(i = 0; i < numberOfRooms; i++)
    {
      printf("\nRoom %d:\n", i+1);
      printf("X Coor Upper Left: %u\n", roomArr[i].xPosUpperLeft);
      printf("Y Coor Upper Left: %u\n", roomArr[i].yPosUpperLeft);
      printf("xSize (width): %u\n", roomArr[i].xSize);
      printf("ySize (height): %u\n", roomArr[i].ySize);
    }


  printf("\nNumebr of Rooms: %d\n", numberOfRooms);
  */


  //closes the file
  fclose(f);
	    
  switch(a)
    {
    case action_load:
      //printf("\nThe file will be loaded\n");
      break;
    case action_save:
      //printf("\nThe file will be saved\n");
      break;
    case action_load_save:
      //printf("\nThe file will be loaded and then saved\n");
      break;
    }

}
