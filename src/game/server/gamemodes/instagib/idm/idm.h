#ifndef GAME_SERVER_GAMEMODES_INSTAGIB_IDM_IDM_H
#define GAME_SERVER_GAMEMODES_INSTAGIB_IDM_IDM_H

#include <game/server/gamemodes/instagib/dm.h>

class CGameControllerIDM : public CGameControllerInstaBaseDM
{
public:
	CGameControllerIDM(class CGameContext *pGameServer);
	~CGameControllerIDM() override;

	void OnCharacterSpawn(class CCharacter *pChr) override;
	void Tick() override;
};
#endif