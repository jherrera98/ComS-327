#ifndef DESCRIPTIONS_H
# define DESCRIPTIONS_H

# include <stdint.h>
# undef swap
# include <vector>
# include <string>
# include "dice.h"
# include "npc.h"

typedef struct dungeon dungeon_t;

uint32_t parse_descriptions(dungeon_t *d);
uint32_t print_descriptions(dungeon_t *d);
uint32_t destroy_descriptions(dungeon_t *d);
void gen_objects(dungeon_t *d);

typedef enum object_type {
  objtype_no_type,
  objtype_WEAPON,
  objtype_OFFHAND,
  objtype_RANGED,
  objtype_LIGHT,
  objtype_ARMOR,
  objtype_HELMET,
  objtype_CLOAK,
  objtype_GLOVES,
  objtype_BOOTS,
  objtype_AMULET,
  objtype_RING,
  objtype_SCROLL,
  objtype_BOOK,
  objtype_FLASK,
  objtype_GOLD,
  objtype_AMMUNITION,
  objtype_FOOD,
  objtype_WAND,
  objtype_CONTAINER
} object_type_t;

extern const char object_symbol[];


class monster_description {
 private:
  std::string name, description;
  char symbol;
  std::vector<uint32_t> color;
  uint32_t abilities;
  dice speed, hitpoints, damage;
  uint32_t rarity;
 public:
  monster_description() : name(),       description(), symbol(0),   color(0),
                          abilities(0), speed(),       hitpoints(), damage(),
                          rarity(0)
  {
  }
  void set(const std::string &name,
           const std::string &description,
           const char symbol,
           const std::vector<uint32_t> &color,
           const dice &speed,
           const uint32_t abilities,
           const dice &hitpoints,
           const dice &damage,
           const uint32_t rarity);
  std::ostream &print(std::ostream &o);
  char get_symbol() { return symbol; }

  npc createMonster(){
    
    npc monster(name, description, extractColor(color), abilities, hitpoints.roll(), damage, 
		rarity, symbol, speed.roll());
    return monster;
  }
  uint32_t extractColor(std::vector<uint32_t> colors){
    uint32_t color = colors.at(0);
    return color;
  }
};


//New object class for the init of object_description
class object{
 private:
  std::string name, description;
  object_type_t type;
  uint32_t color;
  int32_t hit;
  dice damage; 
  int32_t dodge; 
  int32_t defence;
  int32_t weight;
  int32_t speed;
  int32_t attribute;
  int32_t value;
  bool artifact;
  uint32_t rarity;
  char symbol;

 public:
 object(std::string name, std::string description, object_type_t type,
	uint32_t color, int32_t hit, dice damage, int32_t dodge,
	int32_t defence, int32_t weight, int32_t speed, int32_t attribute,
	int32_t value, bool artifact, uint32_t rarity)
   : name(name),    description(description), type(type),
    color(color),  hit(hit),         damage(damage),
    dodge(dodge),   defence(defence),     weight(weight),
    speed(speed),   attribute(attribute),   value(value),
    artifact(artifact), rarity(rarity), symbol(convertToSymbol(type))
    {
    }
  void set(const std::string &name,
           const std::string &description,
           const object_type_t type,
           const uint32_t color,
           const int32_t hit,
           const dice &damage,
           const int32_t dodge,
           const int32_t defence,
           const int32_t weight,
           const int32_t speed,
           const int32_t attrubute,
           const int32_t value,
           const bool artifact,
           const uint32_t rarity);
  std::ostream &print(std::ostream &o);
  /* Need all these accessors because otherwise there is a *
   * circular dependancy that is difficult to get around.  */
  inline const std::string &get_name() const { return name; }
  inline const std::string &get_description() const { return description; }
  inline const object_type_t get_type() const { return type; }
  inline const uint32_t get_color() const { return color; }
  inline const int32_t get_hit() const { return hit; }
  inline const dice &get_damage() const { return damage; }
  inline const int32_t get_dodge() const { return dodge; }
  inline const int32_t get_defence() const { return defence; }
  inline const int32_t get_weight() const { return weight; }
  inline const int32_t get_speed() const { return speed; }
  inline const int32_t get_attribute() const { return attribute; }
  inline const int32_t get_value() const { return value; }
  inline const char get_symbol() const { return symbol;}
  inline const char convertToSymbol(object_type_t t) const {
    char s;
    switch(t){
    case objtype_WEAPON:
      s = '|'; break;
    case objtype_OFFHAND:
      s = ')'; break;
    case objtype_RANGED:
      s = '}'; break;
    case objtype_LIGHT:
      s = '_'; break;
    case objtype_ARMOR:
      s = '['; break;
    case objtype_HELMET:
      s = ']'; break;
    case objtype_CLOAK:
      s = '('; break;
    case objtype_GLOVES:
      s = '{'; break;
    case objtype_BOOTS:
      s = '\\'; break;
    case objtype_AMULET:
      s = '"'; break;
    case objtype_RING:
      s = '='; break;
    case objtype_SCROLL:
      s = '~'; break;
    case objtype_BOOK:
      s = '?'; break;
    case objtype_FLASK:
      s = '!'; break;
    case objtype_GOLD:
      s = '$'; break;
    case objtype_AMMUNITION:
      s = '/'; break;
    case objtype_FOOD:
      s = ','; break;
    case objtype_WAND:
      s = '-'; break;
    case objtype_CONTAINER:
      s = '%'; break;
    case objtype_no_type:
      s = '*'; break;
    default:
      s = '*';break;
    }
    return s;
  }
};

class object_description {
 private:
  std::string name, description;
  object_type_t type;
  uint32_t color;
  dice hit, damage, dodge, defence, weight, speed, attribute, value;
  bool artifact;
  uint32_t rarity;
 public:
 object_description() : name(),    description(), type(objtype_no_type),
                         color(0),  hit(),         damage(),
                         dodge(),   defence(),     weight(),
                         speed(),   attribute(),   value(),
                         artifact(false), rarity(0)
  {
  }
  void set(const std::string &name,
           const std::string &description,
           const object_type_t type,
           const uint32_t color,
           const dice &hit,
           const dice &damage,
           const dice &dodge,
           const dice &defence,
           const dice &weight,
           const dice &speed,
           const dice &attrubute,
           const dice &value,
           const bool artifact,
           const uint32_t rarity);
  std::ostream &print(std::ostream &o);
  /* Need all these accessors because otherwise there is a *
   * circular dependancy that is difficult to get around.  */
  inline const std::string &get_name() const { return name; }
  inline const std::string &get_description() const { return description; }
  inline const object_type_t get_type() const { return type; }
  inline const uint32_t get_color() const { return color; }
  inline const dice &get_hit() const { return hit; }
  inline const dice &get_damage() const { return damage; }
  inline const dice &get_dodge() const { return dodge; }
  inline const dice &get_defence() const { return defence; }
  inline const dice &get_weight() const { return weight; }
  inline const dice &get_speed() const { return speed; }
  inline const dice &get_attribute() const { return attribute; }
  inline const dice &get_value() const { return value; }
  inline const bool get_artifact() const { return artifact; }
  inline const uint32_t get_rarity() const { return rarity; }

  object createObject(){
    
    object o(get_name(), get_description(), get_type(), get_color(), get_hit().roll(), get_damage(),
	     get_dodge().roll(), get_defence().roll(), get_weight().roll(),
	     get_speed().roll(), get_attribute().roll(), get_value().roll(), get_artifact(), get_rarity());
    return o;
  }
};





std::ostream &operator<<(std::ostream &o, monster_description &m);
std::ostream &operator<<(std::ostream &o, object_description &od);
std::ostream &operator<<(std::ostream &o, object &od);

#endif
