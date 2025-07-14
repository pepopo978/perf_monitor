#include "events.hpp"
#include <queue>

namespace perf_monitor {
    // Event tracking globals - definitions
    std::map<EVENT_ID, FunctionStats> gEventStats = {
        {EVENT_ID_CAPTURECHANGED,     FunctionStats("EVT_CAPTURECHANGED")},
        {EVENT_ID_CHAR,               FunctionStats("EVT_CHAR")},
        {EVENT_ID_FOCUS,              FunctionStats("EVT_FOCUS")},
        {EVENT_ID_CLOSE,              FunctionStats("EVT_CLOSE")},
        {EVENT_ID_DESTROY,            FunctionStats("EVT_DESTROY")},
        {EVENT_ID_IDLE,               FunctionStats("EVT_IDLE")},
        {EVENT_ID_POLL,               FunctionStats("EVT_POLL")},
        {EVENT_ID_INITIALIZE,         FunctionStats("EVT_INITIALIZE")},
        {EVENT_ID_KEYDOWN,            FunctionStats("EVT_KEYDOWN")},
        {EVENT_ID_KEYUP,              FunctionStats("EVT_KEYUP")},
        {EVENT_ID_KEYDOWN_REPEATING,  FunctionStats("EVT_KEYDOWN_REPEATING")},
        {EVENT_ID_MOUSEDOWN,          FunctionStats("EVT_MOUSEDOWN")},
        {EVENT_ID_MOUSEMOVE,          FunctionStats("EVT_MOUSEMOVE")},
        {EVENT_ID_MOUSEMOVE_RELATIVE, FunctionStats("EVT_MOUSEMOVE_RELATIVE")},
        {EVENT_ID_MOUSEUP,            FunctionStats("EVT_MOUSEUP")},
        {EVENT_ID_MOUSEMODE_CHANGED,  FunctionStats("EVT_MOUSEMODE_CHANGED")},
        {EVENT_ID_MOUSEWHEEL,         FunctionStats("EVT_MOUSEWHEEL")},
        {EVENT_ID_PAINT,              FunctionStats("EVT_PAINT")},
        {EVENT_ID_NET_DATA,           FunctionStats("EVT_NET_DATA")},
        {EVENT_ID_NET_CONNECT,        FunctionStats("EVT_NET_CONNECT")},
        {EVENT_ID_NET_DISCONNECT,     FunctionStats("EVT_NET_DISCONNECT")},
        {EVENT_ID_NET_CANTCONNECT,    FunctionStats("EVT_NET_CANTCONNECT")},
        {EVENT_ID_NET_DESTROY,        FunctionStats("EVT_NET_DESTROY")},
        {EVENT_ID_CONSOLE_INPUT,      FunctionStats("EVT_CONSOLE_INPUT")},
        {EVENT_ID_ENGINENET,          FunctionStats("EVT_ENGINENET")},
        {EVENT_ID_BATTLENET,          FunctionStats("EVT_BATTLENET")},
        {EVENT_ID_WOW_Q_IDLE,         FunctionStats("EVT_WOW_Q_IDLE")},
        {EVENT_ID_IME,                FunctionStats("EVT_IME")},
        {EVENT_ID_SIZE,               FunctionStats("EVT_SIZE")}
    };
    
    std::map<std::string, FunctionStats> gAddonStats;
    std::map<std::string, FunctionStats> gAddonScriptEventStats;
    std::map<std::string, std::priority_queue<EventStats>> gAddonSlowEvents;
    int gLastEventCode = -1;
    
    // Per-event code duration tracking
    std::map<int, FunctionStats> gEventCodeStats;
    std::map<int, std::chrono::high_resolution_clock::time_point> gEventCodeStartTimes;

    void initializeEventStats() {
        // Event stats are already initialized above, so we don't need to do anything here
        // This function is kept for future initialization needs
    }

    std::string GetEventName(int eventCode) {
        switch (eventCode) {
            case Events::UNIT_PET_01: return "UNIT_PET";
            case Events::UNIT_PET_02: return "UNIT_PET";
            case Events::UNIT_HEALTH: return "UNIT_HEALTH";
            case Events::UNIT_MANA_01: return "UNIT_MANA";
            case Events::UNIT_RAGE: return "UNIT_RAGE";
            case Events::UNIT_FOCUS: return "UNIT_FOCUS";
            case Events::UNIT_ENERGY: return "UNIT_ENERGY";
            case Events::UNIT_HAPPINESS: return "UNIT_HAPPINESS";
            case Events::UNIT_MAXHEALTH: return "UNIT_MAXHEALTH";
            case Events::UNIT_MAXMANA: return "UNIT_MAXMANA";
            case Events::UNIT_MAXRAGE: return "UNIT_MAXRAGE";
            case Events::UNIT_MAXFOCUS: return "UNIT_MAXFOCUS";
            case Events::UNIT_MAXENERGY: return "UNIT_MAXENERGY";
            case Events::UNIT_MAXHAPPINESS: return "UNIT_MAXHAPPINESS";
            case Events::UNIT_LEVEL: return "UNIT_LEVEL";
            case Events::UNIT_FACTION: return "UNIT_FACTION";
            case Events::UNIT_DISPLAYPOWER: return "UNIT_DISPLAYPOWER";
            case Events::UNIT_FLAGS: return "UNIT_FLAGS";
            case Events::UNIT_AURA_01: return "UNIT_AURA";
            case Events::UNIT_AURA_02: return "UNIT_AURA";
            case Events::UNIT_ATTACK_SPEED_01: return "UNIT_ATTACK_SPEED";
            case Events::UNIT_ATTACK_SPEED_02: return "UNIT_ATTACK_SPEED";
            case Events::UNIT_RANGEDDAMAGE_01: return "UNIT_RANGEDDAMAGE";
            case Events::UNIT_DAMAGE_01: return "UNIT_DAMAGE";
            case Events::UNIT_DAMAGE_02: return "UNIT_DAMAGE";
            case Events::UNIT_DAMAGE_03: return "UNIT_DAMAGE";
            case Events::UNIT_DAMAGE_04: return "UNIT_DAMAGE";
            case Events::UNIT_LOYALTY: return "UNIT_LOYALTY";
            case Events::UNIT_PET_EXPERIENCE_01: return "UNIT_PET_EXPERIENCE";
            case Events::UNIT_PET_EXPERIENCE_02: return "UNIT_PET_EXPERIENCE";
            case Events::UNIT_DYNAMIC_FLAGS: return "UNIT_DYNAMIC_FLAGS";
            case Events::UNIT_PET_TRAINING_POINTS: return "UNIT_PET_TRAINING_POINTS";
            case Events::UNIT_STATS_01: return "UNIT_STATS";
            case Events::UNIT_STATS_02: return "UNIT_STATS";
            case Events::UNIT_STATS_03: return "UNIT_STATS";
            case Events::UNIT_STATS_04: return "UNIT_STATS";
            case Events::UNIT_STATS_05: return "UNIT_STATS";
            case Events::UNIT_RESISTANCES_01: return "UNIT_RESISTANCES";
            case Events::UNIT_RESISTANCES_02: return "UNIT_RESISTANCES";
            case Events::UNIT_RESISTANCES_03: return "UNIT_RESISTANCES";
            case Events::UNIT_RESISTANCES_04: return "UNIT_RESISTANCES";
            case Events::UNIT_RESISTANCES_05: return "UNIT_RESISTANCES";
            case Events::UNIT_RESISTANCES_06: return "UNIT_RESISTANCES";
            case Events::UNIT_RESISTANCES_07: return "UNIT_RESISTANCES";
            case Events::UNIT_ATTACK_POWER_01: return "UNIT_ATTACK_POWER";
            case Events::UNIT_ATTACK_POWER_02: return "UNIT_ATTACK_POWER";
            case Events::UNIT_ATTACK_POWER_03: return "UNIT_ATTACK_POWER";
            case Events::UNIT_RANGED_ATTACK_POWER_01: return "UNIT_RANGED_ATTACK_POWER";
            case Events::UNIT_RANGED_ATTACK_POWER_02: return "UNIT_RANGED_ATTACK_POWER";
            case Events::UNIT_RANGED_ATTACK_POWER_03: return "UNIT_RANGED_ATTACK_POWER";
            case Events::UNIT_RANGEDDAMAGE_02: return "UNIT_RANGEDDAMAGE";
            case Events::UNIT_RANGEDDAMAGE_03: return "UNIT_RANGEDDAMAGE";
            case Events::UNIT_MANA1: return "UNIT_MANA1";
            case Events::UNIT_MANA2: return "UNIT_MANA2";
            case Events::UNIT_COMBAT: return "UNIT_COMBAT";
            case Events::UNIT_NAME_UPDATE: return "UNIT_NAME_UPDATE";
            case Events::UNIT_PORTRAIT_UPDATE: return "UNIT_PORTRAIT_UPDATE";
            case Events::UNIT_MODEL_CHANGED: return "UNIT_MODEL_CHANGED";
            case Events::UNIT_INVENTORY_CHANGED: return "UNIT_INVENTORY_CHANGED";
            case Events::UNIT_CLASSIFICATION_CHANGED: return "UNIT_CLASSIFICATION_CHANGED";
            case Events::ITEM_LOCK_CHANGED: return "ITEM_LOCK_CHANGED";
            case Events::PLAYER_XP_UPDATE: return "PLAYER_XP_UPDATE";
            case Events::PLAYER_REGEN_DISABLED: return "PLAYER_REGEN_DISABLED";
            case Events::PLAYER_REGEN_ENABLED: return "PLAYER_REGEN_ENABLED";
            case Events::PLAYER_AURAS_CHANGED: return "PLAYER_AURAS_CHANGED";
            case Events::PLAYER_ENTER_COMBAT: return "PLAYER_ENTER_COMBAT";
            case Events::PLAYER_LEAVE_COMBAT: return "PLAYER_LEAVE_COMBAT";
            case Events::PLAYER_TARGET_CHANGED: return "PLAYER_TARGET_CHANGED";
            case Events::PLAYER_CONTROL_LOST: return "PLAYER_CONTROL_LOST";
            case Events::PLAYER_CONTROL_GAINED: return "PLAYER_CONTROL_GAINED";
            case Events::PLAYER_FARSIGHT_FOCUS_CHANGED: return "PLAYER_FARSIGHT_FOCUS_CHANGED";
            case Events::PLAYER_LEVEL_UP: return "PLAYER_LEVEL_UP";
            case Events::PLAYER_MONEY: return "PLAYER_MONEY";
            case Events::PLAYER_DAMAGE_DONE_MODS: return "PLAYER_DAMAGE_DONE_MODS";
            case Events::PLAYER_COMBO_POINTS: return "PLAYER_COMBO_POINTS";
            case Events::ZONE_CHANGED: return "ZONE_CHANGED";
            case Events::ZONE_CHANGED_INDOORS: return "ZONE_CHANGED_INDOORS";
            case Events::ZONE_CHANGED_NEW_AREA: return "ZONE_CHANGED_NEW_AREA";
            case Events::MINIMAP_ZONE_CHANGED: return "MINIMAP_ZONE_CHANGED";
            case Events::MINIMAP_UPDATE_ZOOM: return "MINIMAP_UPDATE_ZOOM";
            case Events::SCREENSHOT_SUCCEEDED: return "SCREENSHOT_SUCCEEDED";
            case Events::SCREENSHOT_FAILED: return "SCREENSHOT_FAILED";
            case Events::ACTIONBAR_SHOWGRID: return "ACTIONBAR_SHOWGRID";
            case Events::ACTIONBAR_HIDEGRID: return "ACTIONBAR_HIDEGRID";
            case Events::ACTIONBAR_PAGE_CHANGED: return "ACTIONBAR_PAGE_CHANGED";
            case Events::ACTIONBAR_SLOT_CHANGED: return "ACTIONBAR_SLOT_CHANGED";
            case Events::ACTIONBAR_UPDATE_STATE: return "ACTIONBAR_UPDATE_STATE";
            case Events::ACTIONBAR_UPDATE_USABLE: return "ACTIONBAR_UPDATE_USABLE";
            case Events::ACTIONBAR_UPDATE_COOLDOWN: return "ACTIONBAR_UPDATE_COOLDOWN";
            case Events::UPDATE_BONUS_ACTIONBAR: return "UPDATE_BONUS_ACTIONBAR";
            case Events::PARTY_MEMBERS_CHANGED: return "PARTY_MEMBERS_CHANGED";
            case Events::PARTY_LEADER_CHANGED: return "PARTY_LEADER_CHANGED";
            case Events::PARTY_MEMBER_ENABLE: return "PARTY_MEMBER_ENABLE";
            case Events::PARTY_MEMBER_DISABLE: return "PARTY_MEMBER_DISABLE";
            case Events::PARTY_LOOT_METHOD_CHANGED: return "PARTY_LOOT_METHOD_CHANGED";
            case Events::SYSMSG: return "SYSMSG";
            case Events::UI_ERROR_MESSAGE: return "UI_ERROR_MESSAGE";
            case Events::UI_INFO_MESSAGE: return "UI_INFO_MESSAGE";
            case Events::UPDATE_CHAT_COLOR: return "UPDATE_CHAT_COLOR";
            case Events::CHAT_MSG_ADDON: return "CHAT_MSG_ADDON";
            case Events::CHAT_MSG_SAY: return "CHAT_MSG_SAY";
            case Events::CHAT_MSG_PARTY: return "CHAT_MSG_PARTY";
            case Events::CHAT_MSG_RAID: return "CHAT_MSG_RAID";
            case Events::CHAT_MSG_GUILD: return "CHAT_MSG_GUILD";
            case Events::CHAT_MSG_OFFICER: return "CHAT_MSG_OFFICER";
            case Events::CHAT_MSG_YELL: return "CHAT_MSG_YELL";
            case Events::CHAT_MSG_WHISPER: return "CHAT_MSG_WHISPER";
            case Events::CHAT_MSG_WHISPER_INFORM: return "CHAT_MSG_WHISPER_INFORM";
            case Events::CHAT_MSG_EMOTE: return "CHAT_MSG_EMOTE";
            case Events::CHAT_MSG_TEXT_EMOTE: return "CHAT_MSG_TEXT_EMOTE";
            case Events::CHAT_MSG_SYSTEM: return "CHAT_MSG_SYSTEM";
            case Events::CHAT_MSG_MONSTER_SAY: return "CHAT_MSG_MONSTER_SAY";
            case Events::CHAT_MSG_MONSTER_YELL: return "CHAT_MSG_MONSTER_YELL";
            case Events::CHAT_MSG_MONSTER_WHISPER: return "CHAT_MSG_MONSTER_WHISPER";
            case Events::CHAT_MSG_MONSTER_EMOTE: return "CHAT_MSG_MONSTER_EMOTE";
            case Events::CHAT_MSG_CHANNEL: return "CHAT_MSG_CHANNEL";
            case Events::CHAT_MSG_CHANNEL_JOIN: return "CHAT_MSG_CHANNEL_JOIN";
            case Events::CHAT_MSG_CHANNEL_LEAVE: return "CHAT_MSG_CHANNEL_LEAVE";
            case Events::CHAT_MSG_CHANNEL_LIST: return "CHAT_MSG_CHANNEL_LIST";
            case Events::CHAT_MSG_CHANNEL_NOTICE: return "CHAT_MSG_CHANNEL_NOTICE";
            case Events::CHAT_MSG_CHANNEL_NOTICE_USER: return "CHAT_MSG_CHANNEL_NOTICE_USER";
            case Events::CHAT_MSG_AFK: return "CHAT_MSG_AFK";
            case Events::CHAT_MSG_DND: return "CHAT_MSG_DND";
            case Events::CHAT_MSG_COMBAT_LOG: return "CHAT_MSG_COMBAT_LOG";
            case Events::CHAT_MSG_IGNORED: return "CHAT_MSG_IGNORED";
            case Events::CHAT_MSG_SKILL: return "CHAT_MSG_SKILL";
            case Events::CHAT_MSG_LOOT: return "CHAT_MSG_LOOT";
            case Events::CHAT_MSG_MONEY: return "CHAT_MSG_MONEY";
            case Events::CHAT_MSG_RAID_LEADER: return "CHAT_MSG_RAID_LEADER";
            case Events::CHAT_MSG_RAID_WARNING: return "CHAT_MSG_RAID_WARNING";
            case Events::LANGUAGE_LIST_CHANGED: return "LANGUAGE_LIST_CHANGED";
            case Events::TIME_PLAYED_MSG: return "TIME_PLAYED_MSG";
            case Events::SPELLS_CHANGED: return "SPELLS_CHANGED";
            case Events::CURRENT_SPELL_CAST_CHANGED: return "CURRENT_SPELL_CAST_CHANGED";
            case Events::SPELL_UPDATE_COOLDOWN: return "SPELL_UPDATE_COOLDOWN";
            case Events::SPELL_UPDATE_USABLE: return "SPELL_UPDATE_USABLE";
            case Events::CHARACTER_POINTS_CHANGED: return "CHARACTER_POINTS_CHANGED";
            case Events::SKILL_LINES_CHANGED: return "SKILL_LINES_CHANGED";
            case Events::ITEM_PUSH: return "ITEM_PUSH";
            case Events::LOOT_OPENED: return "LOOT_OPENED";
            case Events::LOOT_SLOT_CLEARED: return "LOOT_SLOT_CLEARED";
            case Events::LOOT_CLOSED: return "LOOT_CLOSED";
            case Events::PLAYER_LOGIN: return "PLAYER_LOGIN";
            case Events::PLAYER_LOGOUT: return "PLAYER_LOGOUT";
            case Events::PLAYER_ENTERING_WORLD: return "PLAYER_ENTERING_WORLD";
            case Events::PLAYER_LEAVING_WORLD: return "PLAYER_LEAVING_WORLD";
            case Events::PLAYER_ALIVE: return "PLAYER_ALIVE";
            case Events::PLAYER_DEAD: return "PLAYER_DEAD";
            case Events::PLAYER_CAMPING: return "PLAYER_CAMPING";
            case Events::PLAYER_QUITING: return "PLAYER_QUITING";
            case Events::LOGOUT_CANCEL: return "LOGOUT_CANCEL";
            case Events::RESURRECT_REQUEST: return "RESURRECT_REQUEST";
            case Events::PARTY_INVITE_REQUEST: return "PARTY_INVITE_REQUEST";
            case Events::PARTY_INVITE_CANCEL: return "PARTY_INVITE_CANCEL";
            case Events::GUILD_INVITE_REQUEST: return "GUILD_INVITE_REQUEST";
            case Events::GUILD_INVITE_CANCEL: return "GUILD_INVITE_CANCEL";
            case Events::GUILD_MOTD: return "GUILD_MOTD";
            case Events::TRADE_REQUEST: return "TRADE_REQUEST";
            case Events::TRADE_REQUEST_CANCEL: return "TRADE_REQUEST_CANCEL";
            case Events::LOOT_BIND_CONFIRM: return "LOOT_BIND_CONFIRM";
            case Events::EQUIP_BIND_CONFIRM: return "EQUIP_BIND_CONFIRM";
            case Events::AUTOEQUIP_BIND_CONFIRM: return "AUTOEQUIP_BIND_CONFIRM";
            case Events::USE_BIND_CONFIRM: return "USE_BIND_CONFIRM";
            case Events::DELETE_ITEM_CONFIRM: return "DELETE_ITEM_CONFIRM";
            case Events::CURSOR_UPDATE: return "CURSOR_UPDATE";
            case Events::ITEM_TEXT_BEGIN: return "ITEM_TEXT_BEGIN";
            case Events::ITEM_TEXT_TRANSLATION: return "ITEM_TEXT_TRANSLATION";
            case Events::ITEM_TEXT_READY: return "ITEM_TEXT_READY";
            case Events::ITEM_TEXT_CLOSED: return "ITEM_TEXT_CLOSED";
            case Events::GOSSIP_SHOW: return "GOSSIP_SHOW";
            case Events::GOSSIP_ENTER_CODE: return "GOSSIP_ENTER_CODE";
            case Events::GOSSIP_CLOSED: return "GOSSIP_CLOSED";
            case Events::QUEST_GREETING: return "QUEST_GREETING";
            case Events::QUEST_DETAIL: return "QUEST_DETAIL";
            case Events::QUEST_PROGRESS: return "QUEST_PROGRESS";
            case Events::QUEST_COMPLETE: return "QUEST_COMPLETE";
            case Events::QUEST_FINISHED: return "QUEST_FINISHED";
            case Events::QUEST_ITEM_UPDATE: return "QUEST_ITEM_UPDATE";
            case Events::TAXIMAP_OPENED: return "TAXIMAP_OPENED";
            case Events::TAXIMAP_CLOSED: return "TAXIMAP_CLOSED";
            case Events::QUEST_LOG_UPDATE: return "QUEST_LOG_UPDATE";
            case Events::TRAINER_SHOW: return "TRAINER_SHOW";
            case Events::TRAINER_UPDATE: return "TRAINER_UPDATE";
            case Events::TRAINER_CLOSED: return "TRAINER_CLOSED";
            case Events::CVAR_UPDATE: return "CVAR_UPDATE";
            case Events::TRADE_SKILL_SHOW: return "TRADE_SKILL_SHOW";
            case Events::TRADE_SKILL_UPDATE: return "TRADE_SKILL_UPDATE";
            case Events::TRADE_SKILL_CLOSE: return "TRADE_SKILL_CLOSE";
            case Events::MERCHANT_SHOW: return "MERCHANT_SHOW";
            case Events::MERCHANT_UPDATE: return "MERCHANT_UPDATE";
            case Events::MERCHANT_CLOSED: return "MERCHANT_CLOSED";
            case Events::TRADE_SHOW: return "TRADE_SHOW";
            case Events::TRADE_CLOSED: return "TRADE_CLOSED";
            case Events::TRADE_UPDATE: return "TRADE_UPDATE";
            case Events::TRADE_ACCEPT_UPDATE: return "TRADE_ACCEPT_UPDATE";
            case Events::TRADE_TARGET_ITEM_CHANGED: return "TRADE_TARGET_ITEM_CHANGED";
            case Events::TRADE_PLAYER_ITEM_CHANGED: return "TRADE_PLAYER_ITEM_CHANGED";
            case Events::TRADE_MONEY_CHANGED: return "TRADE_MONEY_CHANGED";
            case Events::PLAYER_TRADE_MONEY: return "PLAYER_TRADE_MONEY";
            case Events::BAG_OPEN: return "BAG_OPEN";
            case Events::BAG_UPDATE: return "BAG_UPDATE";
            case Events::BAG_CLOSED: return "BAG_CLOSED";
            case Events::BAG_UPDATE_COOLDOWN: return "BAG_UPDATE_COOLDOWN";
            case Events::LOCALPLAYER_PET_RENAMED: return "LOCALPLAYER_PET_RENAMED";
            case Events::UNIT_ATTACK: return "UNIT_ATTACK";
            case Events::UNIT_DEFENSE: return "UNIT_DEFENSE";
            case Events::PET_ATTACK_START: return "PET_ATTACK_START";
            case Events::PET_ATTACK_STOP: return "PET_ATTACK_STOP";
            case Events::UPDATE_MOUSEOVER_UNIT: return "UPDATE_MOUSEOVER_UNIT";
            case Events::SPELLCAST_START: return "SPELLCAST_START";
            case Events::SPELLCAST_STOP: return "SPELLCAST_STOP";
            case Events::SPELLCAST_FAILED: return "SPELLCAST_FAILED";
            case Events::SPELLCAST_INTERRUPTED: return "SPELLCAST_INTERRUPTED";
            case Events::SPELLCAST_DELAYED: return "SPELLCAST_DELAYED";
            case Events::SPELLCAST_CHANNEL_START: return "SPELLCAST_CHANNEL_START";
            case Events::SPELLCAST_CHANNEL_UPDATE: return "SPELLCAST_CHANNEL_UPDATE";
            case Events::SPELLCAST_CHANNEL_STOP: return "SPELLCAST_CHANNEL_STOP";
            case Events::PLAYER_GUILD_UPDATE: return "PLAYER_GUILD_UPDATE";
            case Events::QUEST_ACCEPT_CONFIRM: return "QUEST_ACCEPT_CONFIRM";
            case Events::PLAYERBANKSLOTS_CHANGED: return "PLAYERBANKSLOTS_CHANGED";
            case Events::BANKFRAME_OPENED: return "BANKFRAME_OPENED";
            case Events::BANKFRAME_CLOSED: return "BANKFRAME_CLOSED";
            case Events::PLAYERBANKBAGSLOTS_CHANGED: return "PLAYERBANKBAGSLOTS_CHANGED";
            case Events::FRIENDLIST_UPDATE: return "FRIENDLIST_UPDATE";
            case Events::IGNORELIST_UPDATE: return "IGNORELIST_UPDATE";
            case Events::PET_BAR_UPDATE: return "PET_BAR_UPDATE";
            case Events::PET_BAR_UPDATE_COOLDOWN: return "PET_BAR_UPDATE_COOLDOWN";
            case Events::PET_BAR_SHOWGRID: return "PET_BAR_SHOWGRID";
            case Events::PET_BAR_HIDEGRID: return "PET_BAR_HIDEGRID";
            case Events::MINIMAP_PING: return "MINIMAP_PING";
            case Events::CHAT_MSG_COMBAT_MISC_INFO: return "CHAT_MSG_COMBAT_MISC_INFO";
            case Events::CRAFT_SHOW: return "CRAFT_SHOW";
            case Events::CRAFT_UPDATE: return "CRAFT_UPDATE";
            case Events::CRAFT_CLOSE: return "CRAFT_CLOSE";
            case Events::MIRROR_TIMER_START: return "MIRROR_TIMER_START";
            case Events::MIRROR_TIMER_PAUSE: return "MIRROR_TIMER_PAUSE";
            case Events::MIRROR_TIMER_STOP: return "MIRROR_TIMER_STOP";
            case Events::WORLD_MAP_UPDATE: return "WORLD_MAP_UPDATE";
            case Events::WORLD_MAP_NAME_UPDATE: return "WORLD_MAP_NAME_UPDATE";
            case Events::AUTOFOLLOW_BEGIN: return "AUTOFOLLOW_BEGIN";
            case Events::AUTOFOLLOW_END: return "AUTOFOLLOW_END";
            case Events::SPELL_QUEUE_EVENT: return "SPELL_QUEUE_EVENT";
            case Events::CINEMATIC_START: return "CINEMATIC_START";
            case Events::CINEMATIC_STOP: return "CINEMATIC_STOP";
            case Events::UPDATE_FACTION: return "UPDATE_FACTION";
            case Events::CLOSE_WORLD_MAP: return "CLOSE_WORLD_MAP";
            case Events::OPEN_TABARD_FRAME: return "OPEN_TABARD_FRAME";
            case Events::CLOSE_TABARD_FRAME: return "CLOSE_TABARD_FRAME";
            case Events::TABARD_CANSAVE_CHANGED: return "TABARD_CANSAVE_CHANGED";
            case Events::SHOW_COMPARE_TOOLTIP: return "SHOW_COMPARE_TOOLTIP";
            case Events::GUILD_REGISTRAR_SHOW: return "GUILD_REGISTRAR_SHOW";
            case Events::GUILD_REGISTRAR_CLOSED: return "GUILD_REGISTRAR_CLOSED";
            case Events::DUEL_REQUESTED: return "DUEL_REQUESTED";
            case Events::DUEL_OUTOFBOUNDS: return "DUEL_OUTOFBOUNDS";
            case Events::DUEL_INBOUNDS: return "DUEL_INBOUNDS";
            case Events::DUEL_FINISHED: return "DUEL_FINISHED";
            case Events::TUTORIAL_TRIGGER: return "TUTORIAL_TRIGGER";
            case Events::PET_DISMISS_START: return "PET_DISMISS_START";
            case Events::UPDATE_BINDINGS: return "UPDATE_BINDINGS";
            case Events::UPDATE_SHAPESHIFT_FORMS: return "UPDATE_SHAPESHIFT_FORMS";
            case Events::WHO_LIST_UPDATE: return "WHO_LIST_UPDATE";
            case Events::UPDATE_LFG: return "UPDATE_LFG";
            case Events::PETITION_SHOW: return "PETITION_SHOW";
            case Events::PETITION_CLOSED: return "PETITION_CLOSED";
            case Events::EXECUTE_CHAT_LINE: return "EXECUTE_CHAT_LINE";
            case Events::UPDATE_MACROS: return "UPDATE_MACROS";
            case Events::UPDATE_TICKET: return "UPDATE_TICKET";
            case Events::UPDATE_CHAT_WINDOWS: return "UPDATE_CHAT_WINDOWS";
            case Events::CONFIRM_XP_LOSS: return "CONFIRM_XP_LOSS";
            case Events::CORPSE_IN_RANGE: return "CORPSE_IN_RANGE";
            case Events::CORPSE_IN_INSTANCE: return "CORPSE_IN_INSTANCE";
            case Events::CORPSE_OUT_OF_RANGE: return "CORPSE_OUT_OF_RANGE";
            case Events::UPDATE_GM_STATUS: return "UPDATE_GM_STATUS";
            case Events::PLAYER_UNGHOST: return "PLAYER_UNGHOST";
            case Events::BIND_ENCHANT: return "BIND_ENCHANT";
            case Events::REPLACE_ENCHANT: return "REPLACE_ENCHANT";
            case Events::TRADE_REPLACE_ENCHANT: return "TRADE_REPLACE_ENCHANT";
            case Events::PLAYER_UPDATE_RESTING: return "PLAYER_UPDATE_RESTING";
            case Events::UPDATE_EXHAUSTION: return "UPDATE_EXHAUSTION";
            case Events::PLAYER_FLAGS_CHANGED: return "PLAYER_FLAGS_CHANGED";
            case Events::GUILD_ROSTER_UPDATE: return "GUILD_ROSTER_UPDATE";
            case Events::GM_PLAYER_INFO: return "GM_PLAYER_INFO";
            case Events::MAIL_SHOW: return "MAIL_SHOW";
            case Events::MAIL_CLOSED: return "MAIL_CLOSED";
            case Events::SEND_MAIL_MONEY_CHANGED: return "SEND_MAIL_MONEY_CHANGED";
            case Events::SEND_MAIL_COD_CHANGED: return "SEND_MAIL_COD_CHANGED";
            case Events::MAIL_SEND_INFO_UPDATE: return "MAIL_SEND_INFO_UPDATE";
            case Events::MAIL_SEND_SUCCESS: return "MAIL_SEND_SUCCESS";
            case Events::MAIL_INBOX_UPDATE: return "MAIL_INBOX_UPDATE";
            case Events::BATTLEFIELDS_SHOW: return "BATTLEFIELDS_SHOW";
            case Events::BATTLEFIELDS_CLOSED: return "BATTLEFIELDS_CLOSED";
            case Events::UPDATE_BATTLEFIELD_STATUS: return "UPDATE_BATTLEFIELD_STATUS";
            case Events::UPDATE_BATTLEFIELD_SCORE: return "UPDATE_BATTLEFIELD_SCORE";
            case Events::AUCTION_HOUSE_SHOW: return "AUCTION_HOUSE_SHOW";
            case Events::AUCTION_HOUSE_CLOSED: return "AUCTION_HOUSE_CLOSED";
            case Events::NEW_AUCTION_UPDATE: return "NEW_AUCTION_UPDATE";
            case Events::AUCTION_ITEM_LIST_UPDATE: return "AUCTION_ITEM_LIST_UPDATE";
            case Events::AUCTION_OWNED_LIST_UPDATE: return "AUCTION_OWNED_LIST_UPDATE";
            case Events::AUCTION_BIDDER_LIST_UPDATE: return "AUCTION_BIDDER_LIST_UPDATE";
            case Events::PET_UI_UPDATE: return "PET_UI_UPDATE";
            case Events::PET_UI_CLOSE: return "PET_UI_CLOSE";
            case Events::ADDON_LOADED: return "ADDON_LOADED";
            case Events::VARIABLES_LOADED: return "VARIABLES_LOADED";
            case Events::MACRO_ACTION_FORBIDDEN: return "MACRO_ACTION_FORBIDDEN";
            case Events::ADDON_ACTION_FORBIDDEN: return "ADDON_ACTION_FORBIDDEN";
            case Events::MEMORY_EXHAUSTED: return "MEMORY_EXHAUSTED";
            case Events::MEMORY_RECOVERED: return "MEMORY_RECOVERED";
            case Events::START_AUTOREPEAT_SPELL: return "START_AUTOREPEAT_SPELL";
            case Events::STOP_AUTOREPEAT_SPELL: return "STOP_AUTOREPEAT_SPELL";
            case Events::PET_STABLE_SHOW: return "PET_STABLE_SHOW";
            case Events::PET_STABLE_UPDATE: return "PET_STABLE_UPDATE";
            case Events::PET_STABLE_UPDATE_PAPERDOLL: return "PET_STABLE_UPDATE_PAPERDOLL";
            case Events::PET_STABLE_CLOSED: return "PET_STABLE_CLOSED";
            case Events::CHAT_MSG_COMBAT_SELF_HITS: return "CHAT_MSG_COMBAT_SELF_HITS";
            case Events::CHAT_MSG_COMBAT_SELF_MISSES: return "CHAT_MSG_COMBAT_SELF_MISSES";
            case Events::CHAT_MSG_COMBAT_PET_HITS: return "CHAT_MSG_COMBAT_PET_HITS";
            case Events::CHAT_MSG_COMBAT_PET_MISSES: return "CHAT_MSG_COMBAT_PET_MISSES";
            case Events::CHAT_MSG_COMBAT_PARTY_HITS: return "CHAT_MSG_COMBAT_PARTY_HITS";
            case Events::CHAT_MSG_COMBAT_PARTY_MISSES: return "CHAT_MSG_COMBAT_PARTY_MISSES";
            case Events::CHAT_MSG_COMBAT_FRIENDLYPLAYER_HITS: return "CHAT_MSG_COMBAT_FRIENDLYPLAYER_HITS";
            case Events::CHAT_MSG_COMBAT_FRIENDLYPLAYER_MISSES: return "CHAT_MSG_COMBAT_FRIENDLYPLAYER_MISSES";
            case Events::CHAT_MSG_COMBAT_HOSTILEPLAYER_HITS: return "CHAT_MSG_COMBAT_HOSTILEPLAYER_HITS";
            case Events::CHAT_MSG_COMBAT_HOSTILEPLAYER_MISSES: return "CHAT_MSG_COMBAT_HOSTILEPLAYER_MISSES";
            case Events::CHAT_MSG_COMBAT_CREATURE_VS_SELF_HITS: return "CHAT_MSG_COMBAT_CREATURE_VS_SELF_HITS";
            case Events::CHAT_MSG_COMBAT_CREATURE_VS_SELF_MISSES: return "CHAT_MSG_COMBAT_CREATURE_VS_SELF_MISSES";
            case Events::CHAT_MSG_COMBAT_CREATURE_VS_PARTY_HITS: return "CHAT_MSG_COMBAT_CREATURE_VS_PARTY_HITS";
            case Events::CHAT_MSG_COMBAT_CREATURE_VS_PARTY_MISSES: return "CHAT_MSG_COMBAT_CREATURE_VS_PARTY_MISSES";
            case Events::CHAT_MSG_COMBAT_CREATURE_VS_CREATURE_HITS: return "CHAT_MSG_COMBAT_CREATURE_VS_CREATURE_HITS";
            case Events::CHAT_MSG_COMBAT_CREATURE_VS_CREATURE_MISSES: return "CHAT_MSG_COMBAT_CREATURE_VS_CREATURE_MISSES";
            case Events::CHAT_MSG_COMBAT_FRIENDLY_DEATH: return "CHAT_MSG_COMBAT_FRIENDLY_DEATH";
            case Events::CHAT_MSG_COMBAT_HOSTILE_DEATH: return "CHAT_MSG_COMBAT_HOSTILE_DEATH";
            case Events::CHAT_MSG_COMBAT_XP_GAIN: return "CHAT_MSG_COMBAT_XP_GAIN";
            case Events::CHAT_MSG_COMBAT_HONOR_GAIN: return "CHAT_MSG_COMBAT_HONOR_GAIN";
            case Events::CHAT_MSG_SPELL_SELF_DAMAGE: return "CHAT_MSG_SPELL_SELF_DAMAGE";
            case Events::CHAT_MSG_SPELL_SELF_BUFF: return "CHAT_MSG_SPELL_SELF_BUFF";
            case Events::CHAT_MSG_SPELL_PET_DAMAGE: return "CHAT_MSG_SPELL_PET_DAMAGE";
            case Events::CHAT_MSG_SPELL_PET_BUFF: return "CHAT_MSG_SPELL_PET_BUFF";
            case Events::CHAT_MSG_SPELL_PARTY_DAMAGE: return "CHAT_MSG_SPELL_PARTY_DAMAGE";
            case Events::CHAT_MSG_SPELL_PARTY_BUFF: return "CHAT_MSG_SPELL_PARTY_BUFF";
            case Events::CHAT_MSG_SPELL_FRIENDLYPLAYER_DAMAGE: return "CHAT_MSG_SPELL_FRIENDLYPLAYER_DAMAGE";
            case Events::CHAT_MSG_SPELL_FRIENDLYPLAYER_BUFF: return "CHAT_MSG_SPELL_FRIENDLYPLAYER_BUFF";
            case Events::CHAT_MSG_SPELL_HOSTILEPLAYER_DAMAGE: return "CHAT_MSG_SPELL_HOSTILEPLAYER_DAMAGE";
            case Events::CHAT_MSG_SPELL_HOSTILEPLAYER_BUFF: return "CHAT_MSG_SPELL_HOSTILEPLAYER_BUFF";
            case Events::CHAT_MSG_SPELL_CREATURE_VS_SELF_DAMAGE: return "CHAT_MSG_SPELL_CREATURE_VS_SELF_DAMAGE";
            case Events::CHAT_MSG_SPELL_CREATURE_VS_SELF_BUFF: return "CHAT_MSG_SPELL_CREATURE_VS_SELF_BUFF";
            case Events::CHAT_MSG_SPELL_CREATURE_VS_PARTY_DAMAGE: return "CHAT_MSG_SPELL_CREATURE_VS_PARTY_DAMAGE";
            case Events::CHAT_MSG_SPELL_CREATURE_VS_PARTY_BUFF: return "CHAT_MSG_SPELL_CREATURE_VS_PARTY_BUFF";
            case Events::CHAT_MSG_SPELL_CREATURE_VS_CREATURE_DAMAGE: return "CHAT_MSG_SPELL_CREATURE_VS_CREATURE_DAMAGE";
            case Events::CHAT_MSG_SPELL_CREATURE_VS_CREATURE_BUFF: return "CHAT_MSG_SPELL_CREATURE_VS_CREATURE_BUFF";
            case Events::CHAT_MSG_SPELL_TRADESKILLS: return "CHAT_MSG_SPELL_TRADESKILLS";
            case Events::CHAT_MSG_SPELL_DAMAGESHIELDS_ON_SELF: return "CHAT_MSG_SPELL_DAMAGESHIELDS_ON_SELF";
            case Events::CHAT_MSG_SPELL_DAMAGESHIELDS_ON_OTHERS: return "CHAT_MSG_SPELL_DAMAGESHIELDS_ON_OTHERS";
            case Events::CHAT_MSG_SPELL_AURA_GONE_SELF: return "CHAT_MSG_SPELL_AURA_GONE_SELF";
            case Events::CHAT_MSG_SPELL_AURA_GONE_PARTY: return "CHAT_MSG_SPELL_AURA_GONE_PARTY";
            case Events::CHAT_MSG_SPELL_AURA_GONE_OTHER: return "CHAT_MSG_SPELL_AURA_GONE_OTHER";
            case Events::CHAT_MSG_SPELL_ITEM_ENCHANTMENTS: return "CHAT_MSG_SPELL_ITEM_ENCHANTMENTS";
            case Events::CHAT_MSG_SPELL_BREAK_AURA: return "CHAT_MSG_SPELL_BREAK_AURA";
            case Events::CHAT_MSG_SPELL_PERIODIC_SELF_DAMAGE: return "CHAT_MSG_SPELL_PERIODIC_SELF_DAMAGE";
            case Events::CHAT_MSG_SPELL_PERIODIC_SELF_BUFFS: return "CHAT_MSG_SPELL_PERIODIC_SELF_BUFFS";
            case Events::CHAT_MSG_SPELL_PERIODIC_PARTY_DAMAGE: return "CHAT_MSG_SPELL_PERIODIC_PARTY_DAMAGE";
            case Events::CHAT_MSG_SPELL_PERIODIC_PARTY_BUFFS: return "CHAT_MSG_SPELL_PERIODIC_PARTY_BUFFS";
            case Events::CHAT_MSG_SPELL_PERIODIC_FRIENDLYPLAYER_DAMAGE: return "CHAT_MSG_SPELL_PERIODIC_FRIENDLYPLAYER_DAMAGE";
            case Events::CHAT_MSG_SPELL_PERIODIC_FRIENDLYPLAYER_BUFFS: return "CHAT_MSG_SPELL_PERIODIC_FRIENDLYPLAYER_BUFFS";
            case Events::CHAT_MSG_SPELL_PERIODIC_HOSTILEPLAYER_DAMAGE: return "CHAT_MSG_SPELL_PERIODIC_HOSTILEPLAYER_DAMAGE";
            case Events::CHAT_MSG_SPELL_PERIODIC_HOSTILEPLAYER_BUFFS: return "CHAT_MSG_SPELL_PERIODIC_HOSTILEPLAYER_BUFFS";
            case Events::CHAT_MSG_SPELL_PERIODIC_CREATURE_DAMAGE: return "CHAT_MSG_SPELL_PERIODIC_CREATURE_DAMAGE";
            case Events::CHAT_MSG_SPELL_PERIODIC_CREATURE_BUFFS: return "CHAT_MSG_SPELL_PERIODIC_CREATURE_BUFFS";
            case Events::CHAT_MSG_SPELL_FAILED_LOCALPLAYER: return "CHAT_MSG_SPELL_FAILED_LOCALPLAYER";
            case Events::CHAT_MSG_BG_SYSTEM_NEUTRAL: return "CHAT_MSG_BG_SYSTEM_NEUTRAL";
            case Events::CHAT_MSG_BG_SYSTEM_ALLIANCE: return "CHAT_MSG_BG_SYSTEM_ALLIANCE";
            case Events::CHAT_MSG_BG_SYSTEM_HORDE: return "CHAT_MSG_BG_SYSTEM_HORDE";
            case Events::RAID_ROSTER_UPDATE: return "RAID_ROSTER_UPDATE";
            case Events::UPDATE_PENDING_MAIL: return "UPDATE_PENDING_MAIL";
            case Events::UPDATE_INVENTORY_ALERTS: return "UPDATE_INVENTORY_ALERTS";
            case Events::UPDATE_TRADESKILL_RECAST: return "UPDATE_TRADESKILL_RECAST";
            case Events::OPEN_MASTER_LOOT_LIST: return "OPEN_MASTER_LOOT_LIST";
            case Events::UPDATE_MASTER_LOOT_LIST: return "UPDATE_MASTER_LOOT_LIST";
            case Events::START_LOOT_ROLL: return "START_LOOT_ROLL";
            case Events::CANCEL_LOOT_ROLL: return "CANCEL_LOOT_ROLL";
            case Events::CONFIRM_LOOT_ROLL: return "CONFIRM_LOOT_ROLL";
            case Events::INSTANCE_BOOT_START: return "INSTANCE_BOOT_START";
            case Events::INSTANCE_BOOT_STOP: return "INSTANCE_BOOT_STOP";
            case Events::LEARNED_SPELL_IN_TAB: return "LEARNED_SPELL_IN_TAB";
            case Events::DISPLAY_SIZE_CHANGED: return "DISPLAY_SIZE_CHANGED";
            case Events::CONFIRM_TALENT_WIPE: return "CONFIRM_TALENT_WIPE";
            case Events::CONFIRM_BINDER: return "CONFIRM_BINDER";
            case Events::MAIL_FAILED: return "MAIL_FAILED";
            case Events::CLOSE_INBOX_ITEM: return "CLOSE_INBOX_ITEM";
            case Events::CONFIRM_SUMMON: return "CONFIRM_SUMMON";
            case Events::BILLING_NAG_DIALOG: return "BILLING_NAG_DIALOG";
            case Events::IGR_BILLING_NAG_DIALOG: return "IGR_BILLING_NAG_DIALOG";
            case Events::MEETINGSTONE_CHANGED: return "MEETINGSTONE_CHANGED";
            case Events::PLAYER_SKINNED: return "PLAYER_SKINNED";
            case Events::TABARD_SAVE_PENDING: return "TABARD_SAVE_PENDING";
            case Events::UNIT_QUEST_LOG_CHANGED: return "UNIT_QUEST_LOG_CHANGED";
            case Events::PLAYER_PVP_KILLS_CHANGED: return "PLAYER_PVP_KILLS_CHANGED";
            case Events::PLAYER_PVP_RANK_CHANGED: return "PLAYER_PVP_RANK_CHANGED";
            case Events::INSPECT_HONOR_UPDATE: return "INSPECT_HONOR_UPDATE";
            case Events::UPDATE_WORLD_STATES: return "UPDATE_WORLD_STATES";
            case Events::AREA_SPIRIT_HEALER_IN_RANGE: return "AREA_SPIRIT_HEALER_IN_RANGE";
            case Events::AREA_SPIRIT_HEALER_OUT_OF_RANGE: return "AREA_SPIRIT_HEALER_OUT_OF_RANGE";
            case Events::CONFIRM_PET_UNLEARN: return "CONFIRM_PET_UNLEARN";
            case Events::PLAYTIME_CHANGED: return "PLAYTIME_CHANGED";
            case Events::UPDATE_LFG_TYPES: return "UPDATE_LFG_TYPES";
            case Events::UPDATE_LFG_LIST: return "UPDATE_LFG_LIST";
            case Events::CHAT_MSG_COMBAT_FACTION_CHANGE: return "CHAT_MSG_COMBAT_FACTION_CHANGE";
            case Events::START_MINIGAME: return "START_MINIGAME";
            case Events::MINIGAME_UPDATE: return "MINIGAME_UPDATE";
            case Events::READY_CHECK: return "READY_CHECK";
            case Events::RAID_TARGET_UPDATE: return "RAID_TARGET_UPDATE";
            case Events::GMSURVEY_DISPLAY: return "GMSURVEY_DISPLAY";
            case Events::UPDATE_INSTANCE_INFO: return "UPDATE_INSTANCE_INFO";
            case Events::SPELL_CAST_EVENT: return "SPELL_CAST_EVENT";
            case Events::CHAT_MSG_RAID_BOSS_EMOTE: return "CHAT_MSG_RAID_BOSS_EMOTE";
            case Events::COMBAT_TEXT_UPDATE: return "COMBAT_TEXT_UPDATE";
            case Events::LOTTERY_SHOW: return "LOTTERY_SHOW";
            case Events::CHAT_MSG_FILTERED: return "CHAT_MSG_FILTERED";
            case Events::QUEST_WATCH_UPDATE: return "QUEST_WATCH_UPDATE";
            case Events::CHAT_MSG_BATTLEGROUND: return "CHAT_MSG_BATTLEGROUND";
            case Events::CHAT_MSG_BATTLEGROUND_LEADER: return "CHAT_MSG_BATTLEGROUND_LEADER";
            case Events::LOTTERY_ITEM_UPDATE: return "LOTTERY_ITEM_UPDATE";
            case Events::SPELL_DAMAGE_EVENT_SELF: return "SPELL_DAMAGE_EVENT_SELF";
            case Events::SPELL_DAMAGE_EVENT_OTHER: return "SPELL_DAMAGE_EVENT_OTHER";
            case Events::UNIT_CASTEVENT: return "UNIT_CASTEVENT";
            case Events::RAW_COMBATLOG: return "RAW_COMBATLOG";
            case Events::CREATE_CHATBUBBLE: return "CREATE_CHATBUBBLE";
            case Events::OTHER_UI_EVENTS: return "OTHER_UI_EVENTS";
            default: return "UNKNOWN_EVENT_" + std::to_string(eventCode);
        }
    }
}