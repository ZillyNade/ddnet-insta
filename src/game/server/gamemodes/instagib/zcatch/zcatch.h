#ifndef GAME_SERVER_GAMEMODES_INSTAGIB_ZCATCH_ZCATCH_H
#define GAME_SERVER_GAMEMODES_INSTAGIB_ZCATCH_ZCATCH_H

#include "../base_instagib.h"

class CGameControllerZcatch : public CGameControllerInstagib
{
public:
	CGameControllerZcatch(class CGameContext *pGameServer);
	~CGameControllerZcatch() override;

	int m_aBodyColors[MAX_CLIENTS] = {0};

	void KillPlayer(class CPlayer *pVictim, class CPlayer *pKiller);
	void OnCaught(class CPlayer *pVictim, class CPlayer *pKiller);
	void ReleasePlayer(class CPlayer *pPlayer, const char *pMsg);

	void Tick() override;
	void Snap(int SnappingClient) override;
	void OnPlayerConnect(CPlayer *pPlayer) override;
	void OnPlayerDisconnect(class CPlayer *pDisconnectingPlayer, const char *pReason) override;
	int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon) override;
	void OnCharacterSpawn(class CCharacter *pChr) override;
	bool CanJoinTeam(int Team, int NotThisId, char *pErrorReason, int ErrorReasonSize) override;
	void DoTeamChange(CPlayer *pPlayer, int Team, bool DoChatMsg) override;
	bool OnEntity(int Index, int x, int y, int Layer, int Flags, bool Initial, int Number) override;
	bool DoWincheckRound() override;
	void OnRoundStart() override;
	bool OnSelfkill(int ClientId) override;
	int GetAutoTeam(int NotThisId) override;
	bool OnChangeInfoNetMessage(const CNetMsg_Cl_ChangeInfo *pMsg, int ClientId) override;
	int GetPlayerTeam(class CPlayer *pPlayer, bool Sixup) override;
	bool OnSetTeamNetMessage(const CNetMsg_Cl_SetTeam *pMsg, int ClientId) override;
	bool IsWinner(const CPlayer *pPlayer, char *pMessage, int SizeOfMessage) override;
	bool IsLoser(const CPlayer *pPlayer) override;

	enum class ECatchGameState
	{
		WAITING_FOR_PLAYERS,
		RELEASE_GAME,
		RUNNING,
		RUNNING_COMPETITIVE, // NEEDS 10 OR MORE TO START A REAL ROUND
	};
	ECatchGameState m_CatchGameState = ECatchGameState::WAITING_FOR_PLAYERS;
	ECatchGameState CatchGameState() const;
	void SetCatchGameState(ECatchGameState State);

	void CheckGameState();
	bool IsCatchGameRunning() const;

	// colors

	enum class ECatchColors
	{
		TEETIME,
		SAVANDER
	};
	ECatchColors m_CatchColors = ECatchColors::TEETIME;

	// gets the tee's body color based on the amount of its kills
	// the value is the integer that will be sent over the network
	int GetBodyColor(int Kills);

	int GetBodyColorTeetime(int Kills);
	int GetBodyColorSavander(int Kills);

	void SetCatchColors(class CPlayer *pPlayer);

	void SendSkinBodyColor7(int ClientId, int Color);

	void OnUpdateZcatchColorConfig() override;
};
#endif // GAME_SERVER_GAMEMODES_ZCATCH_H
