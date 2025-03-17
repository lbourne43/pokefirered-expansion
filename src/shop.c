#include "global.h"
#include "gflib.h"
#include "shop.h"
#include "menu.h"
#include "data.h"
#include "graphics.h"
#include "strings.h"
#include "list_menu.h"
#include "party_menu.h"
#include "field_specials.h"
#include "field_weather.h"
#include "task.h"
#include "item.h"
#include "item_menu.h"
#include "item_menu_icons.h"
#include "overworld.h"
#include "field_fadetransition.h"
#include "scanline_effect.h"
#include "item_icon.h"
#include "decompress.h"
#include "menu_indicators.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "money.h"
#include "move.h"
#include "quest_log.h"
#include "script.h"
#include "text_window.h"
#include "constants/field_weather.h"
#include "constants/game_stat.h"
#include "constants/item_menu.h"
#include "constants/items.h"
#include "constants/songs.h"

#define tItemCount data[1]
#define tItemId data[5]
#define tListTaskId data[7]

// mart types
enum
{
    MART_TYPE_REGULAR = 0,
    MART_TYPE_TMHM,
    MART_TYPE_DECOR,
    MART_TYPE_DECOR2,
};

// shop view window NPC info enum
enum
{
    OBJECT_EVENT_ID,
    X_COORD,
    Y_COORD,
    ANIM_NUM
};

struct ShopData
{
    /*0x00*/ void (*callback)(void);
    /*0x04*/ const u16 *itemList;
    /*0x08*/ u32 itemPrice;
    /*0x0C*/ u16 selectedRow;
    /*0x0E*/ u16 scrollOffset;
    /*0x10*/ u16 itemCount;
    /*0x12*/ u16 itemsShowed;
    /*0x14*/ u16 maxQuantity;
    /*0x16*/ u16 martType:4;    // 0x1 if tm list
             u16 fontId:5;
             u16 itemSlot:2;
             u16 unk16_11:5;
    /*0x18*/ u16 unk18;
};

static EWRAM_DATA s16 sViewportObjectEvents[OBJECT_EVENTS_COUNT][4] = {0};
static EWRAM_DATA struct ShopData sShopData = {0};
static EWRAM_DATA u8 sShopMenuWindowId = 0;
EWRAM_DATA u16 (*gShopTilemapBuffer1)[0x400] = {0};
EWRAM_DATA u16 (*gShopTilemapBuffer2)[0x400] = {0};
EWRAM_DATA u16 (*gShopTilemapBuffer3)[0x400] = {0};
EWRAM_DATA u16 (*gShopTilemapBuffer4)[0x400] = {0};
EWRAM_DATA struct ListMenuItem *sShopMenuListMenu = {0};
static EWRAM_DATA u8 (*sShopMenuItemStrings)[ITEM_NAME_LENGTH + 2] = {0};
EWRAM_DATA struct QuestLogEvent_Shop sHistory[2] = {0};

//Function Declarations
static u8 CreateShopMenu(u8 martType);
static u8 GetMartTypeFromItemList(u32 a0);
static void SetShopItemsForSale(const u16 *items);
static void SetShopMenuCallback(MainCallback callback);
static void Task_ShopMenu(u8 taskId);
static void Task_HandleShopMenuBuy(u8 taskId);
static void Task_HandleShopMenuSell(u8 taskId);
static void CB2_GoToSellMenu(void);
static void Task_HandleShopMenuQuit(u8 taskId);
static void ClearShopMenuWindow(void);
static void Task_GoToBuyOrSellMenu(u8 taskId);
static void MapPostLoadHook_ReturnToShopMenu(void);
static void Task_ReturnToShopMenu(u8 taskId);
static void ShowShopMenuAfterExitingBuyOrSellMenu(u8 taskId);
static void CB2_BuyMenu(void);
static void VBlankCB_BuyMenu(void);
static void CB2_InitBuyMenu(void);
static bool8 InitShopData(void);
static void BuyMenuInitBgs(void);
static void BuyMenuDecompressBgGraphics(void);
static void RecolorItemDescriptionBox(bool32 a0);
static void BuyMenuDrawGraphics(void);
static bool8 BuyMenuBuildListMenuTemplate(void);
static void PokeMartWriteNameAndIdAt(struct ListMenuItem *list, u16 index, u8 *dst);
static void BuyMenuPrintItemDescriptionAndShowItemIcon(s32 item, bool8 onInit, struct ListMenu *list);
static void BuyMenuPrintPriceInList(u8 windowId, u32 itemId, u8 y);
static void LoadTmHmNameInMart(s32 item);
static void BuyMenuPrintCursor(u8 listTaskId, u8 a1);
static void BuyMenuPrintCursorAtYPosition(u8 y, u8 a1);
static void BuyMenuFreeMemory(void);
static void SetShopExitCallback(void);
static void BuyMenuAddScrollIndicatorArrows(void);
static void BuyQuantityAddScrollIndicatorArrows(void);
static void BuyMenuRemoveScrollIndicatorArrows(void);
static void BuyMenuDrawMapView(void);
static void BuyMenuDrawMapBg(void);
static void BuyMenuDrawMapMetatile(s16 x, s16 y, const u16 *src, u8 metatileLayerType);
static void BuyMenuDrawMapMetatileLayer(u16 *dest, s16 offset1, s16 offset2, const u16 *src);
static void BuyMenuCollectObjectEventData(void);
static void BuyMenuDrawObjectEvents(void);
static void BuyMenuCopyTilemapData(void);
static void BuyMenuPrintItemQuantityAndPrice(u8 taskId);
static void Task_BuyMenu(u8 taskId);
static void Task_BuyHowManyDialogueInit(u8 taskId);
static void Task_BuyHowManyDialogueHandleInput(u8 taskId);
static void CreateBuyMenuConfirmPurchaseWindow(u8 taskId);
static void BuyMenuTryMakePurchase(u8 taskId);
static void BuyMenuSubtractMoney(u8 taskId);
static void Task_ReturnToItemListAfterItemPurchase(u8 taskId);
static void BuyMenuReturnToItemList(u8 taskId);
static void ExitBuyMenu(u8 taskId);
static void Task_ExitBuyMenu(u8 taskId);
static void DebugFunc_PrintPurchaseDetails(u8 taskId);
static void DebugFunc_PrintShopMenuHistoryBeforeClearMaybe(void);
static void RecordTransactionForQuestLog(void);
static void BuyMenuInitWindows(bool32 isSellingTM);
static void BuyMenuPrint(u8 windowId, u8 font, const u8 *text, u8 x, u8 y, u8 letterSpacing, u8 lineSpacing, u8 speed, u8 color);
static void BuyMenuDisplayMessage(u8 taskId, const u8 *text, TaskFunc callback);
static void BuyMenuQuantityBoxNormalBorder(u8 windowId, bool8 copyToVram);
static void BuyMenuQuantityBoxThinBorder(u8 windowId, bool8 copyToVram);
static void BuyMenuConfirmPurchase(u8 taskId, const struct YesNoFuncTable *yesNo);

static const struct MenuAction sShopMenuActions_BuySellQuit[] =
{
    {gText_ShopBuy, {.void_u8 = Task_HandleShopMenuBuy}},
    {gText_ShopSell, {.void_u8 = Task_HandleShopMenuSell}},
    {gText_ShopQuit, {.void_u8 = Task_HandleShopMenuQuit}}
};

static const struct YesNoFuncTable sShopMenuActions_BuyQuit =
{
    .yesFunc = BuyMenuTryMakePurchase, 
    .noFunc = BuyMenuReturnToItemList,
};

static const struct WindowTemplate sShopMenuWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 2,
    .tilemapTop = 1,
    .width = 12,
    .height = 6,
    .paletteNum = 15,
    .baseBlock = 8
};

static const struct BgTemplate sShopBuyMenuBgTemplates[4] =
{
    {
        .bg = 0,
        .charBaseIndex = 2,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 1,
        .charBaseIndex = 0,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    },
    {
        .bg = 2,
        .charBaseIndex = 0,
        .mapBaseIndex = 29,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0
    },
    {
        .bg = 3,
        .charBaseIndex = 0,
        .mapBaseIndex = 28,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0
    }
};

static const struct WindowTemplate sShopBuyMenuWindowTemplatesNormal[] =
{
    {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 1,
        .width = 8,
        .height = 3,
        .paletteNum = 15,
        .baseBlock = 0x27,
    },
    {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 11,
        .width = 13,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0x3F,
    },
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 15,
        .width = 26,
        .height = 4,
        .paletteNum = 14,
        .baseBlock = 0x59,
    },
    {
        .bg = 0,
        .tilemapLeft = 17,
        .tilemapTop = 9,
        .width = 12,
        .height = 4,
        .paletteNum = 14,
        .baseBlock = 0xC1,
    },
    {
        .bg = 0,
        .tilemapLeft = 11,
        .tilemapTop = 1,
        .width = 17,
        .height = 12,
        .paletteNum = 14,
        .baseBlock = 0xF1,
    },
    {
        .bg = 0,
        .tilemapLeft = 5,
        .tilemapTop = 14,
        .width = 25,
        .height = 6,
        .paletteNum = 15,
        .baseBlock = 0x1BD,
    },
    DUMMY_WIN_TEMPLATE,
};

// firered uses different layout when selling TMs
static const struct WindowTemplate sShopBuyMenuWindowTemplatesTM[] =
{
    {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 1,
        .width = 8,
        .height = 3,
        .paletteNum = 15,
        .baseBlock = 0x27,
    },
    {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 11,
        .width = 13,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0x3F,
    },
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 15,
        .width = 26,
        .height = 4,
        .paletteNum = 14,
        .baseBlock = 0x59,
    },
    {
        .bg = 0,
        .tilemapLeft = 17,
        .tilemapTop = 9,
        .width = 12,
        .height = 4,
        .paletteNum = 14,
        .baseBlock = 0xC1,
    },
    {
        .bg = 0,
        .tilemapLeft = 11,
        .tilemapTop = 1,
        .width = 17,
        .height = 10,
        .paletteNum = 14,
        .baseBlock = 0xF1,
    },
    {
        .bg = 0,
        .tilemapLeft = 12,
        .tilemapTop = 12,
        .width = 18,
        .height = 8,
        .paletteNum = 14,
        .baseBlock = 0x19B,
    },
    {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 14,
        .width = 10,
        .height = 4,
        .paletteNum = 14,
        .baseBlock = 0x22B,
    },
    DUMMY_WIN_TEMPLATE,
};

static const struct WindowTemplate sShopBuyMenuYesNoWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 21,
    .tilemapTop = 9,
    .width = 6,
    .height = 4,
    .paletteNum = 14,
    .baseBlock = 0xC1,
};

static const u8 sShopBuyMenuTextColors[][3] =
{
    {0, 1, 2},
    {0, 2, 3},
    {0, 3, 2}
};



// nettux

static const u16 sNettuxBattleInventory_ZeroBadges[] = {
    ITEM_SILK_SCARF,
    ITEM_CHARCOAL,
    ITEM_MYSTIC_WATER,
    ITEM_MAGNET,
    ITEM_MIRACLE_SEED,
    ITEM_NEVER_MELT_ICE,
    ITEM_BLACK_BELT,
    ITEM_POISON_BARB,
    ITEM_SOFT_SAND,
    ITEM_SHARP_BEAK,
    ITEM_TWISTED_SPOON,
    ITEM_SILVER_POWDER,
    ITEM_DRAGON_FANG,
    ITEM_BLACK_GLASSES,
    ITEM_SNOWBALL,
    ITEM_HARD_STONE,
    ITEM_SPELL_TAG,
    ITEM_METAL_COAT,
    ITEM_EXP_SHARE,
    ITEM_NONE
};

static const u16 sNettuxBattleInventory_OneBadge[] = {
    ITEM_SILK_SCARF,
    ITEM_CHARCOAL,
    ITEM_MYSTIC_WATER,
    ITEM_MAGNET,
    ITEM_MIRACLE_SEED,
    ITEM_NEVER_MELT_ICE,
    ITEM_BLACK_BELT,
    ITEM_POISON_BARB,
    ITEM_SOFT_SAND,
    ITEM_SHARP_BEAK,
    ITEM_TWISTED_SPOON,
    ITEM_SILVER_POWDER,
    ITEM_DRAGON_FANG,
    ITEM_BLACK_GLASSES,
    ITEM_SNOWBALL,
    ITEM_HARD_STONE,
    ITEM_SPELL_TAG,
    ITEM_METAL_COAT,
    ITEM_EXP_SHARE,
    ITEM_NONE
};

static const u16 sNettuxBattleInventory_TwoBadges[] = {
    ITEM_SILK_SCARF,
    ITEM_CHARCOAL,
    ITEM_MYSTIC_WATER,
    ITEM_MAGNET,
    ITEM_MIRACLE_SEED,
    ITEM_NEVER_MELT_ICE,
    ITEM_BLACK_BELT,
    ITEM_POISON_BARB,
    ITEM_SOFT_SAND,
    ITEM_SHARP_BEAK,
    ITEM_TWISTED_SPOON,
    ITEM_SILVER_POWDER,
    ITEM_DRAGON_FANG,
    ITEM_BLACK_GLASSES,
    ITEM_SNOWBALL,
    ITEM_HARD_STONE,
    ITEM_SPELL_TAG,
    ITEM_METAL_COAT,
    ITEM_EXP_SHARE,
    ITEM_NONE
};

static const u16 sNettuxBattleInventory_ThreeBadges[] = {
    ITEM_SILK_SCARF,
    ITEM_CHARCOAL,
    ITEM_MYSTIC_WATER,
    ITEM_MAGNET,
    ITEM_MIRACLE_SEED,
    ITEM_NEVER_MELT_ICE,
    ITEM_BLACK_BELT,
    ITEM_POISON_BARB,
    ITEM_SOFT_SAND,
    ITEM_SHARP_BEAK,
    ITEM_TWISTED_SPOON,
    ITEM_SILVER_POWDER,
    ITEM_DRAGON_FANG,
    ITEM_BLACK_GLASSES,
    ITEM_SNOWBALL,
    ITEM_HARD_STONE,
    ITEM_SPELL_TAG,
    ITEM_METAL_COAT,
    ITEM_EXP_SHARE,
    ITEM_NONE
};

static const u16 sNettuxBattleInventory_FourBadges[] = {
    ITEM_SILK_SCARF,
    ITEM_CHARCOAL,
    ITEM_MYSTIC_WATER,
    ITEM_MAGNET,
    ITEM_MIRACLE_SEED,
    ITEM_NEVER_MELT_ICE,
    ITEM_BLACK_BELT,
    ITEM_POISON_BARB,
    ITEM_SOFT_SAND,
    ITEM_SHARP_BEAK,
    ITEM_TWISTED_SPOON,
    ITEM_SILVER_POWDER,
    ITEM_DRAGON_FANG,
    ITEM_BLACK_GLASSES,
    ITEM_SNOWBALL,
    ITEM_HARD_STONE,
    ITEM_SPELL_TAG,
    ITEM_METAL_COAT,
    ITEM_EXP_SHARE,
    ITEM_NONE
};

static const u16 sNettuxBattleInventory_FiveBadges[] = {
    ITEM_SILK_SCARF,
    ITEM_CHARCOAL,
    ITEM_MYSTIC_WATER,
    ITEM_MAGNET,
    ITEM_MIRACLE_SEED,
    ITEM_NEVER_MELT_ICE,
    ITEM_BLACK_BELT,
    ITEM_POISON_BARB,
    ITEM_SOFT_SAND,
    ITEM_SHARP_BEAK,
    ITEM_TWISTED_SPOON,
    ITEM_SILVER_POWDER,
    ITEM_DRAGON_FANG,
    ITEM_BLACK_GLASSES,
    ITEM_SNOWBALL,
    ITEM_HARD_STONE,
    ITEM_SPELL_TAG,
    ITEM_METAL_COAT,
    ITEM_EXP_SHARE,
    ITEM_NONE
};

static const u16 sNettuxBattleInventory_SixBadges[] = {
    ITEM_SILK_SCARF,
    ITEM_CHARCOAL,
    ITEM_MYSTIC_WATER,
    ITEM_MAGNET,
    ITEM_MIRACLE_SEED,
    ITEM_NEVER_MELT_ICE,
    ITEM_BLACK_BELT,
    ITEM_POISON_BARB,
    ITEM_SOFT_SAND,
    ITEM_SHARP_BEAK,
    ITEM_TWISTED_SPOON,
    ITEM_SILVER_POWDER,
    ITEM_DRAGON_FANG,
    ITEM_BLACK_GLASSES,
    ITEM_SNOWBALL,
    ITEM_HARD_STONE,
    ITEM_SPELL_TAG,
    ITEM_METAL_COAT,
    ITEM_EXP_SHARE,
    ITEM_NONE
};

static const u16 sNettuxBattleInventory_SevenBadges[] = {
    ITEM_SILK_SCARF,
    ITEM_CHARCOAL,
    ITEM_MYSTIC_WATER,
    ITEM_MAGNET,
    ITEM_MIRACLE_SEED,
    ITEM_NEVER_MELT_ICE,
    ITEM_BLACK_BELT,
    ITEM_POISON_BARB,
    ITEM_SOFT_SAND,
    ITEM_SHARP_BEAK,
    ITEM_TWISTED_SPOON,
    ITEM_SILVER_POWDER,
    ITEM_DRAGON_FANG,
    ITEM_BLACK_GLASSES,
    ITEM_SNOWBALL,
    ITEM_HARD_STONE,
    ITEM_SPELL_TAG,
    ITEM_METAL_COAT,
    ITEM_EXP_SHARE,
    ITEM_NONE
};

static const u16 sNettuxBattleInventory_EightBadges[] = {
    ITEM_SILK_SCARF,
    ITEM_CHARCOAL,
    ITEM_MYSTIC_WATER,
    ITEM_MAGNET,
    ITEM_MIRACLE_SEED,
    ITEM_NEVER_MELT_ICE,
    ITEM_BLACK_BELT,
    ITEM_POISON_BARB,
    ITEM_SOFT_SAND,
    ITEM_SHARP_BEAK,
    ITEM_TWISTED_SPOON,
    ITEM_SILVER_POWDER,
    ITEM_DRAGON_FANG,
    ITEM_BLACK_GLASSES,
    ITEM_SNOWBALL,
    ITEM_HARD_STONE,
    ITEM_SPELL_TAG,
    ITEM_METAL_COAT,
    ITEM_EXP_SHARE,
    ITEM_NONE
};

static const u16 *const sNettuxBattleInventories[] =
{
    sNettuxBattleInventory_ZeroBadges,
    sNettuxBattleInventory_OneBadge,
    sNettuxBattleInventory_TwoBadges,
    sNettuxBattleInventory_ThreeBadges,
    sNettuxBattleInventory_FourBadges,
    sNettuxBattleInventory_FiveBadges,
    sNettuxBattleInventory_SixBadges,
    sNettuxBattleInventory_SevenBadges,
    sNettuxBattleInventory_EightBadges
};

static const u16 sNettuxTMInventory_ZeroBadges[] = {
    ITEM_TM_SUNNY_DAY,
    ITEM_TM_LIGHT_SCREEN,
    ITEM_TM_PROTECT,
    ITEM_TM_RAIN_DANCE,
    ITEM_TM_REFLECT,
    ITEM_NONE
};

static const u16 sNettuxTMInventory_OneBadge[] = {
    ITEM_TM_SUNNY_DAY,
    ITEM_TM_LIGHT_SCREEN,
    ITEM_TM_PROTECT,
    ITEM_TM_RAIN_DANCE,
    ITEM_TM_REFLECT,
    ITEM_NONE
};

static const u16 sNettuxTMInventory_TwoBadges[] = {
    ITEM_TM_SUNNY_DAY,
    ITEM_TM_LIGHT_SCREEN,
    ITEM_TM_PROTECT,
    ITEM_TM_RAIN_DANCE,
    ITEM_TM_REFLECT,
    ITEM_NONE
};

static const u16 sNettuxTMInventory_ThreeBadges[] = {
    ITEM_TM_SUNNY_DAY,
    ITEM_TM_LIGHT_SCREEN,
    ITEM_TM_PROTECT,
    ITEM_TM_RAIN_DANCE,
    ITEM_TM_REFLECT,
    ITEM_NONE
};

static const u16 sNettuxTMInventory_FourBadges[] = {
    ITEM_TM_SUNNY_DAY,
    ITEM_TM_LIGHT_SCREEN,
    ITEM_TM_PROTECT,
    ITEM_TM_RAIN_DANCE,
    ITEM_TM_REFLECT,
    ITEM_NONE
};

static const u16 sNettuxTMInventory_FiveBadges[] = {
    ITEM_TM_SUNNY_DAY,
    ITEM_TM_LIGHT_SCREEN,
    ITEM_TM_PROTECT,
    ITEM_TM_RAIN_DANCE,
    ITEM_TM_REFLECT,
    ITEM_NONE
};

static const u16 sNettuxTMInventory_SixBadges[] = {
    ITEM_TM_SUNNY_DAY,
    ITEM_TM_LIGHT_SCREEN,
    ITEM_TM_PROTECT,
    ITEM_TM_RAIN_DANCE,
    ITEM_TM_REFLECT,
    ITEM_NONE
};

static const u16 sNettuxTMInventory_SevenBadges[] = {
    ITEM_TM_SUNNY_DAY,
    ITEM_TM_LIGHT_SCREEN,
    ITEM_TM_PROTECT,
    ITEM_TM_RAIN_DANCE,
    ITEM_TM_REFLECT,
    ITEM_NONE
};

static const u16 sNettuxTMInventory_EightBadges[] = {
    ITEM_TM_SUNNY_DAY,
    ITEM_TM_LIGHT_SCREEN,
    ITEM_TM_PROTECT,
    ITEM_TM_RAIN_DANCE,
    ITEM_TM_REFLECT,
    ITEM_NONE
};

static const u16 *const sNettuxTMInventories[] =
{
    sNettuxTMInventory_ZeroBadges,
    sNettuxTMInventory_OneBadge,
    sNettuxTMInventory_TwoBadges,
    sNettuxTMInventory_ThreeBadges,
    sNettuxTMInventory_FourBadges,
    sNettuxTMInventory_FiveBadges,
    sNettuxTMInventory_SixBadges,
    sNettuxTMInventory_SevenBadges,
    sNettuxTMInventory_EightBadges
};

static const u16 sNettuxEvolveInventory_ZeroBadges[] = {
    ITEM_FIRE_STONE,
    ITEM_WATER_STONE,
    ITEM_THUNDER_STONE,
    ITEM_LEAF_STONE,
    ITEM_ICE_STONE,
    ITEM_SUN_STONE,
    ITEM_MOON_STONE,
    ITEM_SHINY_STONE,
    ITEM_DUSK_STONE,
    ITEM_DAWN_STONE,
    ITEM_NONE
};

static const u16 sNettuxEvolveInventory_OneBadge[] = {
    ITEM_FIRE_STONE,
    ITEM_WATER_STONE,
    ITEM_THUNDER_STONE,
    ITEM_LEAF_STONE,
    ITEM_ICE_STONE,
    ITEM_SUN_STONE,
    ITEM_MOON_STONE,
    ITEM_SHINY_STONE,
    ITEM_DUSK_STONE,
    ITEM_DAWN_STONE,
    ITEM_NONE
};

static const u16 sNettuxEvolveInventory_TwoBadges[] = {
    ITEM_FIRE_STONE,
    ITEM_WATER_STONE,
    ITEM_THUNDER_STONE,
    ITEM_LEAF_STONE,
    ITEM_ICE_STONE,
    ITEM_SUN_STONE,
    ITEM_MOON_STONE,
    ITEM_SHINY_STONE,
    ITEM_DUSK_STONE,
    ITEM_DAWN_STONE,
    ITEM_NONE
};

static const u16 sNettuxEvolveInventory_ThreeBadges[] = {
    ITEM_FIRE_STONE,
    ITEM_WATER_STONE,
    ITEM_THUNDER_STONE,
    ITEM_LEAF_STONE,
    ITEM_ICE_STONE,
    ITEM_SUN_STONE,
    ITEM_MOON_STONE,
    ITEM_SHINY_STONE,
    ITEM_DUSK_STONE,
    ITEM_DAWN_STONE,
    ITEM_NONE
};

static const u16 sNettuxEvolveInventory_FourBadges[] = {
    ITEM_FIRE_STONE,
    ITEM_WATER_STONE,
    ITEM_THUNDER_STONE,
    ITEM_LEAF_STONE,
    ITEM_ICE_STONE,
    ITEM_SUN_STONE,
    ITEM_MOON_STONE,
    ITEM_SHINY_STONE,
    ITEM_DUSK_STONE,
    ITEM_DAWN_STONE,
    ITEM_NONE
};

static const u16 sNettuxEvolveInventory_FiveBadges[] = {
    ITEM_FIRE_STONE,
    ITEM_WATER_STONE,
    ITEM_THUNDER_STONE,
    ITEM_LEAF_STONE,
    ITEM_ICE_STONE,
    ITEM_SUN_STONE,
    ITEM_MOON_STONE,
    ITEM_SHINY_STONE,
    ITEM_DUSK_STONE,
    ITEM_DAWN_STONE,
    ITEM_NONE
};

static const u16 sNettuxEvolveInventory_SixBadges[] = {
    ITEM_FIRE_STONE,
    ITEM_WATER_STONE,
    ITEM_THUNDER_STONE,
    ITEM_LEAF_STONE,
    ITEM_ICE_STONE,
    ITEM_SUN_STONE,
    ITEM_MOON_STONE,
    ITEM_SHINY_STONE,
    ITEM_DUSK_STONE,
    ITEM_DAWN_STONE,
    ITEM_NONE
};

static const u16 sNettuxEvolveInventory_SevenBadges[] = {
    ITEM_FIRE_STONE,
    ITEM_WATER_STONE,
    ITEM_THUNDER_STONE,
    ITEM_LEAF_STONE,
    ITEM_ICE_STONE,
    ITEM_SUN_STONE,
    ITEM_MOON_STONE,
    ITEM_SHINY_STONE,
    ITEM_DUSK_STONE,
    ITEM_DAWN_STONE,
    ITEM_NONE
};

static const u16 sNettuxEvolveInventory_EightBadges[] = {
    ITEM_FIRE_STONE,
    ITEM_WATER_STONE,
    ITEM_THUNDER_STONE,
    ITEM_LEAF_STONE,
    ITEM_ICE_STONE,
    ITEM_SUN_STONE,
    ITEM_MOON_STONE,
    ITEM_SHINY_STONE,
    ITEM_DUSK_STONE,
    ITEM_DAWN_STONE,
    ITEM_NONE
};

static const u16 *const sNettuxEvolveInventories[] =
{
    sNettuxEvolveInventory_ZeroBadges,
    sNettuxEvolveInventory_OneBadge,
    sNettuxEvolveInventory_TwoBadges,
    sNettuxEvolveInventory_ThreeBadges,
    sNettuxEvolveInventory_FourBadges,
    sNettuxEvolveInventory_FiveBadges,
    sNettuxEvolveInventory_SixBadges,
    sNettuxEvolveInventory_SevenBadges,
    sNettuxEvolveInventory_EightBadges
};

static const u16 sNettuxTrainingInventory_ZeroBadges[] = {
    ITEM_EXP_CANDY_XS,
    ITEM_EXP_CANDY_S,
    ITEM_RARE_CANDY,
    ITEM_HP_UP,
    ITEM_PROTEIN,
    ITEM_IRON,
    ITEM_CALCIUM,
    ITEM_ZINC,
    ITEM_CARBOS,
    ITEM_LONELY_MINT,
    ITEM_ADAMANT_MINT,
    ITEM_NAUGHTY_MINT,
    ITEM_BRAVE_MINT,
    ITEM_BOLD_MINT,
    ITEM_IMPISH_MINT,
    ITEM_LAX_MINT,
    ITEM_RELAXED_MINT,
    ITEM_MODEST_MINT,
    ITEM_MILD_MINT,
    ITEM_RASH_MINT,
    ITEM_QUIET_MINT,
    ITEM_CALM_MINT,
    ITEM_GENTLE_MINT,
    ITEM_CAREFUL_MINT,
    ITEM_SASSY_MINT,
    ITEM_TIMID_MINT,
    ITEM_HASTY_MINT,
    ITEM_JOLLY_MINT,
    ITEM_NAIVE_MINT,
    ITEM_SERIOUS_MINT,
    ITEM_NONE
};

static const u16 sNettuxTrainingInventory_OneBadge[] = {
    ITEM_EXP_CANDY_XS,
    ITEM_RARE_CANDY,
    ITEM_HP_UP,
    ITEM_PROTEIN,
    ITEM_IRON,
    ITEM_CALCIUM,
    ITEM_ZINC,
    ITEM_CARBOS,
    ITEM_LONELY_MINT,
    ITEM_ADAMANT_MINT,
    ITEM_NAUGHTY_MINT,
    ITEM_BRAVE_MINT,
    ITEM_BOLD_MINT,
    ITEM_IMPISH_MINT,
    ITEM_LAX_MINT,
    ITEM_RELAXED_MINT,
    ITEM_MODEST_MINT,
    ITEM_MILD_MINT,
    ITEM_RASH_MINT,
    ITEM_QUIET_MINT,
    ITEM_CALM_MINT,
    ITEM_GENTLE_MINT,
    ITEM_CAREFUL_MINT,
    ITEM_SASSY_MINT,
    ITEM_TIMID_MINT,
    ITEM_HASTY_MINT,
    ITEM_JOLLY_MINT,
    ITEM_NAIVE_MINT,
    ITEM_SERIOUS_MINT,
    ITEM_NONE
};

static const u16 sNettuxTrainingInventory_TwoBadges[] = {
    ITEM_EXP_CANDY_XS,
    ITEM_RARE_CANDY,
    ITEM_HP_UP,
    ITEM_PROTEIN,
    ITEM_IRON,
    ITEM_CALCIUM,
    ITEM_ZINC,
    ITEM_CARBOS,
    ITEM_LONELY_MINT,
    ITEM_ADAMANT_MINT,
    ITEM_NAUGHTY_MINT,
    ITEM_BRAVE_MINT,
    ITEM_BOLD_MINT,
    ITEM_IMPISH_MINT,
    ITEM_LAX_MINT,
    ITEM_RELAXED_MINT,
    ITEM_MODEST_MINT,
    ITEM_MILD_MINT,
    ITEM_RASH_MINT,
    ITEM_QUIET_MINT,
    ITEM_CALM_MINT,
    ITEM_GENTLE_MINT,
    ITEM_CAREFUL_MINT,
    ITEM_SASSY_MINT,
    ITEM_TIMID_MINT,
    ITEM_HASTY_MINT,
    ITEM_JOLLY_MINT,
    ITEM_NAIVE_MINT,
    ITEM_SERIOUS_MINT,
    ITEM_NONE
};

static const u16 sNettuxTrainingInventory_ThreeBadges[] = {
    ITEM_EXP_CANDY_XS,
    ITEM_RARE_CANDY,
    ITEM_HP_UP,
    ITEM_PROTEIN,
    ITEM_IRON,
    ITEM_CALCIUM,
    ITEM_ZINC,
    ITEM_CARBOS,
    ITEM_LONELY_MINT,
    ITEM_ADAMANT_MINT,
    ITEM_NAUGHTY_MINT,
    ITEM_BRAVE_MINT,
    ITEM_BOLD_MINT,
    ITEM_IMPISH_MINT,
    ITEM_LAX_MINT,
    ITEM_RELAXED_MINT,
    ITEM_MODEST_MINT,
    ITEM_MILD_MINT,
    ITEM_RASH_MINT,
    ITEM_QUIET_MINT,
    ITEM_CALM_MINT,
    ITEM_GENTLE_MINT,
    ITEM_CAREFUL_MINT,
    ITEM_SASSY_MINT,
    ITEM_TIMID_MINT,
    ITEM_HASTY_MINT,
    ITEM_JOLLY_MINT,
    ITEM_NAIVE_MINT,
    ITEM_SERIOUS_MINT,
    ITEM_NONE
};

static const u16 sNettuxTrainingInventory_FourBadges[] = {
    ITEM_EXP_CANDY_XS,
    ITEM_RARE_CANDY,
    ITEM_HP_UP,
    ITEM_PROTEIN,
    ITEM_IRON,
    ITEM_CALCIUM,
    ITEM_ZINC,
    ITEM_CARBOS,
    ITEM_LONELY_MINT,
    ITEM_ADAMANT_MINT,
    ITEM_NAUGHTY_MINT,
    ITEM_BRAVE_MINT,
    ITEM_BOLD_MINT,
    ITEM_IMPISH_MINT,
    ITEM_LAX_MINT,
    ITEM_RELAXED_MINT,
    ITEM_MODEST_MINT,
    ITEM_MILD_MINT,
    ITEM_RASH_MINT,
    ITEM_QUIET_MINT,
    ITEM_CALM_MINT,
    ITEM_GENTLE_MINT,
    ITEM_CAREFUL_MINT,
    ITEM_SASSY_MINT,
    ITEM_TIMID_MINT,
    ITEM_HASTY_MINT,
    ITEM_JOLLY_MINT,
    ITEM_NAIVE_MINT,
    ITEM_SERIOUS_MINT,
    ITEM_NONE
};

static const u16 sNettuxTrainingInventory_FiveBadges[] = {
    ITEM_EXP_CANDY_XS,
    ITEM_RARE_CANDY,
    ITEM_HP_UP,
    ITEM_PROTEIN,
    ITEM_IRON,
    ITEM_CALCIUM,
    ITEM_ZINC,
    ITEM_CARBOS,
    ITEM_LONELY_MINT,
    ITEM_ADAMANT_MINT,
    ITEM_NAUGHTY_MINT,
    ITEM_BRAVE_MINT,
    ITEM_BOLD_MINT,
    ITEM_IMPISH_MINT,
    ITEM_LAX_MINT,
    ITEM_RELAXED_MINT,
    ITEM_MODEST_MINT,
    ITEM_MILD_MINT,
    ITEM_RASH_MINT,
    ITEM_QUIET_MINT,
    ITEM_CALM_MINT,
    ITEM_GENTLE_MINT,
    ITEM_CAREFUL_MINT,
    ITEM_SASSY_MINT,
    ITEM_TIMID_MINT,
    ITEM_HASTY_MINT,
    ITEM_JOLLY_MINT,
    ITEM_NAIVE_MINT,
    ITEM_SERIOUS_MINT,
    ITEM_NONE
};

static const u16 sNettuxTrainingInventory_SixBadges[] = {
    ITEM_EXP_CANDY_XS,
    ITEM_RARE_CANDY,
    ITEM_HP_UP,
    ITEM_PROTEIN,
    ITEM_IRON,
    ITEM_CALCIUM,
    ITEM_ZINC,
    ITEM_CARBOS,
    ITEM_LONELY_MINT,
    ITEM_ADAMANT_MINT,
    ITEM_NAUGHTY_MINT,
    ITEM_BRAVE_MINT,
    ITEM_BOLD_MINT,
    ITEM_IMPISH_MINT,
    ITEM_LAX_MINT,
    ITEM_RELAXED_MINT,
    ITEM_MODEST_MINT,
    ITEM_MILD_MINT,
    ITEM_RASH_MINT,
    ITEM_QUIET_MINT,
    ITEM_CALM_MINT,
    ITEM_GENTLE_MINT,
    ITEM_CAREFUL_MINT,
    ITEM_SASSY_MINT,
    ITEM_TIMID_MINT,
    ITEM_HASTY_MINT,
    ITEM_JOLLY_MINT,
    ITEM_NAIVE_MINT,
    ITEM_SERIOUS_MINT,
    ITEM_NONE
};

static const u16 sNettuxTrainingInventory_SevenBadges[] = {
    ITEM_EXP_CANDY_XS,
    ITEM_RARE_CANDY,
    ITEM_HP_UP,
    ITEM_PROTEIN,
    ITEM_IRON,
    ITEM_CALCIUM,
    ITEM_ZINC,
    ITEM_CARBOS,
    ITEM_LONELY_MINT,
    ITEM_ADAMANT_MINT,
    ITEM_NAUGHTY_MINT,
    ITEM_BRAVE_MINT,
    ITEM_BOLD_MINT,
    ITEM_IMPISH_MINT,
    ITEM_LAX_MINT,
    ITEM_RELAXED_MINT,
    ITEM_MODEST_MINT,
    ITEM_MILD_MINT,
    ITEM_RASH_MINT,
    ITEM_QUIET_MINT,
    ITEM_CALM_MINT,
    ITEM_GENTLE_MINT,
    ITEM_CAREFUL_MINT,
    ITEM_SASSY_MINT,
    ITEM_TIMID_MINT,
    ITEM_HASTY_MINT,
    ITEM_JOLLY_MINT,
    ITEM_NAIVE_MINT,
    ITEM_SERIOUS_MINT,
    ITEM_NONE
};

static const u16 sNettuxTrainingInventory_EightBadges[] = {
    ITEM_EXP_CANDY_XS,
    ITEM_RARE_CANDY,
    ITEM_HP_UP,
    ITEM_PROTEIN,
    ITEM_IRON,
    ITEM_CALCIUM,
    ITEM_ZINC,
    ITEM_CARBOS,
    ITEM_LONELY_MINT,
    ITEM_ADAMANT_MINT,
    ITEM_NAUGHTY_MINT,
    ITEM_BRAVE_MINT,
    ITEM_BOLD_MINT,
    ITEM_IMPISH_MINT,
    ITEM_LAX_MINT,
    ITEM_RELAXED_MINT,
    ITEM_MODEST_MINT,
    ITEM_MILD_MINT,
    ITEM_RASH_MINT,
    ITEM_QUIET_MINT,
    ITEM_CALM_MINT,
    ITEM_GENTLE_MINT,
    ITEM_CAREFUL_MINT,
    ITEM_SASSY_MINT,
    ITEM_TIMID_MINT,
    ITEM_HASTY_MINT,
    ITEM_JOLLY_MINT,
    ITEM_NAIVE_MINT,
    ITEM_SERIOUS_MINT,
    ITEM_NONE
};

static const u16 *const sNettuxTrainingInventories[] =
{
    sNettuxTrainingInventory_ZeroBadges,
    sNettuxTrainingInventory_OneBadge,
    sNettuxTrainingInventory_TwoBadges,
    sNettuxTrainingInventory_ThreeBadges,
    sNettuxTrainingInventory_FourBadges,
    sNettuxTrainingInventory_FiveBadges,
    sNettuxTrainingInventory_SixBadges,
    sNettuxTrainingInventory_SevenBadges,
    sNettuxTrainingInventory_EightBadges
};

static const u16 sNettuxMartInventory_ZeroBadges[] = {
    ITEM_POKE_BALL,
    ITEM_POTION,
    ITEM_ANTIDOTE,
    ITEM_BURN_HEAL,
    ITEM_ICE_HEAL,
    ITEM_AWAKENING,
    ITEM_PARALYZE_HEAL,
    ITEM_REVIVE,
    ITEM_NONE
};

static const u16 sNettuxMartInventory_OneBadge[] = {
    ITEM_POKE_BALL,
    ITEM_GREAT_BALL,
    ITEM_POTION,
    ITEM_SUPER_POTION,
    ITEM_ANTIDOTE,
    ITEM_BURN_HEAL,
    ITEM_ICE_HEAL,
    ITEM_AWAKENING,
    ITEM_PARALYZE_HEAL,
    ITEM_REVIVE,
    ITEM_NONE
};

static const u16 sNettuxMartInventory_TwoBadges[] = {
    ITEM_POKE_BALL,
    ITEM_GREAT_BALL,
    ITEM_POTION,
    ITEM_SUPER_POTION,
    ITEM_ANTIDOTE,
    ITEM_BURN_HEAL,
    ITEM_ICE_HEAL,
    ITEM_AWAKENING,
    ITEM_PARALYZE_HEAL,
    ITEM_REVIVE,
    ITEM_POKE_DOLL,
    ITEM_NONE
};

static const u16 sNettuxMartInventory_ThreeBadges[] = {
    ITEM_POKE_BALL,
    ITEM_GREAT_BALL,
    ITEM_POTION,
    ITEM_SUPER_POTION,
    ITEM_ANTIDOTE,
    ITEM_BURN_HEAL,
    ITEM_ICE_HEAL,
    ITEM_AWAKENING,
    ITEM_PARALYZE_HEAL,
    ITEM_REVIVE,
    ITEM_REPEL,
    ITEM_POKE_DOLL,
    ITEM_NONE
};

static const u16 sNettuxMartInventory_FourBadges[] = {
    ITEM_POKE_BALL,
    ITEM_GREAT_BALL,
    ITEM_POTION,
    ITEM_SUPER_POTION,
    ITEM_HYPER_POTION,
    ITEM_ANTIDOTE,
    ITEM_BURN_HEAL,
    ITEM_ICE_HEAL,
    ITEM_AWAKENING,
    ITEM_PARALYZE_HEAL,
    ITEM_REVIVE,
    ITEM_REPEL,
    ITEM_POKE_DOLL,
    ITEM_NONE
};

static const u16 sNettuxMartInventory_FiveBadges[] = {
    ITEM_POKE_BALL,
    ITEM_GREAT_BALL,
    ITEM_ULTRA_BALL,
    ITEM_POTION,
    ITEM_SUPER_POTION,
    ITEM_HYPER_POTION,
    ITEM_ANTIDOTE,
    ITEM_BURN_HEAL,
    ITEM_ICE_HEAL,
    ITEM_AWAKENING,
    ITEM_PARALYZE_HEAL,
    ITEM_REVIVE,
    ITEM_REPEL,
    ITEM_SUPER_REPEL,
    ITEM_POKE_DOLL,
    ITEM_NONE
};

static const u16 sNettuxMartInventory_SixBadges[] = {
    ITEM_POKE_BALL,
    ITEM_GREAT_BALL,
    ITEM_ULTRA_BALL,
    ITEM_POTION,
    ITEM_SUPER_POTION,
    ITEM_HYPER_POTION,
    ITEM_ANTIDOTE,
    ITEM_BURN_HEAL,
    ITEM_ICE_HEAL,
    ITEM_AWAKENING,
    ITEM_PARALYZE_HEAL,
    ITEM_FULL_HEAL,
    ITEM_REVIVE,
    ITEM_REPEL,
    ITEM_SUPER_REPEL,
    ITEM_POKE_DOLL,
    ITEM_NONE
};

static const u16 sNettuxMartInventory_SevenBadges[] = {
    ITEM_POKE_BALL,
    ITEM_GREAT_BALL,
    ITEM_ULTRA_BALL,
    ITEM_POTION,
    ITEM_SUPER_POTION,
    ITEM_HYPER_POTION,
    ITEM_MAX_POTION,
    ITEM_ANTIDOTE,
    ITEM_BURN_HEAL,
    ITEM_ICE_HEAL,
    ITEM_AWAKENING,
    ITEM_PARALYZE_HEAL,
    ITEM_FULL_HEAL,
    ITEM_REVIVE,
    ITEM_REPEL,
    ITEM_SUPER_REPEL,
    ITEM_MAX_REPEL,
    ITEM_POKE_DOLL,
    ITEM_NONE
};

static const u16 sNettuxMartInventory_EightBadges[] = {
    ITEM_POKE_BALL,
    ITEM_GREAT_BALL,
    ITEM_ULTRA_BALL,
    ITEM_POTION,
    ITEM_SUPER_POTION,
    ITEM_HYPER_POTION,
    ITEM_MAX_POTION,
    ITEM_FULL_RESTORE,
    ITEM_ANTIDOTE,
    ITEM_BURN_HEAL,
    ITEM_ICE_HEAL,
    ITEM_AWAKENING,
    ITEM_PARALYZE_HEAL,
    ITEM_FULL_HEAL,
    ITEM_REVIVE,
    ITEM_REPEL,
    ITEM_SUPER_REPEL,
    ITEM_MAX_REPEL,
    ITEM_MAX_ETHER,
    ITEM_POKE_DOLL,
    ITEM_NONE
};

static const u16 *const sNettuxMartInventories[] =
{
    sNettuxMartInventory_ZeroBadges,
    sNettuxMartInventory_OneBadge,
    sNettuxMartInventory_TwoBadges,
    sNettuxMartInventory_ThreeBadges,
    sNettuxMartInventory_FourBadges,
    sNettuxMartInventory_FiveBadges,
    sNettuxMartInventory_SixBadges,
    sNettuxMartInventory_SevenBadges,
    sNettuxMartInventory_EightBadges
};
// nettux

// Functions
static u8 CreateShopMenu(u8 martType)
{
    sShopData.martType = GetMartTypeFromItemList(martType);
    sShopData.selectedRow = 0;
    if (ContextNpcGetTextColor() == NPC_TEXT_COLOR_MALE)
        sShopData.fontId = FONT_MALE;
    else
        sShopData.fontId = FONT_FEMALE;

    sShopMenuWindowId = AddWindow(&sShopMenuWindowTemplate);
    SetStandardWindowBorderStyle(sShopMenuWindowId, 0);
    PrintMenuActionTextsAtPos(sShopMenuWindowId, FONT_NORMAL, GetMenuCursorDimensionByFont(FONT_NORMAL, 0), 2, 16, 3, sShopMenuActions_BuySellQuit);
    InitMenuNormal(sShopMenuWindowId, FONT_NORMAL, 0, 2, 16, 3, 0);
    PutWindowTilemap(sShopMenuWindowId);
    CopyWindowToVram(sShopMenuWindowId, COPYWIN_MAP);
    return CreateTask(Task_ShopMenu, 8);
}

static u8 GetMartTypeFromItemList(u32 martType)
{
    if (martType != MART_TYPE_REGULAR)
        return martType;
    return MART_TYPE_REGULAR;
}

static u8 GetNumberOfBadges(void)
{
    u16 badgeFlag;
    u8 count = 0;

    for (badgeFlag = FLAG_BADGE01_GET; badgeFlag < FLAG_BADGE01_GET + NUM_BADGES; badgeFlag++)
    {
        if (FlagGet(badgeFlag))
            count++;
    }

    return count;
}

static void SetShopItemsForSale(const u16 *items)
{
    u8 badgeCount = GetNumberOfBadges();
    if (items[0] == ITEM_PREMIER_BALL) {
	// standard mart
	sShopData.itemList = sNettuxMartInventories[badgeCount];
    } else if (items[0] == ITEM_EXP_CANDY_XS) {
	// training mart
	sShopData.itemList = sNettuxTrainingInventories[badgeCount];
    } else if (items[0] == ITEM_EXP_SHARE) {
	// training mart
	sShopData.itemList = sNettuxBattleInventories[badgeCount];
    } else if (items[0] == ITEM_TM_PROTECT) {
	// training mart
	sShopData.itemList = sNettuxTMInventories[badgeCount];
    } else if (items[0] == ITEM_LINKING_CORD) {
	// training mart
	sShopData.itemList = sNettuxEvolveInventories[badgeCount];
    } else {
	// passed in item list directly
        sShopData.itemList = items;
    }

    sShopData.itemCount = 0;
    if (sShopData.itemList[0] == 0)
        return;

    while (sShopData.itemList[sShopData.itemCount])
    {
        ++sShopData.itemCount;
    }
}

static void SetShopMenuCallback(void (*callback)(void))
{
    sShopData.callback = callback;
}

static void Task_ShopMenu(u8 taskId)
{
    s8 input = Menu_ProcessInputNoWrap();

    switch (input)
    {
    case MENU_NOTHING_CHOSEN:
        break;
    case MENU_B_PRESSED:
        PlaySE(SE_SELECT);
        Task_HandleShopMenuQuit(taskId);
        break;
    default:
        sShopMenuActions_BuySellQuit[Menu_GetCursorPos()].func.void_u8(taskId);
        break;
    }
}

static void Task_HandleShopMenuBuy(u8 taskId)
{
    SetWordTaskArg(taskId, 0xE, (u32)CB2_InitBuyMenu);
    FadeScreen(FADE_TO_BLACK, 0);
    gTasks[taskId].func = Task_GoToBuyOrSellMenu;
}

static void Task_HandleShopMenuSell(u8 taskId)
{
    SetWordTaskArg(taskId, 0xE, (u32)CB2_GoToSellMenu);
    FadeScreen(FADE_TO_BLACK, 0);
    gTasks[taskId].func = Task_GoToBuyOrSellMenu;
}

static void CB2_GoToSellMenu(void)
{
    GoToBagMenu(ITEMMENULOCATION_SHOP, OPEN_BAG_LAST, CB2_ReturnToField);
    gFieldCallback = MapPostLoadHook_ReturnToShopMenu;
}

static void Task_HandleShopMenuQuit(u8 taskId)
{
    ClearShopMenuWindow();
    RecordTransactionForQuestLog();
    DestroyTask(taskId);
    if (sShopData.callback != NULL)
        sShopData.callback();
}

static void ClearShopMenuWindow(void)
{
    ClearStdWindowAndFrameToTransparent(sShopMenuWindowId, 2);
    RemoveWindow(sShopMenuWindowId);
}

static void Task_GoToBuyOrSellMenu(u8 taskId)
{
    if (gPaletteFade.active)
        return;

    SetMainCallback2((void *)GetWordTaskArg(taskId, 0xE));
    FreeAllWindowBuffers();
    DestroyTask(taskId);
}

static void MapPostLoadHook_ReturnToShopMenu(void)
{
    FadeInFromBlack();
    CreateTask(Task_ReturnToShopMenu, 8);
}

static void Task_ReturnToShopMenu(u8 taskId)
{
    if (IsWeatherNotFadingIn() != TRUE)
        return;

    DisplayItemMessageOnField(taskId, GetMartFontId(), gText_AnythingElseICanHelp, ShowShopMenuAfterExitingBuyOrSellMenu);
}

static void ShowShopMenuAfterExitingBuyOrSellMenu(u8 taskId)
{
    CreateShopMenu(sShopData.martType);
    DestroyTask(taskId);
}

static void CB2_BuyMenu(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
    DoScheduledBgTilemapCopiesToVram();
}

static void VBlankCB_BuyMenu(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_InitBuyMenu(void)
{
    u8 taskId;

    switch (gMain.state)
    {
    case 0:
        SetVBlankHBlankCallbacksToNull();
        CpuFastFill(0, (void *)OAM, 0x400);
        ScanlineEffect_Stop();
        ResetTempTileDataBuffers();
        FreeAllSpritePalettes();
        ResetPaletteFade();
        ResetSpriteData();
        ResetTasks();
        ClearScheduledBgCopiesToVram();
        ResetItemMenuIconState();
        if (!(InitShopData()) || !(BuyMenuBuildListMenuTemplate()))
            return;
        BuyMenuInitBgs();
        FillBgTilemapBufferRect_Palette0(0, 0, 0, 0, 0x20, 0x20);
        FillBgTilemapBufferRect_Palette0(1, 0, 0, 0, 0x20, 0x20);
        FillBgTilemapBufferRect_Palette0(2, 0, 0, 0, 0x20, 0x20);
        FillBgTilemapBufferRect_Palette0(3, 0, 0, 0, 0x20, 0x20);
        BuyMenuInitWindows(sShopData.martType);
        BuyMenuDecompressBgGraphics();
        gMain.state++;
        break;
    case 1:
        if (FreeTempTileDataBuffersIfPossible())
            return;
        gMain.state++;
        break;
    default:
        sShopData.selectedRow = 0;
        sShopData.scrollOffset = 0;
        BuyMenuDrawGraphics();
        BuyMenuAddScrollIndicatorArrows();
        taskId = CreateTask(Task_BuyMenu, 8);
        gTasks[taskId].tListTaskId = ListMenuInit(&gMultiuseListMenuTemplate, 0, 0);
        BlendPalettes(PALETTES_ALL, 0x10, RGB_BLACK);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0x10, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB_BuyMenu);
        SetMainCallback2(CB2_BuyMenu);
        break;
    }
}

static bool8 InitShopData(void)
{
    gShopTilemapBuffer1 = Alloc(sizeof(*gShopTilemapBuffer1));
    if (gShopTilemapBuffer1 == NULL)
    {
        BuyMenuFreeMemory();
        SetShopExitCallback();
        return FALSE;
    }

    gShopTilemapBuffer2 = Alloc(sizeof(*gShopTilemapBuffer2));
    if (gShopTilemapBuffer2 == NULL)
    {
        BuyMenuFreeMemory();
        SetShopExitCallback();
        return FALSE;
    }

    gShopTilemapBuffer3 = Alloc(sizeof(*gShopTilemapBuffer3));
    if (gShopTilemapBuffer3 == NULL)
    {
        BuyMenuFreeMemory();
        SetShopExitCallback();
        return FALSE;
    }

    gShopTilemapBuffer4 = Alloc(sizeof(*gShopTilemapBuffer4));
    if (gShopTilemapBuffer4 == NULL)
    {
        BuyMenuFreeMemory();
        SetShopExitCallback();
        return FALSE;
    }

    return TRUE;
}

static void BuyMenuInitBgs(void)
{
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sShopBuyMenuBgTemplates, NELEMS(sShopBuyMenuBgTemplates));
    SetBgTilemapBuffer(1, gShopTilemapBuffer2);
    SetBgTilemapBuffer(2, gShopTilemapBuffer4);
    SetBgTilemapBuffer(3, gShopTilemapBuffer3);
    SetGpuReg(REG_OFFSET_BG0HOFS, DISPCNT_MODE_0);
    SetGpuReg(REG_OFFSET_BG0VOFS, DISPCNT_MODE_0);
    SetGpuReg(REG_OFFSET_BG1HOFS, DISPCNT_MODE_0);
    SetGpuReg(REG_OFFSET_BG1VOFS, DISPCNT_MODE_0);
    SetGpuReg(REG_OFFSET_BG2HOFS, DISPCNT_MODE_0);
    SetGpuReg(REG_OFFSET_BG2VOFS, DISPCNT_MODE_0);
    SetGpuReg(REG_OFFSET_BG3HOFS, DISPCNT_MODE_0);
    SetGpuReg(REG_OFFSET_BG3VOFS, DISPCNT_MODE_0);
    SetGpuReg(REG_OFFSET_BLDCNT, DISPCNT_MODE_0);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
    ShowBg(0);
    ShowBg(1);
    ShowBg(2);
    ShowBg(3);
}

static void BuyMenuDecompressBgGraphics(void)
{
    u16 *pal;

    DecompressAndCopyTileDataToVram(1, gBuyMenuFrame_Gfx, 0x480, 0x3DC, 0);
    if ((sShopData.martType) != MART_TYPE_TMHM)
        LZDecompressWram(gBuyMenuFrame_Tilemap, gShopTilemapBuffer1);
    else
        LZDecompressWram(gBuyMenuFrame_TmHmTilemap, gShopTilemapBuffer1);

    pal = Alloc(2 * PLTT_SIZE_4BPP);
    LZDecompressWram(gBuyMenuFrame_Pal, pal);
    LoadPalette(&pal[0 * 16], BG_PLTT_ID(11), PLTT_SIZE_4BPP);
    LoadPalette(&pal[1 * 16], BG_PLTT_ID(6), PLTT_SIZE_4BPP);
    Free(pal);
}

static void BuyMenuInitWindows(bool32 isSellingTM)
{
    if (isSellingTM != TRUE)
        InitWindows(sShopBuyMenuWindowTemplatesNormal);
    else
        InitWindows(sShopBuyMenuWindowTemplatesTM);
    DeactivateAllTextPrinters();
    LoadUserWindowBorderGfx(0, 0x1, BG_PLTT_ID(13));
    LoadMessageBoxGfx(0, 0x13, BG_PLTT_ID(14));
    LoadStdWindowGfx(0, 0xA, BG_PLTT_ID(15));
    PutWindowTilemap(0);
    PutWindowTilemap(4);
    PutWindowTilemap(5);
    if (isSellingTM == TRUE)
        PutWindowTilemap(6);
}

static void BuyMenuPrint(u8 windowId, u8 font, const u8 *text, u8 x, u8 y, u8 letterSpacing, u8 lineSpacing, u8 speed, u8 color)
{
    AddTextPrinterParameterized4(windowId, font, x, y, letterSpacing, lineSpacing, sShopBuyMenuTextColors[color], speed, text);
}

static void BuyMenuDisplayMessage(u8 taskId, const u8 *text, TaskFunc callback)
{
    DisplayMessageAndContinueTask(taskId, 2, 0x13, 0xE, GetMartFontId(), GetPlayerTextSpeedDelay(), text, callback);
    ScheduleBgCopyTilemapToVram(0);
}

static void BuyMenuDrawGraphics(void)
{
    BuyMenuDrawMapView();
    BuyMenuCopyTilemapData();
    PrintMoneyAmountInMoneyBoxWithBorder(0, 0xA, 0xF, GetMoney(&gSaveBlock1Ptr->money));
    ScheduleBgCopyTilemapToVram(0);
    ScheduleBgCopyTilemapToVram(1);
    ScheduleBgCopyTilemapToVram(2);
    ScheduleBgCopyTilemapToVram(3);
}

static void BuyMenuDrawMapView(void)
{
    BuyMenuCollectObjectEventData();
    BuyMenuDrawObjectEvents();
    BuyMenuDrawMapBg();
}

static void BuyMenuQuantityBoxNormalBorder(u8 windowId, bool8 copyToVram)
{
    DrawStdFrameWithCustomTileAndPalette(windowId, copyToVram, 0x1, 13);
}

static void BuyMenuQuantityBoxThinBorder(u8 windowId, bool8 copyToVram)
{
    DrawStdFrameWithCustomTileAndPalette(windowId, copyToVram, 0xA, 15);
}

static void BuyMenuConfirmPurchase(u8 taskId, const struct YesNoFuncTable *yesNo)
{
    CreateYesNoMenuWithCallbacks(taskId, &sShopBuyMenuYesNoWindowTemplate, FONT_NORMAL, 0, 2, 1, 13, yesNo);
}

static void RecolorItemDescriptionBox(bool32 a0)
{
    u8 paletteNum;

    if (a0 == FALSE)
        paletteNum = 0xB;
    else
        paletteNum = 0x6;

    if ((sShopData.martType) != MART_TYPE_TMHM)
        SetBgTilemapPalette(1, 0, 14, 30, 6, paletteNum);
    else
        SetBgTilemapPalette(1, 0, 12, 30, 8, paletteNum);

    ScheduleBgCopyTilemapToVram(1);
}

bool8 BuyMenuBuildListMenuTemplate(void)
{
    u16 i, v;

    sShopMenuListMenu = Alloc((sShopData.itemCount + 1) * sizeof(*sShopMenuListMenu));
    if (sShopMenuListMenu == NULL
     || (sShopMenuItemStrings = Alloc((sShopData.itemCount + 1) * sizeof(*sShopMenuItemStrings))) == NULL)
    {
        BuyMenuFreeMemory();
        SetShopExitCallback();
        return FALSE;
    }

    for (i = 0; i < sShopData.itemCount; i++)
    {
        PokeMartWriteNameAndIdAt(&sShopMenuListMenu[i], sShopData.itemList[i], sShopMenuItemStrings[i]);
    }
    StringCopy(sShopMenuItemStrings[i], gFameCheckerText_Cancel);
    sShopMenuListMenu[i].label = sShopMenuItemStrings[i];
    sShopMenuListMenu[i].index = -2;
    gMultiuseListMenuTemplate.items = sShopMenuListMenu;
    gMultiuseListMenuTemplate.totalItems = sShopData.itemCount + 1;
    gMultiuseListMenuTemplate.windowId = 4;
    gMultiuseListMenuTemplate.header_X = 0;
    gMultiuseListMenuTemplate.item_X = 9;
    gMultiuseListMenuTemplate.cursor_X = 1;
    gMultiuseListMenuTemplate.lettersSpacing = 0;
    gMultiuseListMenuTemplate.itemVerticalPadding = 2;
    gMultiuseListMenuTemplate.upText_Y = 2;
    gMultiuseListMenuTemplate.fontId = 2;
    gMultiuseListMenuTemplate.fillValue = 0;
    gMultiuseListMenuTemplate.cursorPal = GetFontAttribute(FONT_NORMAL, FONTATTR_COLOR_FOREGROUND);
    gMultiuseListMenuTemplate.cursorShadowPal = GetFontAttribute(FONT_NORMAL, FONTATTR_COLOR_SHADOW);
    gMultiuseListMenuTemplate.moveCursorFunc = BuyMenuPrintItemDescriptionAndShowItemIcon;
    gMultiuseListMenuTemplate.itemPrintFunc = BuyMenuPrintPriceInList;
    gMultiuseListMenuTemplate.scrollMultiple = 0;
    gMultiuseListMenuTemplate.cursorKind = 0;

    if (sShopData.martType == MART_TYPE_TMHM)
        v = 5;
    else
        v = 6;

    if ((sShopData.itemCount + 1) > v)
        gMultiuseListMenuTemplate.maxShowed = v;
    else
        gMultiuseListMenuTemplate.maxShowed = sShopData.itemCount + 1;

    sShopData.itemsShowed = gMultiuseListMenuTemplate.maxShowed;
    return TRUE;
}

static void PokeMartWriteNameAndIdAt(struct ListMenuItem *list, u16 index, u8 *dst)
{
    CopyItemName(index, dst);
    list->label = dst;
    list->index = index;
}

static void BuyMenuPrintItemDescriptionAndShowItemIcon(s32 item, bool8 onInit, struct ListMenu *list)
{
    const u8 *description;

    if (onInit != TRUE)
        PlaySE(SE_SELECT);

    if (item != INDEX_CANCEL)
        description = ItemId_GetDescription(item);
    else
        description = gText_QuitShopping;

    FillWindowPixelBuffer(5, PIXEL_FILL(0));
    if (sShopData.martType != MART_TYPE_TMHM)
    {
        RemoveBagItemIconSprite(sShopData.itemSlot ^ 1);
        if (item != INDEX_CANCEL)
            AddBagItemIconSprite(item, sShopData.itemSlot);
        else
            AddBagItemIconSprite(ITEMS_COUNT, sShopData.itemSlot);

        sShopData.itemSlot ^= 1;
        BuyMenuPrint(5, FONT_NORMAL, description, 0, 3, 2, 1, 0, 0);
    }
    else //TM Mart
    {
        FillWindowPixelBuffer(6, PIXEL_FILL(0));
        LoadTmHmNameInMart(item);
        BuyMenuPrint(5, FONT_NORMAL, description, 2, 3, 1, 0, 0, 0);
    }
}

static void BuyMenuPrintPriceInList(u8 windowId, u32 item, u8 y)
{
    s32 x;
    u8 *loc;

    if (item != INDEX_CANCEL)
    {
        ConvertIntToDecimalStringN(gStringVar1, ItemId_GetPrice(item), 0, 6);
        x = 6 - StringLength(gStringVar1);
        loc = gStringVar4;
        while (x-- != 0)
            *loc++ = 0;
        StringExpandPlaceholders(loc, gText_PokedollarVar1);
        BuyMenuPrint(windowId, FONT_SMALL, gStringVar4, 0x61, y, 0, 0, TEXT_SKIP_DRAW, 1);
    }
}

static void LoadTmHmNameInMart(s32 item)
{
    if (item != INDEX_CANCEL)
    {
        ConvertIntToDecimalStringN(gStringVar1, item - ITEM_DEVON_SCOPE, 2, 2);
        StringCopy(gStringVar4, gText_NumberClear01);
        StringAppend(gStringVar4, gStringVar1);
        BuyMenuPrint(6, FONT_SMALL, gStringVar4, 0, 0, 0, 0, TEXT_SKIP_DRAW, 1);
        StringCopy(gStringVar4, gMovesInfo[ItemIdToBattleMoveId(item)].name);
        BuyMenuPrint(6, FONT_NORMAL, gStringVar4, 0, 0x10, 0, 0, 0, 1);
    }
    else
    {
        BuyMenuPrint(6, FONT_SMALL, gText_ThreeHyphens, 0, 0, 0, 0, TEXT_SKIP_DRAW, 1);
        BuyMenuPrint(6, FONT_NORMAL, gText_SevenHyphens, 0, 0x10, 0, 0, 0, 1);
    }
}

u8 GetMartFontId(void)
{
    return sShopData.fontId;
}

static void BuyMenuPrintCursor(u8 listTaskId, u8 a1)
{
    BuyMenuPrintCursorAtYPosition(ListMenuGetYCoordForPrintingArrowCursor(listTaskId), a1);
}

static void BuyMenuPrintCursorAtYPosition(u8 y, u8 a1)
{
    if (a1 == 0xFF)
    {
        FillWindowPixelRect(4, 0, 1, y, GetFontAttribute(FONT_NORMAL, FONTATTR_MAX_LETTER_WIDTH), GetFontAttribute(FONT_NORMAL, FONTATTR_MAX_LETTER_HEIGHT));
        CopyWindowToVram(4, COPYWIN_GFX);
    }
    else
    {
        BuyMenuPrint(4, FONT_NORMAL, gText_SelectorArrow2, 1, y, 0, 0, 0, a1);
    }
}

static void BuyMenuFreeMemory(void)
{
    if (gShopTilemapBuffer1 != NULL)
        Free(gShopTilemapBuffer1);

    if (gShopTilemapBuffer2 != NULL)
        Free(gShopTilemapBuffer2);

    if (gShopTilemapBuffer3 != NULL)
        Free(gShopTilemapBuffer3);

    if (gShopTilemapBuffer4 != NULL)
        Free(gShopTilemapBuffer4);

    if (sShopMenuListMenu != NULL)
        Free(sShopMenuListMenu);

    if (sShopMenuItemStrings != NULL)
        Free(sShopMenuItemStrings);

    FreeAllWindowBuffers();
}

static void SetShopExitCallback(void)
{
    gFieldCallback = MapPostLoadHook_ReturnToShopMenu;
    SetMainCallback2(CB2_ReturnToField);
}


static void BuyMenuAddScrollIndicatorArrows(void)
{
    if (sShopData.martType != MART_TYPE_TMHM)
    {
        sShopData.unk16_11 = AddScrollIndicatorArrowPairParameterized(SCROLL_ARROW_UP, 160, 8, 104,
            (sShopData.itemCount - sShopData.itemsShowed) + 1, 110, 110, &sShopData.scrollOffset);
    }
    else
    {
        sShopData.unk16_11 = AddScrollIndicatorArrowPairParameterized(SCROLL_ARROW_UP, 160, 8, 88,
            (sShopData.itemCount - sShopData.itemsShowed) + 1, 110, 110, &sShopData.scrollOffset);
    }
}

static void BuyQuantityAddScrollIndicatorArrows(void)
{
    sShopData.unk18 = 1;
    sShopData.unk16_11 = AddScrollIndicatorArrowPairParameterized(SCROLL_ARROW_UP, 0x98, 0x48, 0x68, 2, 0x6E, 0x6E, &sShopData.unk18);
}

static void BuyMenuRemoveScrollIndicatorArrows(void)
{
    if ((sShopData.unk16_11) == 0x1F)
        return;

    RemoveScrollIndicatorArrowPair(sShopData.unk16_11);
    sShopData.unk16_11 = 0x1F;
}

static void BuyMenuDrawMapBg(void)
{
    s16 i, j, x, y;
    const struct MapLayout *mapLayout;
    u16 metatile;
    u8 metatileLayerType;

    mapLayout = gMapHeader.mapLayout;
    GetXYCoordsOneStepInFrontOfPlayer(&x, &y);
    x -= 2;
    y -= 3;

    for (j = 0; j < 10; j++)
    {
        for (i = 0; i < 5; i++)
        {
            metatile = MapGridGetMetatileIdAt(x + i, y + j);
            metatileLayerType = MapGridGetMetatileLayerTypeAt(x + i, y + j);

            if (metatile < NUM_METATILES_IN_PRIMARY)
                BuyMenuDrawMapMetatile(i, j, GetPrimaryTileset(mapLayout)->metatiles + metatile * 8, metatileLayerType);
            else
                BuyMenuDrawMapMetatile(i, j, GetSecondaryTileset(mapLayout)->metatiles + ((metatile - NUM_METATILES_IN_PRIMARY) * 8), metatileLayerType);
        }
    }
}

static void BuyMenuDrawMapMetatile(s16 x, s16 y, const u16 *src, u8 metatileLayerType)
{
    u16 offset1 = x * 2;
    u16 offset2 = y * 64 + 64;

    switch (metatileLayerType)
    {
    case METATILE_LAYER_TYPE_NORMAL:
        BuyMenuDrawMapMetatileLayer(*gShopTilemapBuffer4, offset1, offset2, src);
        BuyMenuDrawMapMetatileLayer(*gShopTilemapBuffer2, offset1, offset2, src + 4);
        break;
    case METATILE_LAYER_TYPE_COVERED:
        BuyMenuDrawMapMetatileLayer(*gShopTilemapBuffer3, offset1, offset2, src);
        BuyMenuDrawMapMetatileLayer(*gShopTilemapBuffer4, offset1, offset2, src + 4);
        break;
    case METATILE_LAYER_TYPE_SPLIT:
        BuyMenuDrawMapMetatileLayer(*gShopTilemapBuffer3, offset1, offset2, src);
        BuyMenuDrawMapMetatileLayer(*gShopTilemapBuffer2, offset1, offset2, src + 4);
        break;
    }
}

static void BuyMenuDrawMapMetatileLayer(u16 *dest, s16 offset1, s16 offset2, const u16 *src)
{
    dest[offset1 + offset2] = src[0]; // top left
    dest[offset1 + offset2 + 1] = src[1]; // top right
    dest[offset1 + offset2 + 32] = src[2]; // bottom left
    dest[offset1 + offset2 + 33] = src[3]; // bottom right
}

static void BuyMenuCollectObjectEventData(void)
{
    s16 facingX, facingY;
    u8 x, y, elevation;
    u8 num = 0;

    GetXYCoordsOneStepInFrontOfPlayer(&facingX, &facingY);
    elevation = PlayerGetElevation();

    for (y = 0; y < OBJECT_EVENTS_COUNT; y++)
        sViewportObjectEvents[y][OBJECT_EVENT_ID] = OBJECT_EVENTS_COUNT;

    for (y = 0; y < 5; y++)
    {
        for (x = 0; x < 7; x++)
        {
            u8 eventObjId = GetObjectEventIdByPosition(facingX - 3 + x, facingY - 2 + y, elevation);
            if (eventObjId != OBJECT_EVENTS_COUNT)
            {
                sViewportObjectEvents[num][OBJECT_EVENT_ID] = eventObjId;
                sViewportObjectEvents[num][X_COORD] = x;
                sViewportObjectEvents[num][Y_COORD] = y;

                switch (gObjectEvents[eventObjId].facingDirection)
                {
                    case DIR_SOUTH:
                        sViewportObjectEvents[num][ANIM_NUM] = 0;
                        break;
                    case DIR_NORTH:
                        sViewportObjectEvents[num][ANIM_NUM] = 1;
                        break;
                    case DIR_WEST:
                        sViewportObjectEvents[num][ANIM_NUM] = 2;
                        break;
                    case DIR_EAST:
                    default:
                        sViewportObjectEvents[num][ANIM_NUM] = 3;
                        break;
                }
                num++;
            }
        }
    }
}

static void BuyMenuDrawObjectEvents(void)
{
    u8 i, spriteId;
    const struct ObjectEventGraphicsInfo *graphicsInfo;

    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        if (sViewportObjectEvents[i][OBJECT_EVENT_ID] == OBJECT_EVENTS_COUNT)
            continue;

        graphicsInfo = GetObjectEventGraphicsInfo(gObjectEvents[sViewportObjectEvents[i][OBJECT_EVENT_ID]].graphicsId);
        spriteId = CreateObjectGraphicsSprite(
            gObjectEvents[sViewportObjectEvents[i][OBJECT_EVENT_ID]].graphicsId,
            SpriteCallbackDummy,
            (u16)sViewportObjectEvents[i][X_COORD] * 16 - 8,
            (u16)sViewportObjectEvents[i][Y_COORD] * 16 + 48 - graphicsInfo->height / 2,
            2);
        StartSpriteAnim(&gSprites[spriteId], sViewportObjectEvents[i][ANIM_NUM]);
    }
}

static void BuyMenuCopyTilemapData(void)
{
    s16 i;
    u16 *dst = *gShopTilemapBuffer2;
    u16 *src = *gShopTilemapBuffer1;

    for (i = 0; i < 0x400; i++)
    {
        if (src[i] == 0)
            continue;
        dst[i] = src[i] + 0xb3dc;
    }
}

static void BuyMenuPrintItemQuantityAndPrice(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    FillWindowPixelBuffer(3, PIXEL_FILL(1));
    PrintMoneyAmount(3, 0x36, 0xA, sShopData.itemPrice, TEXT_SKIP_DRAW);
    ConvertIntToDecimalStringN(gStringVar1, tItemCount, STR_CONV_MODE_LEADING_ZEROS, 2);
    StringExpandPlaceholders(gStringVar4, gText_TimesStrVar1);
    BuyMenuPrint(3, FONT_SMALL, gStringVar4, 2, 0xA, 0, 0, 0, 1);
}

static void Task_BuyMenu(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (!gPaletteFade.active)
    {
        s32 itemId = ListMenu_ProcessInput(tListTaskId);
        ListMenuGetScrollAndRow(tListTaskId, &sShopData.scrollOffset, &sShopData.selectedRow);
        switch (itemId)
        {
        case LIST_NOTHING_CHOSEN:
            break;
        case LIST_CANCEL:
            PlaySE(SE_SELECT);
            ExitBuyMenu(taskId);
            break;
        default:
            PlaySE(SE_SELECT);
            tItemId = itemId;
            ClearWindowTilemap(5);
            BuyMenuRemoveScrollIndicatorArrows();
            BuyMenuPrintCursor(tListTaskId, 2);
            RecolorItemDescriptionBox(1);
            sShopData.itemPrice = ItemId_GetPrice(itemId);
            if (!IsEnoughMoney(&gSaveBlock1Ptr->money, sShopData.itemPrice))
            {
                BuyMenuDisplayMessage(taskId, gText_YouDontHaveMoney, BuyMenuReturnToItemList);
            }
            else
            {
                CopyItemName(itemId, gStringVar1);
                BuyMenuDisplayMessage(taskId, gText_Var1CertainlyHowMany, Task_BuyHowManyDialogueInit);
            }
            break;
        }
    }
}

static void Task_BuyHowManyDialogueInit(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u16 quantityInBag = BagGetQuantityByItemId(tItemId);
    u16 maxQuantity;

    BuyMenuQuantityBoxThinBorder(1, 0);
    ConvertIntToDecimalStringN(gStringVar1, quantityInBag, STR_CONV_MODE_RIGHT_ALIGN, 3);
    StringExpandPlaceholders(gStringVar4, gText_InBagVar1);
    BuyMenuPrint(1, FONT_NORMAL, gStringVar4, 0, 2, 0, 0, 0, 1);
    tItemCount = 1;
    BuyMenuQuantityBoxNormalBorder(3, 0);
    BuyMenuPrintItemQuantityAndPrice(taskId);
    ScheduleBgCopyTilemapToVram(0);
    maxQuantity = GetMoney(&gSaveBlock1Ptr->money) / ItemId_GetPrice(tItemId);
    if (maxQuantity > 99)
        sShopData.maxQuantity = 99;
    else
        sShopData.maxQuantity = (u8)maxQuantity;

    if (maxQuantity != 1)
        BuyQuantityAddScrollIndicatorArrows();

    gTasks[taskId].func = Task_BuyHowManyDialogueHandleInput;
}

static void Task_BuyHowManyDialogueHandleInput(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (AdjustQuantityAccordingToDPadInput(&tItemCount, sShopData.maxQuantity) == TRUE)
    {
        sShopData.itemPrice = ItemId_GetPrice(tItemId) * tItemCount;
        BuyMenuPrintItemQuantityAndPrice(taskId);
    }
    else
    {
        if (JOY_NEW(A_BUTTON))
        {
            PlaySE(SE_SELECT);
            BuyMenuRemoveScrollIndicatorArrows();
            ClearStdWindowAndFrameToTransparent(3, FALSE);
            ClearStdWindowAndFrameToTransparent(1, FALSE);
            ClearWindowTilemap(3);
            ClearWindowTilemap(1);
            PutWindowTilemap(4);
            CopyItemName(tItemId, gStringVar1);
            ConvertIntToDecimalStringN(gStringVar2, tItemCount, STR_CONV_MODE_LEFT_ALIGN, 2);
            ConvertIntToDecimalStringN(gStringVar3, sShopData.itemPrice, STR_CONV_MODE_LEFT_ALIGN, 8);
            BuyMenuDisplayMessage(taskId, gText_Var1AndYouWantedVar2, CreateBuyMenuConfirmPurchaseWindow);
        }
        else if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            BuyMenuRemoveScrollIndicatorArrows();
            ClearStdWindowAndFrameToTransparent(3, FALSE);
            ClearStdWindowAndFrameToTransparent(1, FALSE);
            ClearWindowTilemap(3);
            ClearWindowTilemap(1);
            BuyMenuReturnToItemList(taskId);
        }
    }
}

static void CreateBuyMenuConfirmPurchaseWindow(u8 taskId)
{
    BuyMenuConfirmPurchase(taskId, &sShopMenuActions_BuyQuit);
}

static void BuyMenuTryMakePurchase(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    PutWindowTilemap(4);
    if (AddBagItem(tItemId, tItemCount) == TRUE)
    {
        GetSetItemObtained(tItemId, FLAG_SET_ITEM_OBTAINED);
        BuyMenuDisplayMessage(taskId, gText_HereYouGoThankYou, BuyMenuSubtractMoney);
        DebugFunc_PrintPurchaseDetails(taskId);
        RecordItemTransaction(tItemId, tItemCount, QL_EVENT_BOUGHT_ITEM - QL_EVENT_USED_POKEMART);
    }
    else
    {
        BuyMenuDisplayMessage(taskId, gText_NoMoreRoomForThis, BuyMenuReturnToItemList);
    }
}

static void BuyMenuSubtractMoney(u8 taskId)
{
    IncrementGameStat(GAME_STAT_SHOPPED);
    RemoveMoney(&gSaveBlock1Ptr->money, sShopData.itemPrice);
    PlaySE(SE_SHOP);
    PrintMoneyAmountInMoneyBox(0, GetMoney(&gSaveBlock1Ptr->money), 0);
    gTasks[taskId].func = Task_ReturnToItemListAfterItemPurchase;
}

static void Task_ReturnToItemListAfterItemPurchase(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    if (JOY_NEW(A_BUTTON) || JOY_NEW(B_BUTTON))
    {
        u16 premierBallsToAdd = tItemCount / 10;
        if (premierBallsToAdd >= 1
         && ((I_PREMIER_BALL_BONUS <= GEN_7 && tItemId == ITEM_POKE_BALL)
          || (I_PREMIER_BALL_BONUS >= GEN_8 && (ItemId_GetPocket(tItemId) == POCKET_POKE_BALLS))))
        {
            u32 spaceAvailable = GetFreeSpaceForItemInBag(ITEM_PREMIER_BALL);
            if (spaceAvailable < premierBallsToAdd)
                premierBallsToAdd = spaceAvailable;
        }
        else
        {
            premierBallsToAdd = 0;
        }

        PlaySE(SE_SELECT);
        AddBagItem(ITEM_PREMIER_BALL, premierBallsToAdd);
        if (premierBallsToAdd > 0)
        {
            ConvertIntToDecimalStringN(gStringVar1, premierBallsToAdd, STR_CONV_MODE_LEFT_ALIGN, MAX_ITEM_DIGITS);
            BuyMenuDisplayMessage(taskId, (premierBallsToAdd >= 2 ? gText_ThrowInPremierBalls : gText_ThrowInPremierBall), BuyMenuReturnToItemList);
        }
        else
        {
            BuyMenuReturnToItemList(taskId);
        }
    }
}

static void BuyMenuReturnToItemList(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    ClearDialogWindowAndFrameToTransparent(2, FALSE);
    BuyMenuPrintCursor(tListTaskId, 1);
    RecolorItemDescriptionBox(0);
    PutWindowTilemap(4);
    PutWindowTilemap(5);
    if (sShopData.martType == MART_TYPE_TMHM)
        PutWindowTilemap(6);

    ScheduleBgCopyTilemapToVram(0);
    BuyMenuAddScrollIndicatorArrows();
    gTasks[taskId].func = Task_BuyMenu;
}

static void ExitBuyMenu(u8 taskId)
{
    gFieldCallback = MapPostLoadHook_ReturnToShopMenu;
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].func = Task_ExitBuyMenu;
}

static void Task_ExitBuyMenu(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (!gPaletteFade.active)
    {
        DestroyListMenuTask(tListTaskId, NULL, NULL);
        BuyMenuFreeMemory();
        SetMainCallback2(CB2_ReturnToField);
        DestroyTask(taskId);
    }
}

static void DebugFunc_PrintPurchaseDetails(u8 taskId)
{
}

static void DebugFunc_PrintShopMenuHistoryBeforeClearMaybe(void)
{
}

// Records a transaction during a single shopping session.
// This is for the Quest Log to save information about the player's purchases/sales when they finish.
void RecordItemTransaction(u16 itemId, u16 quantity, u8 logEventId)
{
    struct QuestLogEvent_Shop *history;

    // There should only be a single entry for buying/selling respectively,
    // so if one has already been created then get it first.
    if (sHistory[0].logEventId == logEventId)
    {
        history = &sHistory[0];
    }
    else if (sHistory[1].logEventId == logEventId)
    {
        history = &sHistory[1];
    }
    else
    {
        // First transaction of this type, save it in an empty slot
        if (sHistory[0].logEventId == 0)
            history = &sHistory[0];
        else
            history = &sHistory[1];
        history->logEventId = logEventId;
    }

    // Set flag if this isn't the first time we've bought/sold in this session
    if (history->lastItemId != ITEM_NONE)
        history->hasMultipleTransactions = TRUE;

    history->lastItemId = itemId;

    // Add to number of items bought/sold
    if (history->itemQuantity < 999)
    {
        history->itemQuantity += quantity;
        if (history->itemQuantity > 999)
            history->itemQuantity = 999;
    }

    // Add to amount of money spent buying or earned selling
    if (history->totalMoney < 999999)
    {
        // logEventId will either be 1 (bought) or 2 (sold)
        // so for buying it will add the full price and selling will add half price
        history->totalMoney += (ItemId_GetPrice(itemId) >> (logEventId - 1)) * quantity;
        if (history->totalMoney > 999999)
            history->totalMoney = 999999;
    }
}

// Will record QL_EVENT_BOUGHT_ITEM and/or QL_EVENT_SOLD_ITEM, or nothing.
static void RecordTransactionForQuestLog(void)
{
    u16 eventId = sHistory[0].logEventId;
    if (eventId != 0)
        SetQuestLogEvent(eventId + QL_EVENT_USED_POKEMART, (const u16 *)&sHistory[0]);

    eventId = sHistory[1].logEventId;
    if (eventId != 0)
        SetQuestLogEvent(eventId + QL_EVENT_USED_POKEMART, (const u16 *)&sHistory[1]);
}

void CreatePokemartMenu(const u16 *itemsForSale)
{
    SetShopItemsForSale(itemsForSale);
    CreateShopMenu(MART_TYPE_REGULAR);
    SetShopMenuCallback(ScriptContext_Enable);
    DebugFunc_PrintShopMenuHistoryBeforeClearMaybe();
    memset(&sHistory, 0, sizeof(sHistory));
    sHistory[0].mapSec = gMapHeader.regionMapSectionId;
    sHistory[1].mapSec = gMapHeader.regionMapSectionId;
}

void CreateDecorationShop1Menu(const u16 *itemsForSale)
{
    SetShopItemsForSale(itemsForSale);
    CreateShopMenu(MART_TYPE_DECOR);
    SetShopMenuCallback(ScriptContext_Enable);
}

void CreateDecorationShop2Menu(const u16 *itemsForSale)
{
    SetShopItemsForSale(itemsForSale);
    CreateShopMenu(MART_TYPE_DECOR2);
    SetShopMenuCallback(ScriptContext_Enable);
}





