/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

enum scar_e {
	SCAR_LONGIDLE = 0,
	SCAR_IDLE1,
	SCAR_RELOAD,
	SCAR_DEPLOY,
	SCAR_SHOOT_1,
	SCAR_SHOOT_2,
	SCAR_SHOOT_3,
};

class CScar : public CBasePlayerWeapon
{
public:
	void Spawn() ;
	void Precache() ;
	int iItemSlot()  { return 2; }
	int GetItemInfo(ItemInfo* p) ;
	//burst - add these two
	int ammoToShoot;
	float nextBurstShoot;

	void IncrementAmmo(CBasePlayer* pPlayer);
	void PrimaryAttack() ;
	void ScarFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
	BOOL Deploy() ;
	void Reload() ;
	void WeaponIdle() ;

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
LINK_ENTITY_TO_CLASS(weapon_scar, CScar);



void CScar::Spawn()
{
	pev->classname = MAKE_STRING("weapon_scar"); // hack to allow for old names
	Precache();
	m_iId = WEAPON_SCAR;
	SET_MODEL(ENT(pev), "models/w_556ar.mdl");

	m_iDefaultAmmo = SCAR_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}


void CScar::Precache(void)
{
	PRECACHE_MODEL("models/v_556ar.mdl");
	PRECACHE_MODEL("models/w_556ar.mdl");

	m_iShell = PRECACHE_MODEL("models/556shell.mdl");// brass shell

	PRECACHE_SOUND("weapons/scarfire_1.wav");//silenced scar
}

int CScar::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SCAR_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_SCAR;
	p->iWeight = SCAR_WEIGHT;

	return 1;
}

BOOL CScar::Deploy()
{
	ammoToShoot = 0; //burst - reset state

	// pev->body = 1;
	return DefaultDeploy("models/v_556ar.mdl", "models/p_556ar.mdl", SCAR_DEPLOY, "onehanded");
}


void CScar::PrimaryAttack(void)
{
	//burst - start shooting
	ammoToShoot = 3;
	m_flTimeWeaponIdle = gpGlobals->time - 0.1f;;
	//end
}

void CScar::ScarFire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = gpGlobals->time + 0.1;
		}

		return;
	}

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	int flags;

	if (m_iClip != 0)
		SendWeaponAnim(SCAR_SHOOT_2);
	else
		SendWeaponAnim(SCAR_SHOOT_2);

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	// silenced
	if (pev->body == 1)
	{
		m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;
	}
	else
	{
		// non-silenced
		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/scar_fire1.wav", RANDOM_FLOAT(0.92, 1.0), ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
	}

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming;

	if (fUseAutoAim)
	{
		vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);
	}
	else
	{
		vecAiming = gpGlobals->v_forward;
	}

	Vector vecDir;
	m_pPlayer->FireBullets(1, vecSrc, vecAiming, Vector(flSpread, flSpread, flSpread), 8192, BULLET_PLAYER_9MM, 0);

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), fUseAutoAim ? m_usFireScar1 : m_usFireScar2, 0.0, (float*)&g_vecZero, (float*)&g_vecZero, vecDir.x, vecDir.y, 0, 0, (m_iClip == 0) ? 1 : 0, 0);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + flCycleTime;

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}


void CScar::Reload(void)
{
	ammoToShoot = 0; //burst - reset state

	DefaultReload(SCAR_MAX_CLIP, SCAR_RELOAD, 1.5);
}


void CScar::WeaponIdle(void)
{

	//burst - burst mechanism ( TODO: Move to CBasePlayerWeapon::ItemPostFrame())
	if (nextBurstShoot < gpGlobals->time && ammoToShoot > 0)
	{
		float delay = 0.07f;

		ScarFire(0.01, delay, FALSE);
		nextBurstShoot = gpGlobals->time + delay;
		m_pPlayer->pev->punchangle.x--;
		ammoToShoot--;
	}
	//end

	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	// only idle if the slid isn't back
	if (m_iClip != 0)
	{
		int iAnim;
		float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0.0, 1.0);

		if (flRand <= 0.3 + 0 * 0.75)
		{
			iAnim = SCAR_IDLE1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 49.0 / 16;
		}
		else if (flRand <= 0.6 + 0 * 0.875)
		{
			iAnim = SCAR_IDLE1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0 / 16.0;
		}
		else
		{
			iAnim = SCAR_IDLE1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 40.0 / 16.0;
		}
		SendWeaponAnim(iAnim, 1);
	}
}













