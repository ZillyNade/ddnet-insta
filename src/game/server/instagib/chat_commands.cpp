#include <base/system.h>
#include <engine/shared/protocol.h>
#include <game/generated/protocol.h>
#include <game/server/entities/character.h>
#include <game/server/gamecontroller.h>
#include <game/server/player.h>
#include <game/server/score.h>
#include <game/version.h>

#include <game/server/gamecontext.h>

// implemented in ddracechat.cpp
// yes that is cursed
bool CheckClientId(int ClientId);

void CGameContext::ConReadyChange(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	if(!pSelf->m_pController)
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	pSelf->m_pController->OnPlayerReadyChange(pPlayer);
}

void CGameContext::ConInstaSwap(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	if(!pSelf->m_pController)
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	pSelf->ComCallSwapTeamsVote(pResult->m_ClientId);
}

void CGameContext::ConRankCmdlist(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	if(!pSelf->m_pController)
		return;

	if(pSelf->m_pController->IsDDRaceGameType())
	{
		ConRank(pResult, pUserData);
		return;
	}

	pSelf->SendChatTarget(pResult->m_ClientId, "~~~ ddnet-insta rank commands");
	pSelf->SendChatTarget(pResult->m_ClientId, "~ /rank_kills - highest amount of kills");

	if(pSelf->m_pController->GameFlags() & GAMEFLAG_FLAGS)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "~ /rank_caps - highest amount of flag captures");
		pSelf->SendChatTarget(pResult->m_ClientId, "~ /rank_flags - best (shortest) flag capture time");
	}
}

void CGameContext::ConTopCmdlist(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	if(!pSelf->m_pController)
		return;

	if(pSelf->m_pController->IsDDRaceGameType())
	{
		ConTop(pResult, pUserData);
		return;
	}

	pSelf->SendChatTarget(pResult->m_ClientId, "~~~ ddnet-insta top commands");
	pSelf->SendChatTarget(pResult->m_ClientId, "~ /top5kills - top killers");
	pSelf->SendChatTarget(pResult->m_ClientId, "~ /top5wins - highest amounts of round wins");
	pSelf->SendChatTarget(pResult->m_ClientId, "~ /top5spree - highest killing sprees");

	if(pSelf->m_pController->GameFlags() & GAMEFLAG_FLAGS)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "~ /top5caps - best flag captures by amount");
		pSelf->SendChatTarget(pResult->m_ClientId, "~ /top5flags - fastest flag capturers");
	}
}

void CGameContext::ConStatsRound(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	if(!pSelf->m_pController)
		return;

	if(pSelf->m_pController->IsDDRaceGameType())
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "This command is not available in ddrace gametypes.");
		return;
	}

	if(!pSelf->m_pController->IsStatTrack())
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"! WARNING: not enough players connected to track stats");

	const char *pName = pResult->NumArguments() ? pResult->GetString(0) : pSelf->Server()->ClientName(pResult->m_ClientId);
	int TargetId = pSelf->m_pController->GetCidByName(pName);
	if(TargetId < 0 || TargetId >= MAX_CLIENTS)
		return;
	const CPlayer *pPlayer = pSelf->m_apPlayers[TargetId];
	CPlayer *pRequestingPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pRequestingPlayer)
		return;

	char aBuf[512];
	char aUntrackedOrAccuracy[512];
	str_format(aBuf, sizeof(aBuf), "~~~ round stats for '%s'", pName);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp", aBuf);

	str_format(aBuf, sizeof(aBuf), "~ Points: %d", pPlayer->m_Stats.m_Points);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp", aBuf);

	aUntrackedOrAccuracy[0] = '\0';
	if(pPlayer->m_Stats.m_ShotsFired)
		str_format(aUntrackedOrAccuracy, sizeof(aUntrackedOrAccuracy), " (%.2f%% hit accuracy)", pPlayer->m_Stats.HitAccuracy());
	if(!pSelf->m_pController->IsStatTrack())
		str_format(aUntrackedOrAccuracy, sizeof(aUntrackedOrAccuracy), " (%d untracked)", pPlayer->m_Kills);
	str_format(aBuf, sizeof(aBuf), "~ Kills: %d%s", pPlayer->m_Stats.m_Kills, aUntrackedOrAccuracy);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp", aBuf);

	char aUntracked[512];
	aUntracked[0] = '\0';
	if(!pSelf->m_pController->IsStatTrack())
		str_format(aUntracked, sizeof(aUntracked), " (%d untracked)", pPlayer->m_Deaths);
	str_format(aBuf, sizeof(aBuf), "~ Deaths: %d%s", pPlayer->m_Stats.m_Deaths, aUntracked);

	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp", aBuf);
	str_format(aBuf, sizeof(aBuf), "~ Current killing spree: %d", pPlayer->Spree());
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp", aBuf);
	str_format(aBuf, sizeof(aBuf), "~ Highest killing spree: %d", pPlayer->m_Stats.m_BestSpree);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp", aBuf);

	pSelf->m_pController->OnShowRoundStats(&pPlayer->m_Stats, pRequestingPlayer, pName);

	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
		"~ see also /statsall");
}

void CGameContext::ConStatsAllTime(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	if(!pSelf->m_pController)
		return;

	if(pSelf->m_pController->IsDDRaceGameType())
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "This command is not available in ddrace gametypes.");
		return;
	}

	const char *pName = pResult->NumArguments() ? pResult->GetString(0) : pSelf->Server()->ClientName(pResult->m_ClientId);
	pSelf->m_pController->m_pSqlStats->ShowStats(pResult->m_ClientId, pName, pSelf->m_pController->StatsTable());
}

void CGameContext::ConRankKills(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	if(!pSelf->m_pController)
		return;

	if(pSelf->m_pController->IsDDRaceGameType())
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "This command is not available in ddrace gametypes.");
		return;
	}

	const char *pName = pResult->NumArguments() ? pResult->GetString(0) : pSelf->Server()->ClientName(pResult->m_ClientId);
	pSelf->m_pController->m_pSqlStats->ShowRank(pResult->m_ClientId, pName, "Kills", "kills", pSelf->m_pController->StatsTable(), "DESC");
}

void CGameContext::ConInstaRankPoints(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	if(!pSelf->m_pController)
		return;

	if(pSelf->m_pController->IsDDRaceGameType())
	{
		ConPoints(pResult, pUserData);
		return;
	}

	const char *pName = pResult->NumArguments() ? pResult->GetString(0) : pSelf->Server()->ClientName(pResult->m_ClientId);
	pSelf->m_pController->m_pSqlStats->ShowRank(pResult->m_ClientId, pName, "Points", "points", pSelf->m_pController->StatsTable(), "DESC");
}

void CGameContext::ConTopKills(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	if(!pSelf->m_pController)
		return;

	if(pSelf->m_pController->IsDDRaceGameType())
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "This command is not available in ddrace gametypes.");
		return;
	}

	const char *pName = pSelf->Server()->ClientName(pResult->m_ClientId);
	int Offset = pResult->NumArguments() ? pResult->GetInteger(0) : 1;
	pSelf->m_pController->m_pSqlStats->ShowTop(pResult->m_ClientId, pName, "Kills", "kills", pSelf->m_pController->StatsTable(), "DESC", Offset);
}

void CGameContext::ConRankFastcaps(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	if(!pSelf->m_pController)
		return;

	if(pSelf->m_pController->IsDDRaceGameType())
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "This command is not available in ddrace gametypes.");
		return;
	}

	if(!(pSelf->m_pController->GameFlags() & GAMEFLAG_FLAGS))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "this gamemode has no flags");
		return;
	}

	const char *pName = pResult->NumArguments() ? pResult->GetString(0) : pSelf->Server()->ClientName(pResult->m_ClientId);
	pSelf->m_pController->m_pSqlStats->ShowFastcapRank(
		pResult->m_ClientId,
		pName,
		pSelf->Server()->GetMapName(),
		pSelf->m_pController->m_pGameType,
		pSelf->m_pController->IsGrenadeGameType(),
		false); // show all times stat track or not
}

void CGameContext::ConTopFastcaps(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	if(!pSelf->m_pController)
		return;

	if(pSelf->m_pController->IsDDRaceGameType())
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "This command is not available in ddrace gametypes.");
		return;
	}

	if(!(pSelf->m_pController->GameFlags() & GAMEFLAG_FLAGS))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "this gamemode has no flags");
		return;
	}

	const char *pName = pResult->NumArguments() ? pResult->GetString(0) : pSelf->Server()->ClientName(pResult->m_ClientId);
	int Offset = pResult->NumArguments() ? pResult->GetInteger(0) : 1;
	pSelf->m_pController->m_pSqlStats->ShowFastcapTop(
		pResult->m_ClientId,
		pName,
		pSelf->Server()->GetMapName(),
		pSelf->m_pController->m_pGameType,
		pSelf->m_pController->IsGrenadeGameType(),
		false, // show all times stat track or not
		Offset);
}

void CGameContext::ConRankFlagCaptures(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	if(!pSelf->m_pController)
		return;

	if(pSelf->m_pController->IsDDRaceGameType())
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "This command is not available in ddrace gametypes.");
		return;
	}

	if(!(pSelf->m_pController->GameFlags() & GAMEFLAG_FLAGS))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "this gamemode has no flags");
		return;
	}

	const char *pName = pResult->NumArguments() ? pResult->GetString(0) : pSelf->Server()->ClientName(pResult->m_ClientId);
	pSelf->m_pController->m_pSqlStats->ShowRank(pResult->m_ClientId, pName, "Flag captures", "flag_captures", pSelf->m_pController->StatsTable(), "DESC");
}

#define MACRO_ADD_COLUMN(name, sql_name, sql_type, bind_type, default, merge_method) ;
#define MACRO_RANK_COLUMN(name, sql_name, display_name, order_by) \
	void CGameContext::ConInstaRank##name(IConsole::IResult *pResult, void *pUserData) \
	{ \
		CGameContext *pSelf = (CGameContext *)pUserData; \
		if(!CheckClientId(pResult->m_ClientId)) \
			return; \
		if(!pSelf->m_pController) \
			return; \
		if(pSelf->m_pController->IsDDRaceGameType()) \
		{ \
			pSelf->SendChatTarget(pResult->m_ClientId, "This command is not available in ddrace gametypes."); \
			return; \
		} \
\
		const char *pName = pResult->NumArguments() ? pResult->GetString(0) : pSelf->Server()->ClientName(pResult->m_ClientId); \
		pSelf->m_pController->m_pSqlStats->ShowRank(pResult->m_ClientId, pName, display_name, #sql_name, pSelf->m_pController->StatsTable(), order_by); \
	}
#define MACRO_TOP_COLUMN(name, sql_name, display_name, order_by) \
	void CGameContext::ConInstaTop##name(IConsole::IResult *pResult, void *pUserData) \
	{ \
		CGameContext *pSelf = (CGameContext *)pUserData; \
		if(!CheckClientId(pResult->m_ClientId)) \
			return; \
		if(!pSelf->m_pController) \
			return; \
		if(pSelf->m_pController->IsDDRaceGameType()) \
		{ \
			pSelf->SendChatTarget(pResult->m_ClientId, "This command is not available in ddrace gametypes."); \
			return; \
		} \
\
		const char *pName = pSelf->Server()->ClientName(pResult->m_ClientId); \
		int Offset = pResult->NumArguments() ? pResult->GetInteger(0) : 1; \
		pSelf->m_pController->m_pSqlStats->ShowTop(pResult->m_ClientId, pName, display_name, #sql_name, pSelf->m_pController->StatsTable(), order_by, Offset); \
	}
#include <game/server/instagib/sql_colums_all.h>
#undef MACRO_ADD_COLUMN
#undef MACRO_RANK_COLUMN
#undef MACRO_TOP_COLUMN
