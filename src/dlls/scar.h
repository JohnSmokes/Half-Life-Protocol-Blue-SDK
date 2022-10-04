#include "weapons.h"
#include "player.h"

class CStealth : public CBasePlayerWeapon
{
public:
	void Spawn();
	void Precache();
	int iItemSlot() { return 2; }
	int GetItemInfo(ItemInfo* p);
	//burst - add these two
	int ammoToShoot;
	float nextBurstShoot;

	void IncrementAmmo(CBasePlayer* pPlayer);
	void PrimaryAttack();
	void SecondaryAttack();
	void ScarFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
	BOOL Deploy();
	void Holster();
	void Reload();
	void WeaponIdle();

	BOOL UseDecrement()
	{
#if defined( CLIENT_WEAPONS )
		return UTIL_DefaultUseDecrement();
#else
		return FALSE;
#endif
	}
private:
	int m_iShell;
	unsigned short m_usFireScar1;
	unsigned short m_usFireScar2;
};