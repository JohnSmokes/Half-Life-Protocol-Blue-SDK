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
	void Spawn(void);
	void Precache(void);
	int iItemSlot(void) { return 2; }
	int GetItemInfo(ItemInfo* p);

	void PrimaryAttack(void);
	void SecondaryAttack(void);
	void BurstFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
	BOOL Deploy(void);
	void Holster();
	void Reload(void);
	void WeaponIdle(void);
	int m_fFireMode;
	int m_iShell;
	int m_fInZoom; // don't save this
	int BurstShots = 0;
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
	PRECACHE_MODEL("models/p_556ar.mdl");

	m_iShell = PRECACHE_MODEL("models/556shell.mdl");// brass shell

	PRECACHE_SOUND("weapons/scarfire_1.wav");//silenced scar
	PRECACHE_SOUND("weapons/scarfire_2.wav");//silenced scar
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
	// pev->body = 1;
	return DefaultDeploy("models/v_556ar.mdl", "models/p_556ar.mdl", SCAR_DEPLOY, "onehanded");
}

void CScar::Holster()
{
	m_fInReload = FALSE;// cancel any reload in progress.

	if (m_fInZoom)
	{
		SecondaryAttack();
	}

	m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5;
	if (m_iClip)
		SendWeaponAnim(SCAR_DEPLOY);
	else
		SendWeaponAnim(SCAR_DEPLOY);
}

void CScar::SecondaryAttack()
{
	if (m_fInZoom)
	{
		m_pPlayer->m_iFOV = 0; // 0 means reset to default fov
		m_fInZoom = 0;
	}
	else
	{
		m_pPlayer->m_iFOV = 5;
		m_fInZoom = 1;
	}

	pev->nextthink = gpGlobals->time + 0.1;
	m_flNextSecondaryAttack = gpGlobals->time + 1.0;
}


void CScar::PrimaryAttack(void)
{
	BurstFire(0.04, 0.05, FALSE);
	BurstShots = BurstShots + 1;
	if (BurstShots >= 3)
	{
		BurstShots = 0;
		BurstFire(0.04, 0.5, FALSE);
	}
}

void CScar::BurstFire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = gpGlobals->time + 0.2;
		}

		return;
	}

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	if (m_iClip != 0)
		SendWeaponAnim(SCAR_SHOOT_2);
	else
		SendWeaponAnim(SCAR_SHOOT_2);

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector	vecShellVelocity = m_pPlayer->pev->velocity
		+ gpGlobals->v_right * RANDOM_FLOAT(50, 70)
		+ gpGlobals->v_up * RANDOM_FLOAT(100, 150)
		+ gpGlobals->v_forward * 20;
	EjectBrass(pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_up * -12 + gpGlobals->v_forward * 32 + gpGlobals->v_right * 6, vecShellVelocity, pev->angles.y, m_iShell, TE_BOUNCE_SHELL);

	// silenced
	if (pev->body == 1)
	{
		m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

		switch (RANDOM_LONG(0, 1))
		{
		case 0:
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/pl_gun1.wav", RANDOM_FLOAT(0.9, 1.0), ATTN_NORM);
			break;
		case 1:
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/pl_gun2.wav", RANDOM_FLOAT(0.9, 1.0), ATTN_NORM);
			break;
		}
	}
	else
	{
		// non-silenced
		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/pl_gun1.wav", RANDOM_FLOAT(0.92, 1.0), ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));

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

	m_pPlayer->FireBullets(1, vecSrc, vecAiming, Vector(flSpread, flSpread, flSpread), 8192, BULLET_PLAYER_9MM, 0);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + flCycleTime;

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT(10, 15);

	m_pPlayer->pev->punchangle.x -= 1;
}


void CScar::Reload(void)
{
	DefaultReload(SCAR_MAX_CLIP, SCAR_RELOAD, 1.5);
}


void CScar::WeaponIdle(void)
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > gpGlobals->time)
		return;

	// only idle if the slid isn't back
	if (m_iClip != 0)
	{
		int iAnim;
		float flRand = RANDOM_FLOAT(0, 1);
		if (flRand <= 0.3 + 0 * 0.75)
		{
			iAnim = SCAR_IDLE1;
			m_flTimeWeaponIdle = gpGlobals->time + 49.0 / 16;
		}
		SendWeaponAnim(iAnim);
	}
}













