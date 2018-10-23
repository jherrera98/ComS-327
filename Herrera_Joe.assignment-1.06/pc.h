#ifndef PC_H
# define PC_H

# include <stdint.h>

# include "dims.h"
# include "character.h"
# include "dungeon.h"

//typedef class dungeon dungeon_t;

//typedef enum terrain_type terrain_type_t;

//enum terrain_type : terrain_type_t; 

//enumterrain_type_t

class pc: public Character{
  public:
  terrain_type_t map[21][80]; 
};

void pc_delete(pc *pc);
uint32_t pc_is_alive(dungeon *d);
void config_pc(dungeon *d);
uint32_t pc_next_pos(dungeon *d, pair_t dir);
void place_pc(dungeon *d);
uint32_t pc_in_room(dungeon *d, uint32_t room);

#endif
