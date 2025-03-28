/**
 * ermerchant_shop.cpp
 *
 * New shop params. This iterates through every obtainable item in the game, and creates a shop
 * param in the appropriate shop that allows buying it for free. The shop lookup functions are
 * hooked in order to return these modded params.
 */
#include "ermerchant_shops.hpp"

#include <array>
#include <set>
#include <vector>

#include "from/paramdef/EQUIP_PARAM_ACCESSORY_ST.hpp"
#include "from/paramdef/EQUIP_PARAM_GEM_ST.hpp"
#include "from/paramdef/EQUIP_PARAM_GOODS_ST.hpp"
#include "from/paramdef/EQUIP_PARAM_PROTECTOR_ST.hpp"
#include "from/paramdef/EQUIP_PARAM_WEAPON_ST.hpp"
#include "from/paramdef/ITEMLOT_PARAM_ST.hpp"
#include "from/paramdef/REINFORCE_PARAM_WEAPON_ST.hpp"
#include "from/paramdef/SHOP_LINEUP_PARAM.hpp"

#include "ermerchant_config.hpp"
#include "ermerchant_messages.hpp"
#include "from/game_data.hpp"
#include "from/param_lookup.hpp"
#include "from/params.hpp"
#include "modutils.hpp"

static const std::wstring cut_content_prefix = L"[ERROR]";

static constexpr unsigned char lot_item_category_goods = 1;

static constexpr unsigned char cost_type_lost_ashes_of_war = 4;

static constexpr unsigned char equip_type_weapon = 0;
static constexpr unsigned char equip_type_protector = 1;
static constexpr unsigned char equip_type_accessory = 2;
static constexpr unsigned char equip_type_goods = 3;
static constexpr unsigned char equip_type_gem = 4;

static constexpr unsigned char weapon_type_arrow = 81;
static constexpr unsigned char weapon_type_greatarrow = 83;
static constexpr unsigned char weapon_type_bolt = 85;
static constexpr unsigned char weapon_type_ballista_bolt = 86;

static constexpr long long weapon_unarmed_id = 110000;

static constexpr unsigned char protector_category_head = 0;
static constexpr unsigned char protector_category_chest = 1;
static constexpr unsigned char protector_category_arms = 2;
static constexpr unsigned char protector_category_legs = 3;

static constexpr long long protector_bare_head_id = 10000;
static constexpr long long protector_bare_chest_id = 10100;
static constexpr long long protector_bare_arms_id = 10200;
static constexpr long long protector_bare_legs_id = 10300;

static constexpr long long goods_golden_seed_id = 10010;
static constexpr long long goods_sacred_tear_id = 10020;

static constexpr unsigned char goods_type_normal_item = 0;
static constexpr unsigned char goods_type_key_item = 1;
static constexpr unsigned char goods_type_crafting_material = 2;
static constexpr unsigned char goods_type_remembrance = 3;
static constexpr unsigned char goods_type_sorcery = 5;
static constexpr unsigned char goods_type_spirit_summon_lesser = 7;
static constexpr unsigned char goods_type_spirit_summon_greater = 8;
static constexpr unsigned char goods_type_wondrous_physick = 9;
static constexpr unsigned char goods_type_wondrous_physick_tear = 10;
static constexpr unsigned char goods_type_regenerative_material = 11;
static constexpr unsigned char goods_type_info_item = 12;
static constexpr unsigned char goods_type_reinforcement_material = 14;
static constexpr unsigned char goods_type_great_rune = 15;
static constexpr unsigned char goods_type_incantation = 16;
static constexpr unsigned char goods_type_self_buff_sorcery = 17;
static constexpr unsigned char goods_type_self_buff_incantation = 18;
// Convergence has a unique goods type for consumable runes and remembrances
static constexpr unsigned char goods_type_convergence_rune = 100;
static constexpr unsigned char goods_type_convergence_remembrance = 101;

static constexpr unsigned char goods_sort_group_tutorial = 20;
static constexpr unsigned char goods_sort_group_gesture = 250;

/*
 * Items that are automatically given to the player in certain scenarios, and should not be
 * purchasable
 */
static const std::set<long long> excluded_goods = {
    107, // Phantom Bloody Finger
    113, // Phantom Bloody Finger
    114, // Phantom Recusant Finger
    115, // Memory of Grace
    135, // Phantom Great Rune
    251, // Flask of Wondrous Physick (empty)
    // Flask of Crimson Tears (+0 through +12, empty and full)
    1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009, 1010, 1011, 1012, 1013, 1014, 1015,
    1016, 1017, 1018, 1019, 1020, 1021, 1022, 1023, 1024, 1025,
    // Flask of Cerulean Tears (+0 through +12, empty and full)
    1050, 1051, 1052, 1053, 1054, 1055, 1056, 1057, 1058, 1059, 1060, 1061, 1062, 1063, 1064, 1065,
    1066, 1067, 1068, 1069, 1070, 1071, 1072, 1073, 1074, 1075};

/* Additional items that don't have the unauthorized "[ERROR]" prefix, but are unobtainable */
static const std::set<long long> cut_content_goods = {
    8860,    // Erdtree Prayerbook
    8861,    // Erdtree Codex
    2008023, // Keep Wall Key
};

static const std::set<long long> cut_content_protectors = {
    610000, // Ragged Hat
    610100, // Ragged Armor
    610200, // Ragged Gloves
    610300, // Ragged Loincloth
    611000, // Ragged Hat (Altered)
    611100, // Ragged Armor (Altered)
};

static constexpr unsigned int kale_alive_flag_id = 4700;
static constexpr unsigned int kale_hostile_flag_id = 4701;
static constexpr unsigned int kale_dead_flag_id = 4703;

static from::CS::GameDataMan **game_data_man_addr;

struct shop
{
    long long id;
    std::vector<from::paramdef::SHOP_LINEUP_PARAM> lineups;
};

static std::array<shop, 22> mod_shops = {
    shop{.id = ermerchant::shops::weapons},
    shop{.id = ermerchant::shops::armor},
    shop{.id = ermerchant::shops::spells},
    shop{.id = ermerchant::shops::talismans},
    shop{.id = ermerchant::shops::ammunition},
    shop{.id = ermerchant::shops::ashes_of_war},
    shop{.id = ermerchant::shops::spirit_summons},
    shop{.id = ermerchant::shops::consumables},
    shop{.id = ermerchant::shops::materials},
    shop{.id = ermerchant::shops::miscellaneous_items},
    shop{.id = ermerchant::shops::cut_goods},
    shop{.id = ermerchant::shops::cut_armor},
    shop{.id = ermerchant::shops::dlc_weapons},
    shop{.id = ermerchant::shops::dlc_armor},
    shop{.id = ermerchant::shops::dlc_spells},
    shop{.id = ermerchant::shops::dlc_talismans},
    shop{.id = ermerchant::shops::dlc_ammunition},
    shop{.id = ermerchant::shops::dlc_ashes_of_war},
    shop{.id = ermerchant::shops::dlc_spirit_summons},
    shop{.id = ermerchant::shops::dlc_consumables},
    shop{.id = ermerchant::shops::dlc_materials},
    shop{.id = ermerchant::shops::dlc_miscellaneous_items},
};

// Map of ReinforceParamWeapon IDs to the maximum possible level of weapons using that upgrade path
static std::map<short, unsigned char> max_level_by_reinforce_type_id;

// Goods that shouldn't be allowed in the storage box, because acquiring a second copy can break
// things
static std::set<unsigned int> no_repository_item_ids;

static bool is_shop_open = false;

static shop *get_mod_shop(int shop_lineup_id)
{
    for (auto &shop : mod_shops)
    {
        if (shop_lineup_id >= shop.id && shop_lineup_id < shop.id + ermerchant::shop_capacity)
        {
            return &shop;
        }
    }

    return nullptr;
}

static from::find_shop_menu_result *(*solo_param_repository_lookup_shop_menu)(
    from::find_shop_menu_result *result, unsigned char shop_type, int begin_id, int end_id);

/**
 * Hook for SoloParamRepositoryImp::LookupShopMenu()
 *
 * Return a shop menu for the Glorious Merchant shop, or fall back to the vanilla shop menus.
 */
static from::find_shop_menu_result *solo_param_repository_lookup_shop_menu_detour(
    from::find_shop_menu_result *result, unsigned char shop_type, int begin_id, int end_id)
{
    for (auto &shop : mod_shops)
    {
        if (begin_id == shop.id)
        {
            result->shop_type = shop_type;
            result->id = begin_id;
            result->row = &shop.lineups[0];
            return result;
        }
    }

    return solo_param_repository_lookup_shop_menu(result, shop_type, begin_id, end_id);
}

static void (*solo_param_repository_lookup_shop_lineup)(from::find_shop_menu_result *,
                                                        unsigned char, int id);

/**
 * Hook for SoloParamRepositoryImp::LookupShopLineup()
 *
 * Return a shop lineup added by the mod to buy an item for $0, or fall back to the vanilla shop
 * lineups.
 */
static void solo_param_repository_lookup_shop_lineup_detour(from::find_shop_menu_result *result,
                                                            unsigned char shop_type, int id)
{
    auto shop = get_mod_shop(id);
    if (shop && id < shop->id + shop->lineups.size())
    {
        result->shop_type = shop_type;
        result->id = id;
        result->row = &shop->lineups[id - shop->id];
        return;
    }

    solo_param_repository_lookup_shop_lineup(result, shop_type, id);
}

static void (*open_regular_shop)(void *, long long, long long);

/**
 * Hook for OpenRegularShop()
 */
static void open_regular_shop_detour(void *unk, long long begin_id, long long end_id)
{
    auto shop = get_mod_shop(begin_id);

    // Change the upgrade level when purchasing weapons to the player's current max
    if (ermerchant::config::auto_upgrade_weapons && shop &&
        (shop->id == ermerchant::shops::weapons || shop->id == ermerchant::shops::dlc_weapons))
    {
        auto max_reinforce_level = (*game_data_man_addr)->player_game_data->max_reinforce_level;

        auto equip_param_weapon =
            from::params::get_param<from::paramdef::EQUIP_PARAM_WEAPON_ST>(L"EquipParamWeapon");

        for (auto &lineup : shop->lineups)
        {
            if (lineup.equipType == equip_type_weapon)
            {
                auto weapon_id = lineup.equipId - lineup.equipId % 100;
                auto weapon = equip_param_weapon[weapon_id];

                auto reinforce_level =
                    (int)std::floor((max_reinforce_level + 0.5) *
                                    max_level_by_reinforce_type_id[weapon.reinforceTypeId] / 25);

                lineup.equipId = weapon_id + reinforce_level;
            }
        }
    }

    open_regular_shop(unk, begin_id, end_id);

    if (shop)
    {
        ermerchant::set_shop_open(true);

        // Change the default sort order when opening one of the shops added by this mod.
        (*game_data_man_addr)->menu_system_save_load->sorts[from::sort_index_all_items] =
            from::menu_sort::item_type_ascending;
    }
}

static int (*get_sell_value)(unsigned int *);

/**
 * Hook for GetSellValue()
 *
 * Overrides the sell value for all items to 0 while the merchant shop is open. We don't directly
 * touch the param fields because this would prevent items from being sold back to merchants, and
 * because Seamless Co-op doesn't allow matchmaking with modified params.
 */
static int get_sell_value_detour(unsigned int *item_id)
{
    if (is_shop_open)
    {
        return 0;
    }

    return get_sell_value(item_id);
}

static unsigned long long (*get_max_repository_num)(unsigned int *);

/**
 * Hook for GetMaxRepositoryNum()
 *
 * Overrides certain items to prevent them from being placed in the storage box, because they
 * should be limited to 1 total purchased to avoid breaking certain flags.
 */
static unsigned long long get_max_repository_num_detour(unsigned int *item_id)
{
    if (is_shop_open && no_repository_item_ids.contains(*item_id))
    {
        return 0;
    }

    return get_max_repository_num(item_id);
}

static unsigned int (*get_event_flag)(void *, unsigned int);

/**
 * Hook for CS::CSFD4VirtualMemoryFlag::GetEventFlag()
 *
 * Make Kalé always alive and non-hostile
 */
static unsigned int get_event_flag_detour(void *self, unsigned int flag_id)
{
    switch (flag_id)
    {
    case kale_alive_flag_id:
        return 1;
    case kale_hostile_flag_id:
        return 0;
    case kale_dead_flag_id:
        return 0;
    default:
        return get_event_flag(self, flag_id);
    }
}

void ermerchant::setup_shops()
{
    auto &weapon_lineups = mod_shops[0].lineups;
    auto &armor_lineups = mod_shops[1].lineups;
    auto &spell_lineups = mod_shops[2].lineups;
    auto &talisman_lineups = mod_shops[3].lineups;
    auto &ammunition_lineups = mod_shops[4].lineups;
    auto &ash_of_war_lineups = mod_shops[5].lineups;
    auto &spirit_summon_lineups = mod_shops[6].lineups;
    auto &consumable_lineups = mod_shops[7].lineups;
    auto &material_lineups = mod_shops[8].lineups;
    auto &miscellaneous_item_lineups = mod_shops[9].lineups;
    auto &cut_good_lineups = mod_shops[10].lineups;
    auto &cut_armor_lineups = mod_shops[11].lineups;
    auto &dlc_weapon_lineups = mod_shops[12].lineups;
    auto &dlc_armor_lineups = mod_shops[13].lineups;
    auto &dlc_spell_lineups = mod_shops[14].lineups;
    auto &dlc_talisman_lineups = mod_shops[15].lineups;
    auto &dlc_ammunition_lineups = mod_shops[16].lineups;
    auto &dlc_ashes_of_war_lineups = mod_shops[17].lineups;
    auto &dlc_spirit_summon_lineups = mod_shops[18].lineups;
    auto &dlc_consumable_lineups = mod_shops[19].lineups;
    auto &dlc_material_lineups = mod_shops[20].lineups;
    auto &dlc_miscellaneous_item_lineups = mod_shops[21].lineups;

    // Look up event flags set when acquiring items like maps and cookbooks. Simply possessing
    // these items doesn't actually unlock anything, an event flag must also be set.
    std::map<int, unsigned int> goods_flags;
    std::map<int, unsigned int> gems_flags;

    // Look up goods IDs that are only used for replacement text in shops. These aren't actual
    // obtainable items.
    std::set<long long> dummy_goods_ids;

    for (auto param : {L"ItemLotParam_map", L"ItemLotParam_enemy"})
    {
        for (auto [id, row] : from::params::get_param<from::paramdef::ITEMLOT_PARAM_ST>(param))
        {
            // Record flags set when looting goods
            if (row.lotItemCategory01 == lot_item_category_goods && row.getItemFlagId > 0)
            {
                goods_flags[row.lotItemId01] = row.getItemFlagId;
            }
        }
    }
    for (auto [id, row] :
         from::params::get_param<from::paramdef::SHOP_LINEUP_PARAM>(L"ShopLineupParam"))
    {
        // Record flags set when purchasing goods
        if (row.equipType == equip_type_goods)
        {
            goods_flags[row.equipId] = row.eventFlag_forStock;
        }
        // Record flags required for Hewg to duplicate AoWs
        else if (row.equipType == equip_type_gem && row.costType == cost_type_lost_ashes_of_war)
        {
            gems_flags[row.equipId] = row.eventFlag_forRelease;
        }
        // Record goods IDs that are used for replacement text in shops
        if (row.nameMsgId != -1)
        {
            dummy_goods_ids.insert(row.nameMsgId);
        }
    }

    for (auto [id, row] :
         from::params::get_param<from::paramdef::EQUIP_PARAM_GOODS_ST>(L"EquipParamGoods"))
    {
        if (row.appearanceReplaceItemId != -1)
        {
            dummy_goods_ids.insert(row.appearanceReplaceItemId);
        }
    }

    // Count of the number of times each flag prefix is used, to help identify flags that are usable
    // as stock counters.
    std::map<unsigned int, int> goods_flag_counts;
    for (auto [_, flag] : goods_flags)
        goods_flag_counts[flag - flag % 10]++;

    // Iterate through every obtainable item in the game and create shop lineups in the appropriate
    // ranges
    for (auto [id, row] :
         from::params::get_param<from::paramdef::EQUIP_PARAM_WEAPON_ST>(L"EquipParamWeapon"))
    {
        // Exclude unarmed fist
        if (id == weapon_unarmed_id)
        {
            continue;
        }

        // Exclude duplicate weapon entries for heavy, keen, etc.
        auto affinity_id = (id % 10000) / 100;
        if (affinity_id != 0)
        {
            continue;
        }

        bool is_dlc = false;
        auto weapon_name = ermerchant::get_message(from::msgbnd::weapon_name, id);
        if (weapon_name.empty())
        {
            is_dlc = true;
            weapon_name = ermerchant::get_message(from::msgbnd::dlc_weapon_name, id);
        }

        // Exclude weapon entries without valid names - these are placeholders for data used by
        // non-weapons (e.g. perfumes) or unused/cut items.
        if (weapon_name.empty() || weapon_name.starts_with(cut_content_prefix))
        {
            continue;
        }

        std::vector<from::paramdef::SHOP_LINEUP_PARAM> *lineups = nullptr;

        if (row.wepType == weapon_type_arrow || row.wepType == weapon_type_greatarrow ||
            row.wepType == weapon_type_bolt || row.wepType == weapon_type_ballista_bolt)
        {

            if (is_dlc)
            {
                lineups = &dlc_ammunition_lineups;
            }
            else
            {
                lineups = &ammunition_lineups;
            }
        }
        else if (is_dlc)
        {
            lineups = &dlc_weapon_lineups;
        }
        else
        {
            lineups = &weapon_lineups;
        }

        lineups->push_back({.equipId = (int)id, .equipType = equip_type_weapon});
    }

    for (auto [id, row] :
         from::params::get_param<from::paramdef::EQUIP_PARAM_PROTECTOR_ST>(L"EquipParamProtector"))
    {
        // Exclude bare unarmored head/chest/etc.
        if (id == protector_bare_head_id || id == protector_bare_chest_id ||
            id == protector_bare_arms_id || id == protector_bare_legs_id)
        {
            continue;
        }

        // Exclude protector entries other than armor (e.g. hair)
        if (row.protectorCategory != protector_category_head &&
            row.protectorCategory != protector_category_chest &&
            row.protectorCategory != protector_category_arms &&
            row.protectorCategory != protector_category_legs)
        {
            continue;
        }

        bool is_dlc = false;
        auto protector_name = ermerchant::get_message(from::msgbnd::protector_name, id);
        if (protector_name.empty())
        {
            is_dlc = true;
            protector_name = ermerchant::get_message(from::msgbnd::dlc_protector_name, id);
        }

        if (protector_name.empty() || protector_name == cut_content_prefix)
        {
            continue;
        }

        std::vector<from::paramdef::SHOP_LINEUP_PARAM> *lineups = nullptr;

        if (protector_name.starts_with(cut_content_prefix) || cut_content_protectors.contains(id))
        {
            lineups = &cut_armor_lineups;
        }
        else if (is_dlc)
        {
            lineups = &dlc_armor_lineups;
        }
        else
        {
            lineups = &armor_lineups;
        }

        lineups->push_back({.equipId = (int)id, .equipType = equip_type_protector});
    }

    for (auto [id, row] :
         from::params::get_param<from::paramdef::EQUIP_PARAM_ACCESSORY_ST>(L"EquipParamAccessory"))
    {
        bool is_dlc = false;
        auto accessory_name = ermerchant::get_message(from::msgbnd::accessory_name, id);
        if (accessory_name.empty())
        {
            is_dlc = true;
            accessory_name = ermerchant::get_message(from::msgbnd::dlc_accessory_name, id);
        }

        if (accessory_name.empty() || accessory_name.starts_with(cut_content_prefix))
        {
            continue;
        }

        std::vector<from::paramdef::SHOP_LINEUP_PARAM> *lineups = nullptr;

        if (is_dlc)
        {
            lineups = &dlc_talisman_lineups;
        }
        else
        {
            lineups = &talisman_lineups;
        }

        lineups->push_back({.equipId = (int)id, .equipType = equip_type_accessory});
    }

    for (auto [id, row] :
         from::params::get_param<from::paramdef::EQUIP_PARAM_GOODS_ST>(L"EquipParamGoods"))
    {
        // Exclude goods which are obtained automatically in some way
        if (excluded_goods.contains(id))
        {
            continue;
        }

        // Exclude gestures, which are technically goods but are unlocked in a different way
        if (row.goodsType == goods_type_normal_item && row.sortGroupId == goods_sort_group_gesture)
        {
            continue;
        }

        // Exclude tutorials, which are also goods but aren't useful to buy
        if (row.goodsType == goods_type_info_item && row.sortGroupId == goods_sort_group_tutorial)
        {
            continue;
        }

        // Exclude goods entries that are just used to replace the icon of another entry or a shop
        // name or description
        if (dummy_goods_ids.contains(id))
        {
            continue;
        }

        bool is_dlc = false;
        auto goods_name = ermerchant::get_message(from::msgbnd::goods_name, id);
        if (goods_name.empty())
        {
            is_dlc = true;
            goods_name = ermerchant::get_message(from::msgbnd::dlc_goods_name, id);
        }

        if (goods_name.empty() || goods_name == cut_content_prefix)
        {
            continue;
        }

        std::vector<from::paramdef::SHOP_LINEUP_PARAM> *lineups = nullptr;

        if (goods_name.starts_with(cut_content_prefix) || !row.iconId ||
            cut_content_goods.contains(id))
        {
            // Put cut items in a separate shop
            lineups = &cut_good_lineups;
        }
        else if (id == goods_golden_seed_id || id == goods_sacred_tear_id)
        {
            // These are classified as materials, but should really appear in the consumables shop
            lineups = is_dlc ? &dlc_consumable_lineups : &consumable_lineups;
        }
        else
        {
            switch (row.goodsType)
            {
            case goods_type_normal_item:
                if (row.isConsume && !row.disable_offline)
                {
                    lineups = is_dlc ? &dlc_consumable_lineups : &consumable_lineups;
                }
                else
                {
                    lineups =
                        is_dlc ? &dlc_miscellaneous_item_lineups : &miscellaneous_item_lineups;
                }
                break;

            case goods_type_sorcery:
            case goods_type_incantation:
            case goods_type_self_buff_sorcery:
            case goods_type_self_buff_incantation:
                lineups = is_dlc ? &dlc_spell_lineups : &spell_lineups;
                break;

            case goods_type_spirit_summon_lesser:
            case goods_type_spirit_summon_greater: {
                // Exclude duplicate entries for upgraded spirit ashes
                auto upgrade_level = id % 100;
                if (upgrade_level == 0)
                {
                    lineups = is_dlc ? &dlc_spirit_summon_lineups : &spirit_summon_lineups;
                }
                break;
            }

            case goods_type_remembrance:
            case goods_type_regenerative_material:
            case goods_type_convergence_rune:
            case goods_type_convergence_remembrance:
                lineups = is_dlc ? &dlc_consumable_lineups : &consumable_lineups;
                break;

            case goods_type_crafting_material:
            case goods_type_reinforcement_material:
                lineups = is_dlc ? &dlc_material_lineups : &material_lineups;
                break;

            case goods_type_key_item:
            case goods_type_info_item:
            case goods_type_wondrous_physick:
            case goods_type_wondrous_physick_tear:
            case goods_type_great_rune:
                lineups = is_dlc ? &dlc_miscellaneous_item_lineups : &miscellaneous_item_lineups;
                break;
            }
        }

        if (lineups)
        {
            auto event_flag = goods_flags[id];
            short sell_quantity = -1;

            // Check for maps, crafting kit, key items, etc. that shouldn't be allowed to have
            // duplicates. Buying extra copies of key items can unset event flags and break things
            // like unlocked map progress.
            if (event_flag && row.maxNum == 1 && row.maxRepositoryNum == 1)
            {
                // Don't allow these items to be stored in the item box, since this is basically
                // a loophole for buying a second copy
                no_repository_item_ids.insert(0x40000000 | id);

                // Additionally, limit the sold quantity of items if they have an event flag that
                // can store stock counts. This is mainly for the flask of wondrous physic, which
                // otherwise would be duplicatable by drinking it before opening the shop.
                if (event_flag % 10 == 0 && goods_flag_counts[event_flag - event_flag % 10] == 1)
                {
                    sell_quantity = 1;
                }
            }

            lineups->push_back({
                .equipId = (int)id,
                .eventFlag_forStock = event_flag,
                .sellQuantity = sell_quantity,
                .equipType = equip_type_goods,
            });
        }
    }

    for (auto [id, row] :
         from::params::get_param<from::paramdef::EQUIP_PARAM_GEM_ST>(L"EquipParamGem"))
    {
        bool is_dlc = false;
        auto gem_name = ermerchant::get_message(from::msgbnd::gem_name, id);
        if (gem_name.empty())
        {
            is_dlc = true;
            gem_name = ermerchant::get_message(from::msgbnd::dlc_gem_name, id);
        }

        if (gem_name.empty() || gem_name.starts_with(cut_content_prefix))
        {
            continue;
        }

        std::vector<from::paramdef::SHOP_LINEUP_PARAM> *lineups = nullptr;

        if (is_dlc)
        {
            lineups = &dlc_ashes_of_war_lineups;
        }
        else
        {
            lineups = &ash_of_war_lineups;
        }

        auto event_flag_it = gems_flags.find(id);
        lineups->push_back({
            .equipId = (int)id,
            .eventFlag_forStock = event_flag_it == gems_flags.end() ? 0 : event_flag_it->second,
            .equipType = equip_type_gem,
        });
    }

    for (auto [id, row] : from::params::get_param<from::paramdef::REINFORCE_PARAM_WEAPON_ST>(
             L"ReinforceParamWeapon"))
    {
        auto level = id % 50;
        max_level_by_reinforce_type_id[id - level] = level;
    }

    // Hook SoloParamRepositoryImp::LookupShopMenu to return the new shops added by the mod
    modutils::hook(
        {
            // Note - the mov instructions are 44 or 45 depending on if this is the Japanese or
            // international .exe, and the stack offset is either -10 or -08. This pattern works
            // for both versions.
            .aob = "?? 8b 4e 14"     // mov r9d, [rsi + 14]
                   "?? 8b 46 10"     // mov r8d, [rsi + 10]
                   "33 d2"           // xor edx, edx
                   "48 8d 4d ??"     // lea rcx, [rbp + ??]
                   "e8 ?? ?? ?? ??", // call SoloParamRepositoryImp::LookupShopMenu
            .offset = 14,
            .relative_offsets = {{1, 5}},
        },
        solo_param_repository_lookup_shop_menu_detour, solo_param_repository_lookup_shop_menu);

    // Hook SoloParamRepositoryImp::LookupShopLineup to return shop lineups for every buyable item
    modutils::hook(
        {
            .aob = "48 8d 15 ?? ?? ?? ??" // lea rdx, [shop_lineup_param_indexes]
                   "45 33 c0"             // xor r8d, r8d
                   "?? ?? ??"             // ???
                   "e8 ?? ?? ?? ??"       // call SoloParamRepositoryImp::GetParamResCap
                   "48 85 c0"             // test rax, rax
                   "74 ??",               // jz end_lbl
            .offset = -129,
        },
        solo_param_repository_lookup_shop_lineup_detour, solo_param_repository_lookup_shop_lineup);

    // Hook OpenRegularShop() to perform some memory hacks when opening up one of the Glorious
    // Merchant shops, in order to change the default sort order. Sorting by item type suits very
    // large lists better.
    modutils::hook(
        {
            .aob = "4c 8b 49 18"           // mov    r9, [rcx + 0x18]
                   "48 8b d9"              // mov    rbx,rcx
                   "48 8d 4c 24 20"        // lea    rcx, [rsp + 0x20]
                   "e8 ?? ?? ?? ??"        // call   OpenRegularShopInner
                   "48 8d 4c 24 20"        // lea    rcx, [rsp + 0x20]
                   "0f 10 00"              // movups xmm0, [rax]
                   "c7 43 10 05 00 00 00", // mov    [rbx + 0x10], 5
            .offset = -6,
        },
        open_regular_shop_detour, open_regular_shop);

    modutils::hook(
        {
            .aob = "83 cb ff"  // or  sellValue, -1
                   "41 8b c0"  // mov eax, r8d
                   "c1 e8 1c"  // shr eax, 28
                   "48 8b f1"  // mov rsi, itemId
                   "83 f8 0f", // cmp eax, 0xf
            .offset = -29,
        },
        get_sell_value_detour, get_sell_value);

    modutils::hook(
        {
            .aob = "48 8b 5c 24 70"  // mov rbx, qword ptr [rsp + local_res8]
                   "b8 58 02 00 00"  // mov maxRepositoryNum, 600
                   "48 8b 7c 24 78", // mov rdi, qword ptr [rsp + local_res10]
            .offset = -521,
        },
        get_max_repository_num_detour, get_max_repository_num);

    // Hook CS::CSFD4VirtualMemoryFlag::GetEventFlag() to make Kalé always alive, so the shop is
    // accessible to players who murdered him.
    modutils::hook(
        {
            .aob = "41 f7 f0"    // div r8d
                   "4c 8b d1"    // mov r10, EventFlagMan
                   "45 33 c9"    // xor r9d, r9d
                   "44 0f af c0" // imul r8d, eax
                   "45 2b d8",   // sub r11d, r8d
            .offset = -12,
        },
        get_event_flag_detour, get_event_flag);

    game_data_man_addr = modutils::scan<from::CS::GameDataMan *>({
        .aob = "48 8B 05 ?? ?? ?? ??" // mov rax, [GameDataMan]
               "48 85 C0"             // test rax, rax
               "74 05"                // je 10
               "48 8B 40 58"          // move rax, [rax + 0x58]
               "C3"                   // ret
               "C3",                  // ret
        .relative_offsets = {{3, 7}},
    });
}

void ermerchant::set_shop_open(bool shop_open)
{
    is_shop_open = shop_open;
}

void ShopItemCache::loadPage(size_t pageIndex) {
    if (pageIndex == currentPage) return;

    // Unload current page
    unloadPage(currentPage);

    // Load new page
    size_t startIndex = pageIndex * PAGE_SIZE;
    size_t endIndex = std::min(startIndex + PAGE_SIZE, getTotalItems());

    for (size_t i = startIndex; i < endIndex; i++) {
        ShopItem* item = itemPool.allocate();
        // Initialize item data here
        activeItems.push_back(item);
    }

    currentPage = pageIndex;
}

void ShopItemCache::unloadPage(size_t pageIndex) {
    for (auto* item : activeItems) {
        itemPool.deallocate(item);
    }
    activeItems.clear();
}

ShopItem* ShopItemCache::getItem(size_t index) {
    size_t pageIndex = index / PAGE_SIZE;
    if (pageIndex != currentPage) {
        loadPage(pageIndex);
    }
    return activeItems[index % PAGE_SIZE];
}

void ShopItemCache::cleanup() {
    unloadPage(currentPage);
    currentPage = 0;
}