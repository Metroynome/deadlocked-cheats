#include <libdl/pad.h>
#include <libdl/ui.h>
#include <libdl/graphics.h>
#include <libdl/stdio.h>
#include <libdl/color.h>
#include <libdl/string.h>
#include <libdl/game.h>
#include <libdl/gamesettings.h>
#include <libdl/mc.h>
#include "menu.h"

#define LINE_HEIGHT         (0.05)
#define LINE_HEIGHT_3_2     (0.075)
#define DEFAULT_GAMEMODE    (0)

int isConfigMenuActive = 0;
int selectedTabItem = 0;
u32 padPointer = 0;

//Save location
char * file = "/BASCUS-97465RATCHET/patch.bin";
int Saving = 0;
int fd = 0;

// Config
extern PatchConfig_t config;

// constants
const char footerText[] = "\x14 \x15 TAB     \x10 SELECT     \x12 BACK";

// menu display properties
const u32 colorBlack = 0x80000000;
const u32 colorBg = 0x80404040;
const u32 colorContentBg = 0x80202020;
const u32 colorTabBg = 0x80404040;
const u32 colorTabBarBg = 0x80101010;
const u32 colorRed = 0x80000040;
const u32 colorSelected = 0x80606060;
const u32 colorButtonBg = 0x80303030;
const u32 colorButtonFg = 0x80505050;
const u32 colorText = 0x80FFFFFF;
const u32 colorOpenBg = 0x20000000;

const float frameX = 0.1;
const float frameY = 0.15;
const float frameW = 0.8;
const float frameH = 0.7;
const float frameTitleH = 0.075;
const float frameFooterH = 0.05;
const float contentPaddingX = 0.01;
const float contentPaddingY = 0;
const float tabBarH = 0.075;
const float tabBarPaddingX = 0.005;

//
void configMenuDisable(void);
void configMenuEnable(void);

// action handlers
void buttonActionHandler(TabElem_t* tab, MenuElem_t* element, int actionType, void * actionArg);
void toggleActionHandler(TabElem_t* tab, MenuElem_t* element, int actionType, void * actionArg);
void toggleInvertedActionHandler(TabElem_t* tab, MenuElem_t* element, int actionType, void * actionArg);
void listActionHandler(TabElem_t* tab, MenuElem_t* element, int actionType, void * actionArg);
void gmOverrideListActionHandler(TabElem_t* tab, MenuElem_t* element, int actionType, void * actionArg);
void labelActionHandler(TabElem_t* tab, MenuElem_t* element, int actionType, void * actionArg);

// state handlers
void menuStateAlwaysHiddenHandler(TabElem_t* tab, MenuElem_t* element, int* state);
void menuStateAlwaysDisabledHandler(TabElem_t* tab, MenuElem_t* element, int* state);
void menuStateAlwaysEnabledHandler(TabElem_t* tab, MenuElem_t* element, int* state);
void menuLabelStateHandler(TabElem_t* tab, MenuElem_t* element, int* state);

void tabDefaultStateHandler(TabElem_t* tab, int * state);

// Saving
void SaveConfig();
void EnableSaving(TabElem_t* tab, MenuElem_t* element);
void RunSavingPopup();

void navMenu(TabElem_t* tab, int direction, int loop);
void navTab(int direction);

MenuElem_ListData_t dataInfiniteChargeboot = {
    &config.enableInfiniteChargeboot,
    NULL,
    3,
    {
      "Off",
      "Hold \x16",
      "Hold \x16 + \x14",
    }
};

MenuElem_ListData_t dataDistanceToShowNames = {
    &config.enableDistanceToShowNames,
    NULL,
    3,
    {
      "Normal",
      "Far",
      "Very Far",
    }
};

// In Game Codes
MenuElem_t menuElementsInGame[] = {
  { "Infinite Health/Moonjump", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableInfiniteHealthMoonjump },
  { "Free Cam", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableFreeCam },
  // { "Singleplayer Music", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableSingleplayerMusic },
  { "Follow Aimer", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableFollowAimer },
  { "Infinite Chargeboot", listActionHandler, menuStateAlwaysEnabledHandler, &dataInfiniteChargeboot },
  { "Render All", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableRenderAll },
  { "Rapid Fire Weapons and Vehicles", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableRapidFire },
  { "Walk Through Walls", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableWalkThroughWalls },
  { "Lots of Deaths", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableLotsOfDeaths },
  { "No Respawn Timer", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableNoRespawnTimer },
  { "Walk Fast", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableWalkFast },
  { "Airwalk", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableAirwalk },
  { "Flying Vehicles", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableFlyingVehicles },
  { "Surfing Vehicles", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableSurfingVehicles },
  { "Fast Vehicles", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableFastVehicles },
  { "Respawn Anywhere", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableRespawnAnywhere },
  // { "Disable vSync", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableVSync },
  { "Have All Omega and Alpha Mods", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableOmegaAlphaMods },
  { "Have All Skill Points", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableSkillPoints },
  { "Hacked Start Menu", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableHackedStartMenu },
  { "Cheats Menu: Weapons", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableCheatsMenuWeapons },
  { "Cheats Menu: End Game", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableCheatsMenuEndGame },
  { "Cheats Menu: Change Team", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableCheatsMenuChangeTeam },
  { "Show Fusion Aimer", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableFusionAimer },
  { "Lock-On Fusion", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableLockOnFusion },
  { "Distance to Show Names", listActionHandler, menuStateAlwaysEnabledHandler, &dataDistanceToShowNames },
  { "Freeze Time", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableFreezeTime }
};

// In Lobby Codes
MenuElem_t menuElementsInLobby[] = {
  { "Mask Username", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableMaskUsername },
  { "Hacked Keyboard", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableHackedKeyboard },
  { "Force G^", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableForceGUp },
  { "Host Options", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableHostOptions },
  { "Vehicle Select", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableVehicleSelect },
  { "Form Party and Unkick", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableFormPartyUnkick },
  { "Max Typing Limit", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableMaxTypingLimit },
  { "More Team Colors", toggleActionHandler, menuStateAlwaysEnabledHandler, &config.enableMoreTeamColors }
};

MenuElem_t menuElementsSave[] = {
  { "Save Settings", buttonActionHandler, menuStateAlwaysEnabledHandler, EnableSaving },
  { "", labelActionHandler, menuLabelStateHandler, (void*)LABELTYPE_HEADER },
  { "While in game, a popup will not show.", labelActionHandler, menuLabelStateHandler, (void*)LABELTYPE_HEADER }
};

// Credits
MenuElem_t menuElementsCredits[] = {
  { "", labelActionHandler, menuLabelStateHandler, (void*)LABELTYPE_HEADER },
  { "Dnawrkshp:  libdl, Mod Menu's UI and much more", labelActionHandler, menuLabelStateHandler, (void*)LABELTYPE_LABEL },
  { "Agent Moose/Metroynome: Codes and modded UI", labelActionHandler, menuLabelStateHandler, (void*)LABELTYPE_LABEL }
};

// tab items
TabElem_t tabElements[] = {
  { "In Game", tabDefaultStateHandler, menuElementsInGame, sizeof(menuElementsInGame)/sizeof(MenuElem_t) },
  { "In Lobby", tabDefaultStateHandler, menuElementsInLobby, sizeof(menuElementsInLobby)/sizeof(MenuElem_t) },
  { "Save", tabDefaultStateHandler, menuElementsSave, sizeof(menuElementsSave)/sizeof(MenuElem_t) },
  { "Credits", tabDefaultStateHandler, menuElementsCredits, sizeof(menuElementsCredits)/sizeof(MenuElem_t) }
};

const int tabsCount = sizeof(tabElements)/sizeof(TabElem_t);

// 
void tabDefaultStateHandler(TabElem_t* tab, int * state)
{
  *state = ELEMENT_SELECTABLE | ELEMENT_VISIBLE | ELEMENT_EDITABLE;
}

//------------------------------------------------------------------------------
void menuStateAlwaysHiddenHandler(TabElem_t* tab, MenuElem_t* element, int* state)
{
  *state = ELEMENT_HIDDEN;
}

// 
void menuStateAlwaysDisabledHandler(TabElem_t* tab, MenuElem_t* element, int* state)
{
  *state = ELEMENT_VISIBLE | ELEMENT_SELECTABLE;
}

// 
void menuStateAlwaysEnabledHandler(TabElem_t* tab, MenuElem_t* element, int* state)
{
  *state = ELEMENT_VISIBLE | ELEMENT_EDITABLE | ELEMENT_SELECTABLE;
}

// 
void menuLabelStateHandler(TabElem_t* tab, MenuElem_t* element, int* state)
{
  *state = ELEMENT_VISIBLE | ELEMENT_EDITABLE;
}

int getMenuElementState(TabElem_t* tab, MenuElem_t* element)
{
  // get tab and element state
  int tabState = 0, state = 0;
  tab->stateHandler(tab, &tabState);
  element->stateHandler(tab, element, &state);

  return tabState & state;
}

//------------------------------------------------------------------------------
void drawToggleMenuElement(TabElem_t* tab, MenuElem_t* element, RECT* rect)
{
  // get element state
  int state = getMenuElementState(tab, element);

  float x,y;
  float lerp = (state & ELEMENT_EDITABLE) ? 0.0 : 0.5;
  u32 color = colorLerp(colorText, 0, lerp);

  // draw name
  x = (rect->TopLeft[0] * SCREEN_WIDTH) + 5;
  y = (rect->TopLeft[1] * SCREEN_HEIGHT) + 5;
  gfxScreenSpaceText(x, y, 1, 1, color, element->name, -1, 0);

  // draw value
  x = (rect->TopRight[0] * SCREEN_WIDTH) - 5;
  gfxScreenSpaceText(x, y, 1, 1, color, *(char*)element->userdata ? "On" : "Off", -1, 2);
}

//------------------------------------------------------------------------------
void drawToggleInvertedMenuElement(TabElem_t* tab, MenuElem_t* element, RECT* rect)
{
  // get element state
  int state = getMenuElementState(tab, element);

  float x,y;
  float lerp = (state & ELEMENT_EDITABLE) ? 0.0 : 0.5;
  u32 color = colorLerp(colorText, 0, lerp);

  // draw name
  x = (rect->TopLeft[0] * SCREEN_WIDTH) + 5;
  y = (rect->TopLeft[1] * SCREEN_HEIGHT) + 5;
  gfxScreenSpaceText(x, y, 1, 1, color, element->name, -1, 0);

  // draw value
  x = (rect->TopRight[0] * SCREEN_WIDTH) - 5;
  gfxScreenSpaceText(x, y, 1, 1, color, *(char*)element->userdata ? "Off" : "On", -1, 2);
}

//------------------------------------------------------------------------------
void drawListMenuElement(TabElem_t* tab, MenuElem_t* element, MenuElem_ListData_t * listData, RECT* rect)
{
  // get element state
  int state = getMenuElementState(tab, element);

  float x,y;
  float lerp = (state & ELEMENT_EDITABLE) ? 0.0 : 0.5;
  u32 color = colorLerp(colorText, 0, lerp);

  // draw name
  x = (rect->TopLeft[0] * SCREEN_WIDTH) + 5;
  y = (rect->TopLeft[1] * SCREEN_HEIGHT) + 5;
  gfxScreenSpaceText(x, y, 1, 1, color, element->name, -1, 0);

  // draw value
  x = (rect->TopRight[0] * SCREEN_WIDTH) - 5;
  gfxScreenSpaceText(x, y, 1, 1, color, listData->items[(int)*listData->value], -1, 2);
}

//------------------------------------------------------------------------------
void drawButtonMenuElement(TabElem_t* tab, MenuElem_t* element, RECT* rect)
{
  // get element state
  int state = getMenuElementState(tab, element);

  float x,y,b = 0.005;
  float lerp = (state & ELEMENT_EDITABLE) ? 0.0 : 0.5;
  u32 color;
  RECT rBg = {
    { rect->TopLeft[0] + 0.05, rect->TopLeft[1] },
    { rect->TopRight[0] - 0.05, rect->TopRight[1] },
    { rect->BottomLeft[0] + 0.05, rect->BottomLeft[1] },
    { rect->BottomRight[0] - 0.05, rect->BottomRight[1] },
  };
  RECT rFg = {
    { rBg.TopLeft[0] + b, rBg.TopLeft[1] + b },
    { rBg.TopRight[0] - b, rBg.TopRight[1] + b },
    { rBg.BottomLeft[0] + b, rBg.BottomLeft[1] - b },
    { rBg.BottomRight[0] - b, rBg.BottomRight[1] - b },
  };

  // bg
  color = colorLerp(colorButtonBg, 0, lerp);
	gfxScreenSpaceQuad(&rBg, color, color, color, color);

  // fg
  color = colorLerp(colorButtonFg, 0, lerp);
	gfxScreenSpaceQuad(&rFg, color, color, color, color);

  // draw name
  x = 0.5 * SCREEN_WIDTH;
  y = ((rFg.TopLeft[1] + rFg.BottomLeft[1]) * SCREEN_HEIGHT * 0.5);
  gfxScreenSpaceText(x, y, 1, 1, colorLerp(colorText, 0, lerp), element->name, -1, 4);

  // add some padding
  rect->TopLeft[1] += 0.01;
  rect->TopRight[1] += 0.01;
}

//------------------------------------------------------------------------------
void drawLabelMenuElement(TabElem_t* tab, MenuElem_t* element, RECT* rect)
{
  // get element state
  int state = getMenuElementState(tab, element);

  float x,y;
  float lerp = (state & ELEMENT_EDITABLE) ? 0.0 : 0.5;

  // draw label
  x = 0.5 * SCREEN_WIDTH;
  y = ((rect->TopLeft[1] + rect->BottomLeft[1]) * SCREEN_HEIGHT * 0.5);
  gfxScreenSpaceText(x, y, 1, 1, colorLerp(colorText, 0, lerp), element->name, -1, 4);
}

//------------------------------------------------------------------------------
void buttonActionHandler(TabElem_t* tab, MenuElem_t* element, int actionType, void * actionArg)
{
  // get element state
  int state = getMenuElementState(tab, element);

  // do nothing if hidden
  if ((state & ELEMENT_VISIBLE) == 0)
    return;

  switch (actionType)
  {
    case ACTIONTYPE_SELECT:
    {
      if ((state & ELEMENT_EDITABLE) == 0)
        break;

      if (element->userdata)
        ((ButtonSelectHandler)element->userdata)(tab, element);
      break;
    }
    case ACTIONTYPE_GETHEIGHT:
    {
      *(float*)actionArg = LINE_HEIGHT * 2;
      break;
    }
    case ACTIONTYPE_DRAW:
    {
      drawButtonMenuElement(tab, element, (RECT*)actionArg);
      break;
    }
  }
}

//------------------------------------------------------------------------------
void labelActionHandler(TabElem_t* tab, MenuElem_t* element, int actionType, void * actionArg)
{
  // get element state
  int state = getMenuElementState(tab, element);

  // do nothing if hidden
  if ((state & ELEMENT_VISIBLE) == 0)
    return;

  switch (actionType)
  {
    case ACTIONTYPE_GETHEIGHT:
    {
      switch ((int)element->userdata)
      {
        case LABELTYPE_LABEL:
        {
          *(float*)actionArg = LINE_HEIGHT * 0.75;
          break;
        }
        default:
        {
          *(float*)actionArg = LINE_HEIGHT * 2;
          break;
        }
      }
      break;
    }
    case ACTIONTYPE_DRAW:
    {
      drawLabelMenuElement(tab, element, (RECT*)actionArg);
      break;
    }
  }
}

//------------------------------------------------------------------------------
void listActionHandler(TabElem_t* tab, MenuElem_t* element, int actionType, void * actionArg)
{
  MenuElem_ListData_t* listData = (MenuElem_ListData_t*)element->userdata;
  int itemCount = listData->count;

  // get element state
  int state = getMenuElementState(tab, element);

  // do nothing if hidden
  if ((state & ELEMENT_VISIBLE) == 0)
    return;

  switch (actionType)
  {
    case ACTIONTYPE_INCREMENT:
    case ACTIONTYPE_SELECT:
    {
      if ((state & ELEMENT_EDITABLE) == 0)
        break;
      char newValue = *listData->value;

      do
      {
        newValue += 1;
        if (newValue >= itemCount)
          newValue = 0;
        if (listData->stateHandler == NULL || listData->stateHandler(listData, newValue))
          break;
      } while (newValue != *listData->value);

      *listData->value = newValue;
      break;
    }
    case ACTIONTYPE_DECREMENT:
    {
      if ((state & ELEMENT_EDITABLE) == 0)
        break;
      char newValue = *listData->value;

      do
      {
        newValue -= 1;
        if (newValue < 0)
          newValue = itemCount - 1;
        if (listData->stateHandler == NULL || listData->stateHandler(listData, newValue))
          break;
      } while (newValue != *listData->value);

      *listData->value = newValue;
      break;
    }
    case ACTIONTYPE_GETHEIGHT:
    {
      *(float*)actionArg = LINE_HEIGHT;
      break;
    }
    case ACTIONTYPE_DRAW:
    {
      drawListMenuElement(tab, element, listData, (RECT*)actionArg);
      break;
    }
    case ACTIONTYPE_VALIDATE:
    {
      if (listData->stateHandler != NULL && !listData->stateHandler(listData, *listData->value))
        *listData->value = 0;
      break;
    }
  }
}

//------------------------------------------------------------------------------
void toggleInvertedActionHandler(TabElem_t* tab, MenuElem_t* element, int actionType, void * actionArg)
{
  // get element state
  int state = getMenuElementState(tab, element);

  // do nothing if hidden
  if ((state & ELEMENT_VISIBLE) == 0)
    return;

  switch (actionType)
  {
    case ACTIONTYPE_INCREMENT:
    case ACTIONTYPE_SELECT:
    case ACTIONTYPE_DECREMENT:
    {
      if ((state & ELEMENT_EDITABLE) == 0)
        break;

      // toggle
      *(char*)element->userdata = !(*(char*)element->userdata);
      break;
    }
    case ACTIONTYPE_GETHEIGHT:
    {
      *(float*)actionArg = LINE_HEIGHT;
      break;
    }
    case ACTIONTYPE_DRAW:
    {
      drawToggleInvertedMenuElement(tab, element, (RECT*)actionArg);
      break;
    }
  }
}

//------------------------------------------------------------------------------
void toggleActionHandler(TabElem_t* tab, MenuElem_t* element, int actionType, void * actionArg)
{
  // get element state
  int state = getMenuElementState(tab, element);

  // do nothing if hidden
  if ((state & ELEMENT_VISIBLE) == 0)
    return;

  switch (actionType)
  {
    case ACTIONTYPE_INCREMENT:
    case ACTIONTYPE_SELECT:
    case ACTIONTYPE_DECREMENT:
    {
      if ((state & ELEMENT_EDITABLE) == 0)
        break;
      // toggle
      *(char*)element->userdata = !(*(char*)element->userdata);;
      break;
    }
    case ACTIONTYPE_GETHEIGHT:
    {
      *(float*)actionArg = LINE_HEIGHT;
      break;
    }
    case ACTIONTYPE_DRAW:
    {
      drawToggleMenuElement(tab, element, (RECT*)actionArg);
      break;
    }
  }
}

//------------------------------------------------------------------------------
void drawFrame(void)
{
  int i;
  TabElem_t * tab = NULL;
  int state = 0;
  float tabX = frameX;
  float tabY = frameY + frameTitleH;

  // bg
  gfxScreenSpaceBox(frameX, frameY, frameW, frameH, colorBg);

  // title bg
  gfxScreenSpaceBox(frameX, frameY, frameW, frameTitleH, colorRed);

  // title
  gfxScreenSpaceText(0.5 * SCREEN_WIDTH, (frameY + frameTitleH * 0.5) * SCREEN_HEIGHT, 1, 1, colorText, "Mod Menu", -1, 4);

  // footer bg
  gfxScreenSpaceBox(frameX, frameY + frameH - frameFooterH, frameW, frameFooterH, colorRed);

  // footer
  gfxScreenSpaceText(((frameX + frameW) * SCREEN_WIDTH) - 5, (frameY + frameH) * SCREEN_HEIGHT - 5, 1, 1, colorText, footerText, -1, 8);

  // content bg
  gfxScreenSpaceBox(frameX + contentPaddingX, frameY + frameTitleH + tabBarH + contentPaddingY, frameW - (contentPaddingX*2), frameH - frameTitleH - tabBarH - frameFooterH - (contentPaddingY * 2), colorContentBg);

  // tab bar
  gfxScreenSpaceBox(tabX, tabY, frameW, tabBarH, colorTabBarBg);

  // tabs
  for (i = 0; i < tabsCount; ++i)
  {
    // get tab state
    tab = &tabElements[i];
    tab->stateHandler(tab, &state);

    // skip hidden elements
    if (state & ELEMENT_VISIBLE)
    {
      // get tab title width
      float pWidth = (4 * tabBarPaddingX) + gfxGetFontWidth(tab->name, -1, 1) / (float)SCREEN_WIDTH;

      // get color
      float lerp = state & ELEMENT_EDITABLE ? 0.0 : 0.5;
      u32 color = colorLerp(colorText, 0, lerp);

      // draw bar
      u32 barColor = selectedTabItem == i ? colorSelected : colorTabBg;
      gfxScreenSpaceBox(tabX + tabBarPaddingX, tabY, pWidth - (2 * tabBarPaddingX), tabBarH, barColor);

      // draw text
      gfxScreenSpaceText((tabX + 2*tabBarPaddingX) * SCREEN_WIDTH, (tabY + (0.5 * tabBarH)) * SCREEN_HEIGHT, 1, 1, color, tab->name, -1, 3);

      // increment X
      tabX += pWidth - tabBarPaddingX;
    }
  }
}


//------------------------------------------------------------------------------
void drawTab(TabElem_t* tab)
{
  if (!tab)
    return;

  int i = 0, state = 0;
  int menuElementRenderEnd = tab->menuOffset;
  MenuElem_t * menuElements = tab->elements;
	int menuElementsCount = tab->elementsCount;
  MenuElem_t* currentElement;

  float contentX = frameX + contentPaddingX;
  float contentY = frameY + frameTitleH + tabBarH + contentPaddingY;
  float contentW = frameW - (contentPaddingX * 2);
  float contentH = frameH - frameTitleH - tabBarH - frameFooterH - (contentPaddingY * 2);
  RECT drawRect = {
    { contentX, contentY },
    { contentX + contentW, contentY },
    { contentX, contentY },
    { contentX + contentW, contentY }
  };

  // draw items
  for (i = tab->menuOffset; i < menuElementsCount; ++i)
  {
    currentElement = &menuElements[i];
    float itemHeight = 0;
    currentElement->handler(tab, currentElement, ACTIONTYPE_GETHEIGHT, &itemHeight);

    // ensure item is within content bounds
    if ((drawRect.BottomLeft[1] + itemHeight) > (contentY + contentH))
      break;

    // set rect to height
    drawRect.BottomLeft[1] = drawRect.TopLeft[1] + itemHeight;
    drawRect.BottomRight[1] = drawRect.TopRight[1] + itemHeight;

    // draw selection
    if (i == tab->selectedMenuItem) {
      state = getMenuElementState(tab, currentElement);
      if (state & ELEMENT_SELECTABLE) {
        gfxScreenSpaceQuad(&drawRect, colorSelected, colorSelected, colorSelected, colorSelected);
      }
    }

    // draw
    currentElement->handler(tab, currentElement, ACTIONTYPE_DRAW, &drawRect);

    // increment rect
    drawRect.TopLeft[1] += itemHeight;
    drawRect.TopRight[1] += itemHeight;

    menuElementRenderEnd = i + 1;
  }
  
  // draw scroll bar
  if (tab->menuOffset > 0 || menuElementRenderEnd < menuElementsCount)
  {
    float scrollValue = tab->menuOffset / (float)(menuElementsCount - (menuElementRenderEnd-tab->menuOffset));
    float scrollBarHeight = 0.05;
    float contentRectHeight = contentH - scrollBarHeight;

    gfxScreenSpaceBox(contentX + contentW, contentY + (scrollValue * contentRectHeight), 0.01, scrollBarHeight, colorRed);
  }

  // 
  if (tab->selectedMenuItem >= menuElementRenderEnd)
    ++tab->menuOffset;
  if (tab->selectedMenuItem < tab->menuOffset)
    tab->menuOffset = tab->selectedMenuItem;

  // get selected element
  if (tab->selectedMenuItem >= menuElementsCount)
    return;

  currentElement = &menuElements[tab->selectedMenuItem];
  state = getMenuElementState(tab, currentElement);

  // find next selectable item if hidden or not selectable
  if ((state & ELEMENT_VISIBLE) == 0 || (state & ELEMENT_SELECTABLE) == 0)
    navMenu(tab, 1, 1);

  // Don't move cursor if saving
  if (Saving != 0)
    return;

  // nav down
  if (padGetButtonDown(0, PAD_DOWN) > 0)
  {
    navMenu(tab, 1, 0);
  }
  // nav up
  else if (padGetButtonDown(0, PAD_UP) > 0)
  {
    navMenu(tab, -1, 0);
  }
  // nav select
  else if (padGetButtonDown(0, PAD_CROSS) > 0)
  {
    if (state & ELEMENT_EDITABLE)
      currentElement->handler(tab, currentElement, ACTIONTYPE_SELECT, NULL);
  }
  // nav inc
  else if (padGetButtonDown(0, PAD_RIGHT) > 0)
  {
    if (state & ELEMENT_EDITABLE)
      currentElement->handler(tab, currentElement, ACTIONTYPE_INCREMENT, NULL);
  }
  // nav dec
  else if (padGetButtonDown(0, PAD_LEFT) > 0)
  {
    if (state & ELEMENT_EDITABLE)
      currentElement->handler(tab, currentElement, ACTIONTYPE_DECREMENT, NULL);
  }
  else if (padGetButtonDown(0, PAD_SQUARE) > 0)
  {
    RunSavingPopup(); // do something
  }
}

//------------------------------------------------------------------------------
void onMenuUpdate(int inGame)
{
  TabElem_t* tab = &tabElements[selectedTabItem];

  if (isConfigMenuActive)
  {
		// prevent pad from affecting menus
		padDisableInput();

		// draw
		if (padGetButton(0, PAD_L3) <= 0)
		{
			// draw frame
			drawFrame();

			// draw tab
			drawTab(tab);
		}

    // if Save button is pressed
    RunSavingPopup();

    // Don't switch tabs if saving
    if (Saving != 0)
      return;

		// nav tab right
		if (padGetButtonDown(0, PAD_R1) > 0)
		{
			navTab(1);
		}
		// nav tab left
		else if (padGetButtonDown(0, PAD_L1) > 0)
		{
			navTab(-1);
		}
		// close
		else if (padGetButtonUp(0, PAD_TRIANGLE) > 0 || padGetButtonDown(0, PAD_R3 | PAD_L3) > 0)
		{
			configMenuDisable();
		}
	}
	else if (!inGame)
  {
		if (uiGetActive() == UI_ID_ONLINE_MAIN_MENU)
		{
			// render message
			gfxScreenSpaceBox(0.1, 0.855, 0.4, 0.05, colorOpenBg);
			gfxScreenSpaceText(SCREEN_WIDTH * 0.3, SCREEN_HEIGHT * 0.855, 1, 1, 0x80FFFFFF, "\x18 + \x19 Open Mod Menu", -1, 4);
		}

		// check for pad input
		if (padGetButtonDown(0, PAD_R3 | PAD_L3) > 0)
		{
			configMenuEnable();
		}
	}
}

//------------------------------------------------------------------------------
void navMenu(TabElem_t* tab, int direction, int loop)
{
  int newElement = tab->selectedMenuItem + direction;
  MenuElem_t *elem = NULL;
  int state = 0;

  // handle case where tab has no items
  if (tab->elementsCount == 0)
  {
    tab->selectedMenuItem = 0;
    tab->menuOffset = 0;
    return;
  }

  while (newElement != tab->selectedMenuItem)
  {
    if (newElement >= tab->elementsCount)
    {
      if (loop && tab->selectedMenuItem != 0)
        newElement = 0;
      else
        break;
    }
    else if (newElement < 0)
    {
      if (loop && tab->selectedMenuItem != (tab->elementsCount - 1))
        newElement = tab->elementsCount - 1;
      else
        break;
    }

    // get newly selected element state
    elem = &tab->elements[newElement];
    elem->stateHandler(tab, elem, &state);

    // skip if hidden
    if ((state & ELEMENT_VISIBLE) == 0 || (state & ELEMENT_SELECTABLE) == 0)
    {
      newElement += direction;
      continue;
    }

    // set new tab
    tab->selectedMenuItem = newElement;
    break;
  }
}

void navTab(int direction)
{
  int newTab = selectedTabItem + direction;
  TabElem_t *tab = NULL;
  int state = 0;

  while (1)
  {
    if (newTab >= tabsCount)
      break;
    if (newTab < 0)
      break;
    
    // get new tab state
    tab = &tabElements[newTab];
    tab->stateHandler(tab, &state);

    // skip if hidden
    if ((state & ELEMENT_VISIBLE) == 0 || (state & ELEMENT_SELECTABLE) == 0)
    {
      newTab += direction;
      continue;
    }

    // set new tab
    selectedTabItem = newTab;
    break;
  }
}

//------------------------------------------------------------------------------
void onConfigGameMenu(void)
{
  onMenuUpdate(1);
}

//------------------------------------------------------------------------------
void onConfigOnlineMenu(void)
{
  onMenuUpdate(0);
}

//------------------------------------------------------------------------------
void configMenuDisable(void)
{
  if (!isConfigMenuActive)
    return;
  
  isConfigMenuActive = 0;

  // re-enable pad
  padEnableInput();
}

//------------------------------------------------------------------------------
void configMenuEnable(void)
{
  // enable
  isConfigMenuActive = 1;

  // return to first tab if current is hidden
  int state = 0;
  tabElements[selectedTabItem].stateHandler(&tabElements[selectedTabItem], &state);
  // if ((state & ELEMENT_SELECTABLE) == 0 || (state & ELEMENT_VISIBLE) == 0)
  selectedTabItem = 0;
}

void RunSavingPopup(void)
{
	if (Saving == 1)
	{
    u32 bgColorDownload = 0x70000000;
		// render background
		gfxScreenSpaceBox(0.2, 0.35, 0.6, 0.3, bgColorDownload);

		// flash color
		u32 downloadColor = 0x80808080;
		int gameTime = ((gameGetTime()/100) % 15);
		if (gameTime > 7)
			gameTime = 15 - gameTime;
		downloadColor += 0x101010 * gameTime;

		// render text
		gfxScreenSpaceText(SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5, 1, 1, downloadColor, "Saving configuration, please wait...", -1, 4);
	}
  else if (Saving == 2)
  {
    configMenuDisable();
    fd = 0;
    Saving = 0;
    // printf("\nSave: %d", Saving);
  }
  return;
}

void EnableSaving(TabElem_t* tab, MenuElem_t* element)
{
  if (Saving == 0)
  {
    Saving = 1;
    SaveConfig();
  }
}

void SaveConfig(void)
{
  // was original 0x10000, which is the whole file, but we actually don't need it.
  // I do apply a buffer just in case if we need it, but probably wont.
  char copy[0x1060];
  // Port, Slot, Path, Mode
  /*
    Modes:
    Read: 1
    Write: 2
  */
  int Open = McOpen(0, 0, file, 2);
  if(Open == 0)
  {
    printf("\nFile opened");
  }
  else
  {
    printf("\nError opening file.");
  }
  McSync(0, NULL, &fd);
  memcpy(copy, (u8*)0x01DFF000, 0x1060); // dest, src, size
  McWrite(fd, &copy, 0x1060); // fd, data, size
  McSync(0, NULL, &fd);
  int Close = McClose(fd);
  if(Close == 0)
  {
    printf("\nFile Closed");
  }
  else
  {
    printf("\nError closing file");
  }

  // printf("\nfd: %d", fd);
  // printf("\nSave: %d", Saving);
  Saving = 2;
}
