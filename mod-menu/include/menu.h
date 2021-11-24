#ifndef __PATCH_CONFIG_H__
#define __PATCH_CONFIG_H__

typedef struct PatchConfig
{
  // In Game
  char enableInfiniteHealthMoonjump;
  char enableFreeCam;
  char enableSingleplayerMusic;
  char enableFollowAimer;
  char enableInfiniteChargeboot;
  char enableRenderAll;
  char enableRapidFireWeapons;
  char enableWalkThroughWalls;
  char enableRapidFireVehicles;
  char enableLotsOfDeaths;
  char enableNoRespawnTimer;
  char enableWalkFast;
  char enableAirwalk;
  char enableFlyingVehicles;
  char enableSurfingVehicles;
  char enableFastVehicles;
  char enableRespawnAnywhere;
  char enableVSync;
  char enableOmegaAlphaMods;
  char enableSkillPoints;
  char enableCheatsMenuWeapons;
  char enableCheatsMenuEndGame;
  char enableCheatsMenuFusionAimer;
  char enableHackedStartMenu;
  char enableLockOnFusion;

  // In Lobby
  char enableMaskUsername;
  char enableHackedKeyboard;
  char enableForceGUp;
  char enableHostOptions;
  char enableVehicleSelect;
  char enableFormPartyUnkick;
  char enableMaxTypingLimit;
  char enableMoreTeamColors;
} PatchConfig_t;

enum ActionType
{
  ACTIONTYPE_DRAW,
  ACTIONTYPE_GETHEIGHT,
  ACTIONTYPE_SELECT,
  ACTIONTYPE_INCREMENT,
  ACTIONTYPE_DECREMENT,
  ACTIONTYPE_VALIDATE
};

enum ElementState
{
  ELEMENT_HIDDEN = 0,
  ELEMENT_VISIBLE = (1 << 0),
  ELEMENT_EDITABLE = (1 << 1),
  ELEMENT_SELECTABLE = (1 << 2),
};

enum LabelType
{
  LABELTYPE_HEADER,
  LABELTYPE_LABEL
};

struct MenuElem;
struct TabElem;
struct MenuElem_ListData;

typedef void (*ActionHandler)(struct TabElem* tab, struct MenuElem* element, int actionType, void * actionArg);
typedef void (*ButtonSelectHandler)(struct TabElem* tab, struct MenuElem* element);
typedef void (*MenuElementStateHandler)(struct TabElem* tab, struct MenuElem* element, int * state);
typedef int (*MenuElementListStateHandler)(struct MenuElem_ListData* listData, char value);
typedef void (*TabStateHandler)(struct TabElem* tab, int * state);

typedef struct MenuElem
{
  char name[48];
  ActionHandler handler;
  MenuElementStateHandler stateHandler;
  void * userdata;
} MenuElem_t;

typedef struct MenuElem_ListData
{
  char * value;
  MenuElementListStateHandler stateHandler;
  int count;
  char * items[];
} MenuElem_ListData_t;

typedef struct TabElem
{
  char name[32];
  TabStateHandler stateHandler;
  MenuElem_t * elements;
  int elementsCount;
  int selectedMenuItem;
  int menuOffset;
} TabElem_t;

#endif // __PATCH_CONFIG_H__
