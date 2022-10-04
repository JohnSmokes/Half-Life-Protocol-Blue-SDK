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
* 
* ----------------------------------------------------------------------------------------------------------------
* Half-Life: Another Shift SCAR DMR Code - by https://www.moddb.com/members/johncalhoun & Baconstu
* ----------------------------------------------------------------------------------------------------------------
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "nodes.h"
#include "scar.h"

enum stealth_e {
	STEALTH_LONGIDLE = 0,
	STEALTH_IDLE1,
	STEALTH_RELOAD,
	STEALTH_DEPLOY,
	STEALTH_SHOOT_1,
	STEALTH_SHOOT_2,
	STEALTH_SHOOT_3,
};

LINK_ENTITY_TO_CLASS(weapon_stealth, CStealth);


void CStealth::Spawn()
{
	pev->classname = MAKE_STRING("weapon_stealth"); // hack to allow for old names
	Precache();
	m_iId = WEAPON_STEALTH;
	SET_MODEL(ENT(pev), "models/w_brifle.mdl");

	m_iDefaultAmmo = STEALTH_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}


void CStealth::Precache(void)
{
	PRECACHE_MODEL("models/v_brifle.mdl");
	PRECACHE_MODEL("models/w_brifle.mdl");
	PRECACHE_MODEL("models/p_brifle.mdl");

	m_iShell = PRECACHE_MODEL("models/556shell.mdl");// brass shell

	PRECACHE_SOUND("weapons/briflefire_1.wav");//silenced scar
}

int CStealth::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = STEALTH_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 3;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_STEALTH;
	p->iWeight = STEALTH_WEIGHT;

	return 1;
}

BOOL CStealth::Deploy()
{
	ammoToShoot = 0; //burst - reset state

	// pev->body = 1;
	return DefaultDeploy("models/v_556ar.mdl", "models/p_9mmAR.mdl", STEALTH_DEPLOY, "onehanded");
}

void CStealth::Holster()
{
	if(m_pPlayer->m_iFOV != 0)
		SecondaryAttack();

	// use default behaviour
	CBasePlayerWeapon::Holster();
}

void CStealth::PrimaryAttack(void)
{
	//burst - start shooting
	if (ammoToShoot == 0)
	{
		ammoToShoot = 3;
	}
	//end

	m_flNextPrimaryAttack = gpGlobals->time + 0.8f;
}

void CStealth::SecondaryAttack(void)
{

	if (m_pPlayer->m_iFOV != 0)
	{
		m_pPlayer->m_iFOV = 0;  // 0 means reset to default fov
	}
	else
	{
		m_pPlayer->m_iFOV = 40;
	}

	m_flNextSecondaryAttack = gpGlobals->time + 0.5;
}

void CStealth::ScarFire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
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
	m_pPlayer->pev->punchangle.x--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	int flags;

	if (m_iClip != 0)
		SendWeaponAnim(STEALTH_SHOOT_2);
	else
		SendWeaponAnim(STEALTH_SHOOT_3);

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
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/briflefire1.wav", RANDOM_FLOAT(0.92, 1.0), ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
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

	//m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + flCycleTime;

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}


void CStealth::Reload(void)
{
	ammoToShoot = 0; //burst - reset state

	if (m_pPlayer->m_iFOV != 0)
		SecondaryAttack();

	DefaultReload(STEALTH_MAX_CLIP, STEALTH_RELOAD, 1.5);
}


void CStealth::WeaponIdle(void)
{
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
			iAnim = STEALTH_LONGIDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 49.0 / 16;
		}
		else if (flRand <= 0.6 + 0 * 0.875)
		{
			iAnim = STEALTH_IDLE1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0 / 16.0;
		}
		else
		{
			iAnim = STEALTH_IDLE1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 40.0 / 16.0;
		}
		SendWeaponAnim(iAnim, 1);
	}
}












