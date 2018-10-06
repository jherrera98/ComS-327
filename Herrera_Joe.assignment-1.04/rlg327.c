#include <stdio.h>
#include <string.h>
#include <sys/time.h>

/* Very slow seed: 686846853 */

#include "dungeon.h"
#include "path.h"
#include "heap.h"
#include "unistd.h"


typedef struct event {
  heap_node_t *hn;
  int32_t time;
  uint8_t id;
  pc_t *player;
  monster_t *monster;
} event_t;



static int32_t event_cmp(const void *key, const void *with) {
  return ((event_t *) key)->time - ((event_t *) with)->time;
}


void usage(char *name)
{
  fprintf(stderr,
          "Usage: %s [-r|--rand <seed>] [-l|--load [<file>]]\n"
          "          [-s|--save [<file>]] [-i|--image <pgm file>]\n",
          name);

  exit(-1);
}


void move(event_t *e, dungeon_t *d);

void movePlayer(dungeon_t *d);

void moveMonster(event_t *e, dungeon_t *d);

//Moves the monster randomly if player is not in sight
//Or if the monster is erratic and passes the 50% chance
void moveMonsterRandomly(pair_t *nextMove, event_t *e, dungeon_t *d);

//Checks to see if nextCoor is a rock, performs calculations if needed
//based on if the monster has tunneling powers
void isRock(pair_t *nextMove, pair_t *nextCoor, event_t *e, dungeon_t *d);


int main(int argc, char *argv[])
{
  dungeon_t d;
  time_t seed;
  struct timeval tv;
  uint32_t i;
  uint32_t do_load, do_save, do_seed, do_image, do_save_seed, do_save_image;
  uint32_t long_arg;
  char *save_file;
  char *load_file;
  char *pgm_file;
  
  uint8_t numOfMonsters = 10;
  

  /* Quiet a false positive from valgrind. */
  memset(&d, 0, sizeof (d));
  
  /* Default behavior: Seed with the time, generate a new dungeon, *
   * and don't write to disk.                                      */
  do_load = do_save = do_image = do_save_seed = do_save_image = 0;
  do_seed = 1;
  save_file = load_file = NULL;

  /* The project spec requires '--load' and '--save'.  It's common  *
   * to have short and long forms of most switches (assuming you    *
   * don't run out of letters).  For now, we've got plenty.  Long   *
   * forms use whole words and take two dashes.  Short forms use an *
    * abbreviation after a single dash.  We'll add '--rand' (to     *
   * specify a random seed), which will take an argument of it's    *
   * own, and we'll add short forms for all three commands, '-l',   *
   * '-s', and '-r', respectively.  We're also going to allow an    *
   * optional argument to load to allow us to load non-default save *
   * files.  No means to save to non-default locations, however.    *
   * And the final switch, '--image', allows me to create a dungeon *
   * from a PGM image, so that I was able to create those more      *
   * interesting test dungeons for you.                             */
 
 if (argc > 1) {
    for (i = 1, long_arg = 0; i < argc; i++, long_arg = 0) {
      if (argv[i][0] == '-') { /* All switches start with a dash */
        if (argv[i][1] == '-') {
          argv[i]++;    /* Make the argument have a single dash so we can */
          long_arg = 1; /* handle long and short args at the same place.  */
        }
        switch (argv[i][1]) {
        case 'r':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-rand")) ||
              argc < ++i + 1 /* No more arguments */ ||
              !sscanf(argv[i], "%lu", &seed) /* Argument is not an integer */) {
            usage(argv[0]);
          }
          do_seed = 0;
          break;
        case 'l':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-load"))) {
            usage(argv[0]);
          }
          do_load = 1;
          if ((argc > i + 1) && argv[i + 1][0] != '-') {
            /* There is another argument, and it's not a switch, so *
             * we'll treat it as a save file and try to load it.    */
            load_file = argv[++i];
          }
          break;
        case 's':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-save"))) {
            usage(argv[0]);
          }
          do_save = 1;
          if ((argc > i + 1) && argv[i + 1][0] != '-') {
            /* There is another argument, and it's not a switch, so *
             * we'll save to it.  If it is "seed", we'll save to    *
	     * <the current seed>.rlg327.  If it is "image", we'll  *
	     * save to <the current image>.rlg327.                  */
	    if (!strcmp(argv[++i], "seed")) {
	      do_save_seed = 1;
	      do_save_image = 0;
	    } else if (!strcmp(argv[i], "image")) {
	      do_save_image = 1;
	      do_save_seed = 0;
	    } else {
	      save_file = argv[i];
	    }
          }
          break;
        case 'i':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-image"))) {
            usage(argv[0]);
          }
          do_image = 1;
          if ((argc > i + 1) && argv[i + 1][0] != '-') {
            /* There is another argument, and it's not a switch, so *
             * we'll treat it as a save file and try to load it.    */
            pgm_file = argv[++i];
          }
          break;
	case 'n':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-nummon"))) {
            usage(argv[0]);
          }
          if ((argc > i + 1) && argv[i + 1][0] != '-') {
            /* There is another argument, and it's not a switch, so *
             * we'll treat it as a save file and try to load it.    */
            numOfMonsters = atoi(argv[++i]);
          }
          break;
        default:
          usage(argv[0]);
        }
      } else { /* No dash */
        usage(argv[0]);
      }
    }
  }

  if (do_seed) {
    /* Allows me to generate more than one dungeon *
     * per second, as opposed to time().           */
    gettimeofday(&tv, NULL);
    seed = (tv.tv_usec ^ (tv.tv_sec << 20)) & 0xffffffff;
  }

  printf("Seed is %ld.\n", seed);
  srand(seed);

  init_dungeon(&d);

  if (do_load) {
    read_dungeon(&d, load_file);
  } else if (do_image) {
    read_pgm(&d, pgm_file);
  } else {
    gen_dungeon(&d);
  }

  if (!do_load) {
    /* Set a valid position for the PC */
    d.pc.position[dim_x] = (d.rooms[0].position[dim_x] +
                            (rand() % d.rooms[0].size[dim_x]));
    d.pc.position[dim_y] = (d.rooms[0].position[dim_y] +
                            (rand() % d.rooms[0].size[dim_y]));
    d.pc.speed = 10;
    d.pc.characteristic = 0;
  }

  printf("PC is at (y, x): %d, %d\n",
         d.pc.position[dim_y], d.pc.position[dim_x]);

  d.numberOfMonsters = numOfMonsters;
  printf("%d\n", d.numberOfMonsters);

  monster_t monsters[d.numberOfMonsters];

  for(i = 0; i < d.numberOfMonsters; i++)
    {
      //Randomly assigns the characteristic and speed of a monster 
      monsters[i].characteristic = rand() % 16;
      monsters[i].speed = rand() % 16 + 5;

      //Location of monster in array
      monsters[i].arrayLoc = i;

      //Picks a random room and placed the monster in it
      uint randomRoom = rand()% (d.num_rooms-1) +1;
      monsters[i].position[dim_x] = (d.rooms[randomRoom].position[dim_x] +
                            (rand() % d.rooms[randomRoom].size[dim_x]));
      monsters[i].position[dim_y] = (d.rooms[randomRoom].position[dim_y] +
                            (rand() % d.rooms[randomRoom].size[dim_y]));

      //Unknown Destination to start off with
      monsters[i].destination[dim_x] = -1;
      monsters[i].destination[dim_y] = -1;
    }

  render_dungeon(&d, monsters);
  dijkstra(&d);
  dijkstra_tunnel(&d);
  //render_distance_map(&d);
  //render_tunnel_distance_map(&d);
  //render_hardness_map(&d);
  //render_movement_cost_map(&d);

  if (do_save) {
    if (do_save_seed) {
       /* 10 bytes for number, plus dot, extention and null terminator. */
      save_file = malloc(18);
      sprintf(save_file, "%ld.rlg327", seed);
    }
    if (do_save_image) {
      if (!pgm_file) {
	fprintf(stderr, "No image file was loaded.  Using default.\n");
	do_save_image = 0;
      } else {
	/* Extension of 3 characters longer than image extension + null. */
	save_file = malloc(strlen(pgm_file) + 4);
	strcpy(save_file, pgm_file);
	strcpy(strchr(save_file, '.') + 1, "rlg327");
      }
    }
    write_dungeon(&d, save_file);

    if (do_save_seed || do_save_image) {
      free(save_file);
    }
  }


  heap_t h;
  
  heap_init(&h, event_cmp, NULL);
  event_t eArr[d.numberOfMonsters +1];

  //Insert the player
  eArr[0].id = 20;
  eArr[0].time = 0;
  eArr[0].player = &d.pc;
  eArr[0].hn = heap_insert(&h, &eArr[0]);
  

  

  //Insert each monster
  for(i = 0; i < d.numberOfMonsters; i++)
  {
    eArr[i+1].id = monsters[i].characteristic;
    eArr[i+1].monster = &monsters[i];
    eArr[i+1].time = 0;
    eArr[i+1].hn = heap_insert(&h, &eArr[i+1]);
  }

  event_t *p;
  /*
  //while loop to keep calling each update based on time and speed
  
  while ((p = heap_remove_min(&h))){
    p->hn = NULL;
    printf("Time: %d, ID:  %d\n", p->time, p->id);
  }
  exit(-1);
*/

  int gameOver = 0;
  
  while(!gameOver)
    {
      p = heap_remove_min(&h);

      //Sees if this monster is under another monster, KILLS if true
      if(p->id != 20)
	{
	  for (i=0; i < d.numberOfMonsters; i++)
	    {
	      //Checks to see if this monster is not the same and then
	      //checks the position of both monsters
	      if(p->monster->arrayLoc != monsters[i].arrayLoc && 
		 p->monster->position[dim_x] == monsters[i].position[dim_x] &&
		 p->monster->position[dim_y] == monsters[i].position[dim_y])
		{
		  p->monster->position[dim_y] = -1;
		  p->monster->position[dim_x] = -1;
		  p = NULL;
		  break;
		}
	    }
	}

      
      
      move(p, &d);


      render_dungeon(&d, monsters);
      dijkstra(&d);
      dijkstra_tunnel(&d);
      //render_distance_map(&d);
      //render_tunnel_distance_map(&d);
      //render_hardness_map(&d);
      //render_movement_cost_map(&d);

      //Pull the values from the pointer into a non pointer variable to insert into the heap 
      if(p != NULL)
	{
	  p->hn = heap_insert(&h, p);
	  
	  //Determine whose turn it was for win/lose
	  if(p->id == 20)
	    {
	      for (i=0; i < d.numberOfMonsters; i++)
	    {
	      if(d.pc.position[dim_x] == monsters[i].position[dim_x] &&
		 d.pc.position[dim_y] == monsters[i].position[dim_y])
		{
		  gameOver = 1;
		  printf("\nWIN\n");
		}
	    }
	    }
	  else
	    {
	      if(p->monster->position[dim_x] == d.pc.position[dim_x] && 
		 p->monster->position[dim_y] == d.pc.position[dim_y])
		{
		  gameOver = 1;
		  printf("\nLOSE\n");
		}
	    }
	} 
      
      usleep(250000);
    }

  heap_delete(&h);
  delete_dungeon(&d);

  

  return 0;
}


void move(event_t *e, dungeon_t *d)
{
  //returns if the value is null
  if (e == NULL)
    {
      return;
    }
  else if(e->id != 20)
    { 
      //Update time based on speed
      e->time += (1000 / e->monster->speed);
      
      moveMonster(e, d);
    }

  else{
    e->time += (1000 / e->player->speed);
    movePlayer(d);
  }
}

void moveMonster(event_t *e, dungeon_t *d)
{
  //The coordinate of the next move that is calculated based off of characteristics
  pair_t nextMove; 

  //Updates next move with the current position of the monster
  nextMove[dim_y] = e->monster->position[dim_y];
  nextMove[dim_x] = e->monster->position[dim_x];

  //Checks the surrounding cells, radius of 5 for the pc: Line of sight
  int inSight = 0;
  int i, j;
  for(i = -5; i < 6; i++){
    for(j = -5; j < 6; j++){
      if((e->monster->position[dim_x] + (i))  == d->pc.position[dim_x] &&
	 (e->monster->position[dim_x] + (j))  == d->pc.position[dim_y])
	{
	  inSight = 1;
	}	
    }
  }
	
  //Update destination based on the telepathy power or if the player is in sight
  if((e->id & 2) == 2 || inSight)
  {
    e->monster->destination[dim_x] = d->pc.position[dim_x];
    e->monster->destination[dim_y] = d->pc.position[dim_y];
  }

  //if erratic is true along with 50% chance or if destination is unknown
  //the monster moves randomly and then returns out of this method
  if(((e->id & 8) == 8 && rand() % 2 == 1) || e->monster->destination[dim_x] == -1)
    {
      moveMonsterRandomly(&nextMove, e , d);
      e->monster->position[dim_x] = nextMove[dim_x];
      e->monster->position[dim_y] = nextMove[dim_y];
      return;
    }

  //Saves the Coor of the player to call dijkstra on the destination
  pair_t tempPlayer;
  tempPlayer[dim_x] = d->pc.position[dim_x];
  tempPlayer[dim_y] = d->pc.position[dim_y];
  
  //Calling dijkstra on the "new player"
  d->pc.position[dim_x] = e->monster->destination[dim_x];
  d->pc.position[dim_y] = e->monster->destination[dim_y];

  dijkstra(d);
  dijkstra_tunnel(d);

  //Checks to see if the monster has the tunneling power
  if ((e->id & 4) == 4)
    {
      //Checks to see if the monster is intelligent
      if((e->id & 1) == 1)
	{
	  
	  
	  //Takes the shortest path to destination	  
	  pair_t smallestCoor;
	  uint8_t cost = 255;

	  //Checks all possible 8 moves to see which one is the smallest
	  if(d->pc_tunnel[e->monster->position[dim_y]-1][e->monster->position[dim_x]] < cost){
	    cost = d->pc_tunnel[e->monster->position[dim_y]-1][e->monster->position[dim_x]];
	    smallestCoor[dim_x] = e->monster->position[dim_x];
	    smallestCoor[dim_y] = e->monster->position[dim_y]-1;
	  }
	 
	  if(d->pc_tunnel[e->monster->position[dim_y]+1][e->monster->position[dim_x]] < cost){
	    cost = d->pc_tunnel[e->monster->position[dim_y]+1][e->monster->position[dim_x]];
	    smallestCoor[dim_x] = e->monster->position[dim_x];
	    smallestCoor[dim_y] = e->monster->position[dim_y]+1;
	  }
	  if(d->pc_tunnel[e->monster->position[dim_y]][e->monster->position[dim_x]-1] < cost){
	    cost = d->pc_tunnel[e->monster->position[dim_y]][e->monster->position[dim_x]-1];
	    smallestCoor[dim_x] = e->monster->position[dim_x]-1;
	    smallestCoor[dim_y] = e->monster->position[dim_y];
	  }
	  if(d->pc_tunnel[e->monster->position[dim_y]][e->monster->position[dim_x]+1] < cost){
	    cost = d->pc_tunnel[e->monster->position[dim_y]][e->monster->position[dim_x]+1];
	    smallestCoor[dim_x] = e->monster->position[dim_x]+1;
	    smallestCoor[dim_y] = e->monster->position[dim_y];
	  }
	  if(d->pc_tunnel[e->monster->position[dim_y]-1][e->monster->position[dim_x]-1] < cost){
	    cost = d->pc_tunnel[e->monster->position[dim_y]-1][e->monster->position[dim_x]-1];
	    smallestCoor[dim_x] = e->monster->position[dim_x]-1;
	    smallestCoor[dim_y] = e->monster->position[dim_y]-1;
	  }
	  if(d->pc_tunnel[e->monster->position[dim_y]+1][e->monster->position[dim_x]+1] < cost){
	    cost = d->pc_tunnel[e->monster->position[dim_y]+1][e->monster->position[dim_x]+1];
	    smallestCoor[dim_x] = e->monster->position[dim_x]+1;
	    smallestCoor[dim_y] = e->monster->position[dim_y]+1;
	  }
	  if(d->pc_tunnel[e->monster->position[dim_y]-1][e->monster->position[dim_x]+1] < cost){
	    cost = d->pc_tunnel[e->monster->position[dim_y]-1][e->monster->position[dim_x]+1];
	    smallestCoor[dim_x] = e->monster->position[dim_x]+1;
	    smallestCoor[dim_y] = e->monster->position[dim_y]-1;
	  }
	  if(d->pc_tunnel[e->monster->position[dim_y]+1][e->monster->position[dim_x]-1] < cost){
	    cost = d->pc_tunnel[e->monster->position[dim_y]+1][e->monster->position[dim_x]-1];
	    smallestCoor[dim_x] = e->monster->position[dim_x]-1;
	    smallestCoor[dim_y] = e->monster->position[dim_y]+1;
	  }


	  isRock(&nextMove, &smallestCoor, e, d);
	  
	}

      //Takes a straight line to desination
      else 
	{
	  //Coordinate of the next for the straight line
	  pair_t straightCoor;
	  straightCoor[dim_x] = e->monster->position[dim_x];
	  straightCoor[dim_y] = e->monster->position[dim_y];

	  //Checks to see if the x position is equal to the destination
	  if(straightCoor[dim_x] != d->pc.position[dim_x])
	    {
	      //Checks to see if the x coor is neg or pos direction 
	      if(straightCoor[dim_x] < d->pc.position[dim_x])
		{
		  straightCoor[dim_x] += 1;
		}
	      else
		{
		  straightCoor[dim_x] -= 1;
		}
	    }
	  //Checks to see if the y position is equal to the destination
	  if(straightCoor[dim_y] != d->pc.position[dim_y])
	    {
	      //Checks to see if the y coor is neg or pos direction 
	      if(straightCoor[dim_y] < d->pc.position[dim_y])
		{
		  straightCoor[dim_y] += 1;
		}
	      else
		{
		  straightCoor[dim_y] -= 1;
		}
	    }

	  isRock(&nextMove, &straightCoor, e, d);
	}
    }
  
  //Monster cannnot tunnel through rock
  else
    {
      //Checks to see if the monster is intelligent
      if((e->id & 1) == 1)
	{
	  //Takes the shortest path to destination
	  
	  pair_t smallestCoor;
	  uint8_t cost = 255;

	  //Checks all possible 8 moves to see which one is the smallest
	  if(d->pc_distance[e->monster->position[dim_y]-1][e->monster->position[dim_x]] < cost){
	    cost = d->pc_distance[e->monster->position[dim_y]-1][e->monster->position[dim_x]];
	    smallestCoor[dim_x] = e->monster->position[dim_x];
	    smallestCoor[dim_y] = e->monster->position[dim_y]-1;
	  }
	 
	  if(d->pc_distance[e->monster->position[dim_y]+1][e->monster->position[dim_x]] < cost){
	    cost = d->pc_distance[e->monster->position[dim_y]+1][e->monster->position[dim_x]];
	    smallestCoor[dim_x] = e->monster->position[dim_x];
	    smallestCoor[dim_y] = e->monster->position[dim_y]+1;
	  }
	  if(d->pc_distance[e->monster->position[dim_y]][e->monster->position[dim_x]-1] < cost){
	    cost = d->pc_distance[e->monster->position[dim_y]][e->monster->position[dim_x]-1];
	    smallestCoor[dim_x] = e->monster->position[dim_x]-1;
	    smallestCoor[dim_y] = e->monster->position[dim_y];
	  }
	  if(d->pc_distance[e->monster->position[dim_y]][e->monster->position[dim_x]+1] < cost){
	    cost = d->pc_distance[e->monster->position[dim_y]][e->monster->position[dim_x]+1];
	    smallestCoor[dim_x] = e->monster->position[dim_x]+1;
	    smallestCoor[dim_y] = e->monster->position[dim_y];
	  }
	  if(d->pc_distance[e->monster->position[dim_y]-1][e->monster->position[dim_x]-1] < cost){
	    cost = d->pc_distance[e->monster->position[dim_y]-1][e->monster->position[dim_x]-1];
	    smallestCoor[dim_x] = e->monster->position[dim_x]-1;
	    smallestCoor[dim_y] = e->monster->position[dim_y]-1;
	  }
	  if(d->pc_distance[e->monster->position[dim_y]+1][e->monster->position[dim_x]+1] < cost){
	    cost = d->pc_distance[e->monster->position[dim_y]+1][e->monster->position[dim_x]+1];
	    smallestCoor[dim_x] = e->monster->position[dim_x]+1;
	    smallestCoor[dim_y] = e->monster->position[dim_y]+1;
	  }
	  if(d->pc_distance[e->monster->position[dim_y]-1][e->monster->position[dim_x]+1] < cost){
	    cost = d->pc_distance[e->monster->position[dim_y]-1][e->monster->position[dim_x]+1];
	    smallestCoor[dim_x] = e->monster->position[dim_x]+1;
	    smallestCoor[dim_y] = e->monster->position[dim_y]-1;
	  }
	  if(d->pc_distance[e->monster->position[dim_y]+1][e->monster->position[dim_x]-1] < cost){
	    cost = d->pc_distance[e->monster->position[dim_y]+1][e->monster->position[dim_x]-1];
	    smallestCoor[dim_x] = e->monster->position[dim_x]-1;
	    smallestCoor[dim_y] = e->monster->position[dim_y]+1;
	  }

	  nextMove[dim_y] = smallestCoor[dim_y];
	  nextMove[dim_x] = smallestCoor[dim_x];
	}
      //Takes a straight line to desination
      else 
	{
	  //Coordinate of the next for the straight line
	  pair_t straightCoor;
	  straightCoor[dim_x] = e->monster->position[dim_x];
	  straightCoor[dim_y] = e->monster->position[dim_y];

	  //Checks to see if the x position is equal to the destination
	  if(straightCoor[dim_x] != d->pc.position[dim_x])
	    {
	      //Checks to see if the x coor is neg or pos direction 
	      if(straightCoor[dim_x] < d->pc.position[dim_x])
		{
		  straightCoor[dim_x] += 1;
		}
	      else
		{
		  straightCoor[dim_x] -= 1;
		}
	    }
	  //Checks to see if the y position is equal to the destination
	  if(straightCoor[dim_y] != d->pc.position[dim_y])
	    {
	      //Checks to see if the y coor is neg or pos direction 
	      if(straightCoor[dim_y] < d->pc.position[dim_y])
		{
		  straightCoor[dim_y] += 1;
		}
	      else
		{
		  straightCoor[dim_y] -= 1;
		}
	    }

	  
	  //Checks to see if straightCoor is a rock
	  if(d->map[straightCoor[dim_y]][straightCoor[dim_x]] != ter_floor_room)
	    {
	      //The monster can't do anything since they can't tunnel
	    }
	  //Moves as normal if not a rock
	  else
	    {
	      nextMove[dim_y] = straightCoor[dim_y];
	      nextMove[dim_x] = straightCoor[dim_x];
	    }  
	}
    }


  
  //Update destination back to -1 if this monster is not intelligent
  if(((e->id & 1) != 1))
    {
      e->monster->destination[dim_x] = -1;
      e->monster->destination[dim_y] = -1;
    }

  //sets the player back to its original location
  d->pc.position[dim_x] = tempPlayer[dim_x];
  d->pc.position[dim_y] = tempPlayer[dim_y];
  
  //side note: might have to call dijksra again 

  e->monster->position[dim_x] = nextMove[dim_x];
  e->monster->position[dim_y] = nextMove[dim_y];
}

void moveMonsterRandomly(pair_t *nextMove, event_t *e, dungeon_t *d){
    
  pair_t p;
  p[dim_x] = e->monster->position[dim_x];
  p[dim_y] = e->monster->position[dim_y];
  
  while(1)
    {
      //Randomly picks surrounding cell
      p[dim_x] = p[dim_x] + ((rand() % 3) - 1);
      p[dim_y] = p[dim_y] + ((rand() % 3) - 1);
      
      //Checks to see if the coors not the same
      if(p[dim_x] != e->monster->position[dim_x]&& 
	 p[dim_x] != e->monster->position[dim_x])
	{
	  break;
	}
      
      //Sets back to original coors if they are the same, to try again
      p[dim_x] = e->monster->position[dim_x];
      p[dim_y] = e->monster->position[dim_y];
    }
  isRock(nextMove, &p, e, d);
}

void isRock(pair_t *nextMove, pair_t *nextCoor, event_t *e, dungeon_t *d)
{
  //Checks to see if nextCoor is a rock and if the monster has tunneling powers
  if(d->map[(*nextCoor)[1]][(*nextCoor)[0]] != ter_floor_room && (e->id & 4) == 4)
    {
      //subtracts 85 from the hardness with a min of 0, temp is to deal with wrapping around
      //when subtracting from an unsigned int
      int temp = d->hardness[(*nextCoor)[1]][(*nextCoor)[0]];
      d->hardness[(*nextCoor)[1]][(*nextCoor)[0]] -=85;
      temp -= 85;
      if (temp < 0)
	{
	  d->hardness[(*nextCoor)[1]][(*nextCoor)[0]] = 0;
	}
      
      //Checks the hardness of the nextCoor, if 0, then updates terrian
      if(d->hardness[(*nextCoor)[1]][(*nextCoor)[0]] == 0)
	{
	  d->map[(*nextCoor)[1]][(*nextCoor)[0]] = ter_floor_hall;
	  (*nextMove)[1] = (*nextCoor)[1];
	  (*nextMove)[0] = (*nextCoor)[0];
	}
    }
  //if the next position is a rock but monster doesn't have tunneling powers
  //recalls random to try again for an open cell
  else if((d->map[(*nextCoor)[1]][(*nextCoor)[0]] != ter_floor_room && (e->id & 4) != 4))
    {
      moveMonsterRandomly(nextMove, e, d);
      return;
    }
  else//Moves as normal if not a rock
    {
      (*nextMove)[0] = (*nextCoor)[0];
      (*nextMove)[1] = (*nextCoor)[1];
    } 
}

void movePlayer(dungeon_t *d)
{
  pair_t p;
  p[dim_x] = d->pc.position[dim_x];
  p[dim_y] = d->pc.position[dim_y];

  while(1)
    {
      //Randomly picks surrounding cell
      p[dim_x] = p[dim_x] + ((rand() % 3) - 1);
      p[dim_y] = p[dim_y] + ((rand() % 3) - 1);
      
      //Checks to see if the coors are valid
      if(mappair(p) != ter_wall_immutable && mappair(p) != ter_wall && p[dim_x] != d->pc.position[dim_x] &&
	 p[dim_x] != d->pc.position[dim_x])
	{
	  break;
	}

      //Sets back to original coors if they are not valid (such as going through wall)
      p[dim_x] = d->pc.position[dim_x];
      p[dim_y] = d->pc.position[dim_y];
    }

  d->pc.position[dim_x] = p[dim_x];
  d->pc.position[dim_y] = p[dim_y];
}
