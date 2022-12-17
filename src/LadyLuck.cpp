#include "LadyLuck.h"

bool LadyLuckCreatureScript::OnGossipHello(Player* player, Creature* creature)
{
    ClearGossipMenuFor(player);

    if (IsInLottery(player))
    {
        PromptExit(player, creature);
    }
    else
    {
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I would like to enter the lottery.", GOSSIP_SENDER_MAIN, LADYLUCK_ENTERLOTTERY);

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Goodbye.", GOSSIP_SENDER_MAIN, LADYLUCK_GOODBYE);
        SendGossipMenuFor(player, LADYLUCK_GOSSIPTEXT, creature->GetGUID());
    }

    return true;
}

void LadyLuckCreatureScript::EnterLottery(Player* player)
{
    PlayerLotteryInfo playerInfo;
    
    playerInfo.playerGuid = player->GetGUID();
    
    playerInfo.previousLocation.Map = player->GetMapId();
    playerInfo.previousLocation.X = player->GetPositionX();
    playerInfo.previousLocation.Y = player->GetPositionY();
    playerInfo.previousLocation.Z = player->GetPositionZ();
    playerInfo.previousLocation.O = player->GetOrientation();
    
    playerInfo.canLoot = true;
    
    playerLotteryInfo.push_back(playerInfo);
    
    player->TeleportTo(ladyLuckTele.Map, ladyLuckTele.X, ladyLuckTele.Y, ladyLuckTele.Z, ladyLuckTele.O);
}

void LadyLuckCreatureScript::SayGoodbye(Player* player, Creature* /*creature*/)
{
    CloseGossipMenuFor(player);
}

bool LadyLuckCreatureScript::CanEnterLottery(Player* player)
{
    uint32 currencyAmount = player->GetItemCount(ladyLuckCurrency, false);

    if (currencyAmount < ladyLuckCurrencyCount)
    {
        return false;
    }

    return true;
}

void LadyLuckCreatureScript::DeductCurrency(Player* player, uint32 count)
{
    Item* currency = player->GetItemByEntry(ladyLuckCurrency);
    player->DestroyItemCount(currency, count, true);
}

void LadyLuckCreatureScript::ValidateCurrency(Player* player, Creature* creature)
{
    if (CanEnterLottery(player))
    {
        CloseGossipMenuFor(player);
        DeductCurrency(player, ladyLuckCurrencyCount);
        EnterLottery(player);
    }
    else
    {
        ClearGossipMenuFor(player);
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I see, goodbye.", GOSSIP_SENDER_MAIN, LADYLUCK_GOODBYE);
        SendGossipMenuFor(player, LADYLUCK_GOSSIPTEXT_FAIL, creature->GetGUID());
    }
}

bool IsInLottery(Player* player)
{
    for (auto it = playerLotteryInfo.begin(); it != playerLotteryInfo.end(); ++it)
    {
        if (it->playerGuid == player->GetGUID())
        {
            return true;
        }
    }

    return false;
}

void UpdateCanLoot(Player* player, bool state)
{
    for (auto it = playerLotteryInfo.begin(); it != playerLotteryInfo.end(); ++it)
    {
        if (it->playerGuid == player->GetGUID())
        {
            it->canLoot = state;
        }
    }
}

bool CanLoot(Player* player)
{
    for (auto it = playerLotteryInfo.begin(); it != playerLotteryInfo.end(); ++it)
    {
        if (it->playerGuid == player->GetGUID())
        {
            return it->canLoot;
        }
    }

    return false;
}

void LadyLuckCreatureScript::PromptExit(Player* player, Creature* creature)
{
    ClearGossipMenuFor(player);
    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Yes, please.", GOSSIP_SENDER_MAIN, LADYLUCK_EXITLOTTERY);
    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "No, I would like to open another box.", GOSSIP_SENDER_MAIN, LADYLUCK_ENTERLOTTERY_RETRY);
    SendGossipMenuFor(player, LADYLUCK_GOSSIPTEXT_EXIT, creature->GetGUID());
}

void LadyLuckCreatureScript::ExitLottery(Player* player)
{
    CloseGossipMenuFor(player);

    TeleportInfo* teleInfo = nullptr;
    std::vector<PlayerLotteryInfo>::iterator* itToRemove = nullptr;

    for (auto it = playerLotteryInfo.begin(); it != playerLotteryInfo.end(); ++it)
    {
        if (it->playerGuid == player->GetGUID())
        {
            teleInfo = &it->previousLocation;
            itToRemove = &it;
            break;
        }
    }

    if(teleInfo != nullptr && itToRemove != nullptr)
    {
        RestorePlayer(player, teleInfo);
        playerLotteryInfo.erase(*itToRemove);
    }
}

void LadyLuckCreatureScript::RestorePlayer(Player* player, TeleportInfo* teleInfo)
{
    player->TeleportTo(teleInfo->Map, teleInfo->X, teleInfo->Y, teleInfo->Z, teleInfo->O);
}

void LadyLuckCreatureScript::DisplayLotteryOptions(Player* player, Creature* creature)
{
    ClearGossipMenuFor(player);

    if (ladyLuckMoney > 0)
    {
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I would like to use gold.", GOSSIP_SENDER_MAIN, LADYLUCK_ENTERLOTTERY_GOLD, "Are you sure you would like to use gold?", ladyLuckMoney, false);
    }

    if (ladyLuckCurrency > 0)
    {
        std::string itemName = sObjectMgr->GetItemTemplate(ladyLuckCurrency)->Name1;
        ladyLuckCurrencyStr = Acore::StringFormatFmt("Are you sure you would like to use currency?|n|nThis will cost you :|n{}x[{}]", ladyLuckCurrencyCount, itemName);
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I would like to use currency.", GOSSIP_SENDER_MAIN, LADYLUCK_ENTERLOTTERY_CURRENCY, ladyLuckCurrencyStr, 0, false);
    }

    SendGossipMenuFor(player, LADYLUCK_GOSSIPTEXT_SELECT_CURRENCY, creature);
}

bool LadyLuckCreatureScript::OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    if (sender != GOSSIP_SENDER_MAIN)
    {
        return false;
    }

    switch (action)
    {
    case LADYLUCK_ENTERLOTTERY:
        DisplayLotteryOptions(player, creature);
        break;

    case LADYLUCK_ENTERLOTTERY_CURRENCY:
        ValidateCurrency(player, creature);
        break;

    case LADYLUCK_ENTERLOTTERY_GOLD:
        CloseGossipMenuFor(player);
        EnterLottery(player);
        break;

    case LADYLUCK_EXITLOTTERY:
        ExitLottery(player);
        break;

    case LADYLUCK_GOODBYE:
        SayGoodbye(player, creature);
        break;
    }

    return true;
}


bool LadyLuckGameObjectScript::OnGossipHello(Player* player, GameObject* go)
{
    ClearGossipMenuFor(player);

    if (CanLoot(player))
    {
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Open the box", GOSSIP_SENDER_MAIN, LOTTERYBOX_OPEN);
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to open a different box.", GOSSIP_SENDER_MAIN, LOTTERYBOX_GOODBYE);
        SendGossipMenuFor(player, LOTTERYBOX_GOSSIPTEXT, go->GetGUID());
    }
    else
    {
        SendGossipMenuFor(player, LOTTERYBOX_DENY, go->GetGUID());
    }
    
    return true;
}

bool LadyLuckGameObjectScript::OnGossipSelect(Player* player, GameObject* /*go*/, uint32 sender, uint32 action)
{
    if (sender != GOSSIP_SENDER_MAIN)
    {
        return false;
    }

    switch (action)
    {
    case LOTTERYBOX_OPEN:
        ChatHandler(player->GetSession()).SendSysMessage("OPEN");

        break;

    case LOTTERYBOX_GOODBYE:
        ChatHandler(player->GetSession()).SendSysMessage("GOODBYE");
        break;
    }

    return true;
}

void LadyLuckWorldScript::OnAfterConfigLoad(bool /*reload*/)
{
    ladyLuckEnabled = sConfigMgr->GetOption<bool>("LadyLuck.Enable", false);

    ladyLuckCurrency = sConfigMgr->GetOption<uint32>("LadyLuck.Currency", 37711);
    ladyLuckCurrencyCount = sConfigMgr->GetOption<uint32>("LadyLuck.CurrencyCount", 3);

    ladyLuckMoney = sConfigMgr->GetOption<uint32>("LadyLuck.Money", 1000000);

    ladyLuckTele.Map = sConfigMgr->GetOption<uint32>("LadyLuck.TeleMap", 449);
    ladyLuckTele.X = sConfigMgr->GetOption<float>("LadyLuck.TeleX", 0.072697);
    ladyLuckTele.Y = sConfigMgr->GetOption<float>("LadyLuck.TeleY", 9.618815);
    ladyLuckTele.Z = sConfigMgr->GetOption<float>("LadyLuck.TeleZ", -0.227239);
    ladyLuckTele.O = sConfigMgr->GetOption<float>("LadyLuck.TeleO", 1.584149);
}

// Add all scripts in one
void AddLadyLuckScripts()
{
    new LadyLuckWorldScript();
    new LadyLuckCreatureScript();
    new LadyLuckGameObjectScript();
}
