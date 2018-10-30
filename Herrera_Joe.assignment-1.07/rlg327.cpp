#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

/* Very slow seed: 686846853 */

#include "dungeon.h"
#include "pc.h"
#include "npc.h"
#include "move.h"
#include "io.h"


const char *victory =
  "\n                                       o\n"
  "                                      $\"\"$o\n"
  "                                     $\"  $$\n"
  "                                      $$$$\n"
  "                                      o \"$o\n"
  "                                     o\"  \"$\n"
  "                oo\"$$$\"  oo$\"$ooo   o$    \"$    ooo\"$oo  $$$\"o\n"
  "   o o o o    oo\"  o\"      \"o    $$o$\"     o o$\"\"  o$      \"$  "
  "\"oo   o o o o\n"
  "   \"$o   \"\"$$$\"   $$         $      \"   o   \"\"    o\"         $"
  "   \"o$$\"    o$$\n"
  "     \"\"o       o  $          $\"       $$$$$       o          $  ooo"
  "     o\"\"\n"
  "        \"o   $$$$o $o       o$        $$$$$\"       $o        \" $$$$"
  "   o\"\n"
  "         \"\"o $$$$o  oo o  o$\"         $$$$$\"        \"o o o o\"  "
  "\"$$$  $\n"
  "           \"\" \"$\"     \"\"\"\"\"            \"\"$\"            \""
  "\"\"      \"\"\" \"\n"
  "            \"oooooooooooooooooooooooooooooooooooooooooooooooooooooo$\n"
  "             \"$$$$\"$$$$\" $$$$$$$\"$$$$$$ \" \"$$$$$\"$$$$$$\"  $$$\""
  "\"$$$$\n"
  "              $$$oo$$$$   $$$$$$o$$$$$$o\" $$$$$$$$$$$$$$ o$$$$o$$$\"\n"
  "              $\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\""
  "\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"$\n"
  "              $\"                                                 \"$\n"
  "              $\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\""
  "$\"$\"$\"$\"$\"$\"$\"$\n"
  "                                   You win!\n\n";

const char *tombstone =
  "\n\n\n\n                /\"\"\"\"\"/\"\"\"\"\"\"\".\n"
  "               /     /         \\             __\n"
  "              /     /           \\            ||\n"
  "             /____ /   Rest in   \\           ||\n"
  "            |     |    Pieces     |          ||\n"
  "            |     |               |          ||\n"
  "            |     |   A. Luser    |          ||\n"
  "            |     |               |          ||\n"
  "            |     |     * *   * * |         _||_\n"
  "            |     |     *\\/* *\\/* |        | TT |\n"
  "            |     |     *_\\_  /   ...\"\"\"\"\"\"| |"
  "| |.\"\"....\"\"\"\"\"\"\"\".\"\"\n"
  "            |     |         \\/..\"\"\"\"\"...\"\"\""
  "\\ || /.\"\"\".......\"\"\"\"...\n"
  "            |     |....\"\"\"\"\"\"\"........\"\"\"\"\""
  "\"^^^^\".......\"\"\"\"\"\"\"\"..\"\n"
  "            |......\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"......"
  "..\"\"\"\"\"....\"\"\"\"\"..\"\"...\"\"\".\n\n"
  "            You're dead.  Better luck in the next life.\n\n\n";

void usage(char *name)
{
  fprintf(stderr,
          "Usage: %s [-r|--rand <seed>] [-l|--load [<file>]]\n"
          "          [-s|--save [<file>]] [-i|--image <pgm file>]\n"
          "          [-n|--nummon <count>]",
          name);

  exit(-1);
}

using namespace std;


enum Abilities{
  SMART,
  TELE,
  TUNNEL,
  ERRATIC,
  PASS,
  PICKUP,
  DESSTROY,
  UNIQ,
  BOSS
};

enum Colors{
  RED,
  GREEN,
  BLUE,
  CYAN,
  YELLOW,
  MAGENTA,
  WHITE,
  BLACK
};


class Dice
{
 public:
  uint8_t base;
  uint8_t dice;
  uint8_t sides;

  //Returns the dice in a formatted string 
  string printDice(void);
};

string Dice::printDice(void){
  string temp;
  std::ostringstream string1;
  string1<< base;
  std::ostringstream string2;
  string2 << dice;
  std::ostringstream string3;
  string3 << sides;
  temp = string1.str() + "+" + string2.str() + "d" + string3.str();
  return temp;
}

class Monster{
 public:
  std::string NAME;
  string DESC;
  vector<string> COLOR;//vector<Colors> COLOR;
  Dice SPEED;
  vector<string> ABIL; //vector<Abilities> ABIL;
  string HP;//Dice HP;
  string DAM;//Dice DAM;
  //Dice die;
  string SYMB;
  string RRTY;

  //Prints the monster out to the console
  void printMonster(Monster m);
};

void Monster::printMonster(Monster m){
  cout << m.NAME;
  cout << m.DESC;
  cout << m.SYMB << endl;

  //Prints out colors
  vector<string>::iterator it;
  for(it = m.COLOR.begin(); it != m.COLOR.end(); it++){
    cout << *it << " ";
  }cout << endl;

  string speed = m.SPEED.printDice();
  cout << speed << endl;
  
  //Prints out Abilities
  for(it = m.ABIL.begin(); it != m.ABIL.end(); it++){
    cout << *it << " ";
  }cout << endl;
  
  cout << m.HP << endl;
  cout << m.DAM << endl;
  cout << m.RRTY << endl << endl;
}


int main(int argc, char *argv[])
{
  dungeon d;
  time_t seed;
  struct timeval tv;
  int32_t i;
  uint32_t do_load, do_save, do_seed, do_image, do_save_seed, do_save_image;
  uint32_t long_arg;
  char *save_file;
  char *load_file;
  char *pgm_file;

  //Assignment 1.07 
  
  //Setting the path of the file
  string filePath;
  filePath = getenv("HOME");
  filePath += "/.rlg327/";
  filePath +=  "monster_desc.txt";

  //Reading from file 
  ifstream f (filePath.c_str());

  //Storing the header of the file
  string title;
  getline(f, title); 

  //Checks to see if the header is appropriate
  if(title != "RLG327 MONSTER DESCRIPTION 1"){
    printf("\ninvalid header in file\n\n");
    exit (1);
  }

  cout<<endl;
  while(!f.eof())
    {
      string s;

      //Searches for the next monster to make
      getline(f, s);
      while (s != "BEGIN MONSTER"){
	getline(f,s);
	if(f.eof()){
	  break;
	}
      }

      Monster m;
      f>>s;// Comsumes newline
      
      int keywordCount = 0;
      while(s != "END"){

	if(s == "NAME"){
	  f.get();//consumes leading whitespace
	  getline(f,s);
	  m.NAME = s;
	}
	else if(s == "DESC"){
	  string description;
	  
	  while(f.peek() != '.'){
	    string line;
	    getline(f,line);

	    int byteCount =0;
	    
	    //read char by char until end of line
	    for(string::iterator it = line.begin(); it != line.end(); it++){
	      description += *it;
	      byteCount++;
	    }
	    if(byteCount > 77){ 
	      keywordCount = -10;//This is so that the monster will be thrown out in the end
	      }
	    description +="\n";
	  }
	  m.DESC = description;
	  getline(f,s);//consumes the last period
	}
	else if (s == "COLOR"){
	  vector<string> c;
	  
	  while(f.peek() != '\n'){
	    //consume the space and then gets the next color
	    f>>s;
	    c.push_back(s);
	  }
	  
	  m.COLOR = c;
	}
	else if(s == "SPEED"){
	  f>>s;//Gets the dice value as a string 

	  Dice d;

	  //Parses the intergers out of the string
	  stringstream stream(s);
	  int i=0;
	  int number;
	  while (stream >> number){
	    cout << number<< endl;
	    switch(i){
	    case 0: d.base = number; 
	      break;
	    case 1: d.dice = number; 
	      break;
	    case 2: d.sides = 10;
	      break;
	    default: keywordCount = -10;//This is so the moster gets thrown away
	      break;
	    }
	    i++;
	  }
	  m.SPEED = d;      
	}
	else if (s == "ABIL"){
	  vector<string> a;
	  
	  while(f.peek() != '\n'){
	    f>>s;
	    a.push_back(s);
	  }
	  m.ABIL = a;
	}
	else if(s == "HP"){
	  f>>s;//Gets the dice value
	  m.HP = s;
	}
	else if(s == "DAM"){
	  f>>s;//Gets the dice value
	  m.DAM = s;
	}
	else if(s == "SYMB"){
	  f>>s;//Gets the dice value
	  m.SYMB = s;
	}
	else if (s == "RRTY"){
	  f>>s;//Gets the dice value	  
	  m.RRTY = s;  
	}
	else{
	  //Discard monster by breaking the while loop
	  break;
	}
	//Gets the next keyword
	f>>s;
	keywordCount++;
      }
      //This checks to see if there were only 9 keywords
      if (keywordCount == 9){
	m.printMonster(m);
      } 
    }
  
  exit(1);
  
  /* Quiet a false positive from valgrind. */
  memset(&d, 0, sizeof (d));
  
  /* Default behavior: Seed with the time, generate a new dungeon, *
   * and don't write to disk.                                      */
  do_load = do_save = do_image = do_save_seed = do_save_image = 0;
  do_seed = 1;
  save_file = load_file = NULL;
  d.max_monsters = MAX_MONSTERS;

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
        case 'n':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-nummon")) ||
              argc < ++i + 1 /* No more arguments */ ||
              !sscanf(argv[i], "%hu", &d.max_monsters)) {
            usage(argv[0]);
          }
          break;
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

  srand(seed);

  io_init_terminal();
  init_dungeon(&d);

  if (do_load) {
    read_dungeon(&d, load_file);
  } else if (do_image) {
    read_pgm(&d, pgm_file);
  } else {
    gen_dungeon(&d);
  }

  /* Ignoring PC position in saved dungeons.  Not a bug. */
  config_pc(&d);
  gen_monsters(&d);

  io_display(&d);
  if (!do_load && !do_image) {
    io_queue_message("Seed is %u.", seed);
  }
  while (pc_is_alive(&d) && dungeon_has_npcs(&d) && !d.quit) {
    do_moves(&d);
  }
  io_display(&d);

  io_reset_terminal();

  if (do_save) {
    if (do_save_seed) {
       /* 10 bytes for number, please dot, extention and null terminator. */
      save_file = (char *) malloc(18);
      sprintf(save_file, "%ld.rlg327", seed);
    }
    if (do_save_image) {
      if (!pgm_file) {
	fprintf(stderr, "No image file was loaded.  Using default.\n");
	do_save_image = 0;
      } else {
	/* Extension of 3 characters longer than image extension + null. */
	save_file = (char *) malloc(strlen(pgm_file) + 4);
	strcpy(save_file, pgm_file);
	strcpy(strchr(save_file, '.') + 1, "rlg327");
      }
    }
    write_dungeon(&d, save_file);

    if (do_save_seed || do_save_image) {
      free(save_file);
    }
  }

  printf("%s", pc_is_alive(&d) ? victory : tombstone);
  printf("You defended your life in the face of %u deadly beasts.\n"
         "You avenged the cruel and untimely murders of %u "
         "peaceful dungeon residents.\n",
         d.PC->kills[kill_direct], d.PC->kills[kill_avenged]);

  if (pc_is_alive(&d)) {
    /* If the PC is dead, it's in the move heap and will get automatically *
     * deleted when the heap destructs.  In that case, we can't call       *
     * delete_pc(), because it will lead to a double delete.               */
    character_delete(d.PC);
  }

  delete_dungeon(&d);

  return 0;
}
