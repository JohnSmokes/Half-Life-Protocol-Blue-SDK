/***
* ----------------------------------------------------------------------------------------------------------------
* Half-Life: Another Shift USAS12 Code - by https://www.moddb.com/members/johncalhoun
* ----------------------------------------------------------------------------------------------------------------
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
// jay - removed gamerules include

// jay - removed shotgun spreads, xash is SP only anyways

enum usas_e {
	USAS_LONGIDLE = 0,
	USAS_IDLE1,
	USAS_RELOAD,
	USAS_DEPLOY,
	USAS_SHOOT
};

class CUSAS : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int iItemSlot() { return 3; }
	int GetItemInfo(ItemInfo* p);
	int AddToPlayer(CBasePlayer* pPlayer);

	void PrimaryAttack(void);
	BOOL Deploy();
	void Reload(void);
	void WeaponIdle(void);

	//int m_fInReload;
	//float m_flNextReload;
	int m_iShell;
	//float m_flPumpTime;
	// jay - these commented out lines are unused in an AA12. they are used for the shotgun's custom reload and pump sound
};
LINK_ENTITY_TO_CLASS(weapon_usas, CUSAS);


void CUSAS::Spawn()
{
	Precache();
	m_iId = WEAPON_USAS;
	SET_MODEL(ENT(pev), "models/w_usas.mdl");

	m_iDefaultAmmo = USAS_MAX_CLIP;

	FallInit();// get ready to fall
}


void CUSAS::Precache(void)
{
	PRECACHE_MODEL("models/v_usas.mdl");
	PRECACHE_MODEL("models/w_usas.mdl");
	PRECACHE_MODEL("models/p_9mmAR.mdl");

	m_iShell = PRECACHE_MODEL("models/shotgunshell.mdl");// shotgun shell

	//PRECACHE_SOUND("items/9mmclip1.wav");
	// jay - ammo pickup sound; not needed in precache

	PRECACHE_SOUND("weapons/usas_fire1.wav");// USAS 12 fire 1
	PRECACHE_SOUND("weapons/auto2.wav");// USAS 12 fire 2 

	PRECACHE_SOUND("weapons/clipinsert1.wav");	// AA12 reload
	PRECACHE_SOUND("weapons/cliprelease1.wav");	// AA12 reload

//	PRECACHE_SOUND ("weapons/clipinsert1.wav");	// AA12 reload - played on client
//	PRECACHE_SOUND ("weapons/clipinsert2.wav");	// AA12 reload - played on client

	PRECACHE_SOUND("weapons/357_cock1.wav"); // gun empty sound
}

int CUSAS::AddToPlayer(CBasePlayer* pPlayer)
{
	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
		WRITE_BYTE(m_iId);
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}


int CUSAS::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "buckshot";
	p->iMaxAmmo1 = BUCKSHOT_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = USAS_MAX_CLIP;
	p->iSlot = 3;
	p->iPosition = 4;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_USAS;
	p->iWeight = USAS_WEIGHT;

	return 1;
}



BOOL CUSAS::Deploy()
{
	return DefaultDeploy("models/v_usas.mdl", "models/p_9mmAR.mdl", USAS_DEPLOY, "shotgun");	// jay - the last line here is the player animation. this should be "shotgun" here, as "aa12" doesn't exist
}


void CUSAS::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = gpGlobals->time + 0.25;
		return;
	}

	if (m_iClip <= 0)
	{
		Reload();
		if (m_iClip == 0)
			PlayEmptySound();
		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_iClip--;
	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	SendWeaponAnim(USAS_SHOOT);

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector	vecShellVelocity = m_pPlayer->pev->velocity
		+ gpGlobals->v_right * RANDOM_FLOAT(50, 70)
		+ gpGlobals->v_up * RANDOM_FLOAT(100, 150)
		+ gpGlobals->v_forward * 25;

	EjectBrass(m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_up * -12 + gpGlobals->v_forward * 20 + gpGlobals->v_right * 4, vecShellVelocity, pev->angles.y, m_iShell, TE_BOUNCE_SHOTSHELL);

	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/usas_fire1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0, 0x1f));

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	// jay - only use singleplayer spread, this is xash after all
	m_pPlayer->FireBullets(6, vecSrc, vecAiming, VECTOR_CONE_10DEGREES, 2048, BULLET_PLAYER_BUCKSHOT, 0);

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	if (m_iClip != 0)
		m_flNextPrimaryAttack = gpGlobals->time + 0.25;

	m_flTimeWeaponIdle = gpGlobals->time + 5;	// jay - fixed idle timer

	m_pPlayer->pev->punchangle.x -= 5;
}


void CUSAS::Reload(void)
{
	DefaultReload(USAS_MAX_CLIP, USAS_RELOAD, 1.5);
}



void CUSAS::WeaponIdle(void)
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle > gpGlobals->time)
		return;

	int iAnim;
	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		iAnim = USAS_LONGIDLE;
		break;

	default:
	case 1:
		iAnim = USAS_IDLE1;
		break;
	}

	SendWeaponAnim(iAnim);

	m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT(10, 15);// how long till we do this again.
}


/*
class CShotgunAmmo : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_shotbox.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_shotbox.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo(CBaseEntity* pOther)
	{
		if (pOther->GiveAmmo(AMMO_BUCKSHOTBOX_GIVE, "buckshot", BUCKSHOT_MAX_CARRY) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};
*/

// jay - this class already exists in shotgun.cpp. redefining it in another file breaks everything horribly
