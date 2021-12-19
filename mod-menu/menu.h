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
  char enableRapidFire;
  char enableWalkThroughWalls;
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
  char enableDistanceToShowNames;

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

enum UIPointers
{
    UIP_BETA_ONLINE_LOBBY = 0x0,
    UIP_GAME_DETAILS = 0x1,
    UIP_ONLINE_AGREEMENT_PAGE_2 = 0x2,
    UIP_BUDDY_LIST = 0x3,
    UIP_CLAN_CHALLENGE_LIST = 0x4,
    UIP_CLAN_1 = 0x5,
    UIP_CLAN_MESSAGE = 0x6,
    UIP_CLAN_3 = 0x7,
    UIP_CLAN_4 = 0x8,
    UIP_CLAN_INVITE = 0x9,
    UIP_CLAN_ROOM = 0xA,
    UIP_CLAN_YES_NO_MESSAGE_BOX = 0xB,
    UIP_CREATE_GAME = 0xC,
    UIP_DNAS_AUTH = 0xD,
    UIP_SELECT_NETWORK = 0xE,
    UIP_FIND_GAME = 0xF,
    UIP_GAME_INFO = 0x10,
    UIP_INVITE_BUDDY = 0x11,
    UIP_END_GAME_DETAILS = 0x12,
    UIP_IGNORED_PLAYERS = 0x13,
    UIP_KEYBOARD = 0x14,
    UIP_ONLINE_AGREEMENT_PAGE_1 = 0x15,
    UIP_STAGING_SECONDARY_PLAYER_OPTIONS = 0x16,
    UIP_CREATE_GAME1 = 0x17,
    UIP_END_GAME_DETAILS_2 = 0x18,
    UIP_LOCAL_PLAY_STAGING = 0x19,
    UIP_ONLINE_LOCAL_EDIT_PROFILE_MENU = 0x1A,
    UIP_ONLINE_LOBBY = 0x1B,
    UIP_FORM_PARTY = 0x1C,
    UIP_PLAYER_PROFILE = 0x1D,
    UIP_EDIT_PROFILE = 0x1E,
    UIP_SELECT_PROFILE = 0x1F,
    UIP_RECENT_PLAYERS = 0x20,
    UIP_STAGING = 0x21,
    UIP_IN_GAME_DETAILS = 0x22,
    UIP_CHANGE_SKIN_TEAM = 0x23,
    UIP_STATS = 0x24,
    UIP_VEHICLE_SELECT = 0x25,
    UIP_WEAPON_SELECT = 0x26,
    UIP_KEYBOARD_UNK = 0x27
};

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
