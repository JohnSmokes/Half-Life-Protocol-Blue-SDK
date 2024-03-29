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
* ---------------------------------------------------------------------------------------------------------------
* Half-Life: Another Shift Flare Code - by Bacontsu
* Modified by ConTraZVII
* ---------------------------------------------------------------------------------------------------------------
****/

#define FLARE_MAX_LIFE	250

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"


// =================================================================================================================
// Flare - holdable weapon
// =================================================================================================================
class CFlare : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 1; }
	int GetItemInfo(ItemInfo *p);

	void PrimaryAttack( void );
	BOOL Deploy( void );
	void Holster( void );
	void SecondaryAttack(void);
	void WeaponIdle(void);

	void EmitLight(void);

	float m_flNextFlareSound;
	int m_iFlareType;
	float m_flNextFlareType;
	float m_flNextIdleAnim;
};
LINK_ENTITY_TO_CLASS( weapon_flare, CFlare);



enum gauss_e {
	CROWBAR_IDLE = 0,
	CROWBAR_DRAW,
	CROWBAR_HOLSTER,
};


void CFlare::Spawn( )
{
	Precache( );
	m_iId = WEAPON_FLARE;
	pev->classname = MAKE_STRING("weapon_flare");
	SET_MODEL(ENT(pev), "models/w_flarewep.mdl");
	
	m_iDefaultAmmo = 1;

	FallInit();// get ready to fall down.
}


void CFlare::Precache( void )
{
	PRECACHE_MODEL("models/v_flare.mdl");
	PRECACHE_MODEL("models/w_flarewep.mdl");
	PRECACHE_MODEL("models/p_crowbar.mdl");
	PRECACHE_SOUND("weapons/flare_burning.wav");
}

int CFlare::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "flare_ammo";
	p->iMaxAmmo1 = 5;
	p->pszAmmo2 = "flare_ammo2";
	p->iMaxAmmo2 = FLARE_MAX_LIFE;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 1;
	p->iId = WEAPON_FLARE;
	p->iWeight = CROWBAR_WEIGHT;

	return 1;
}



BOOL CFlare::Deploy( )
{
	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = FLARE_MAX_LIFE;

	return DefaultDeploy( "models/v_flare.mdl", "models/w_flarewep.mdl", CROWBAR_DRAW, "flare" );
}

void CFlare::Holster( )
{
	m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5;
	SendWeaponAnim( CROWBAR_HOLSTER );
}

void CFlare::EmitLight()
{
}

void CFlare::PrimaryAttack()
{	
	if (!m_pPlayer->m_afButtonPressed & IN_ATTACK)
		return;

	Vector vecSrc = m_pPlayer->pev->origin;
	Vector vecThrow = gpGlobals->v_forward * 274 + m_pPlayer->pev->velocity;

	CBaseEntity* pFlare = Create("item_flare", vecSrc, Vector(0, 0, 0), m_pPlayer->edict());
	pFlare->pev->velocity = vecThrow;

	SendWeaponAnim(CROWBAR_DRAW);
	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = FLARE_MAX_LIFE;
	m_flNextPrimaryAttack = m_flTimeWeaponIdle = gpGlobals->time + 1;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 0)
		RetireWeapon();
}

void CFlare::SecondaryAttack()
{
	//EmitLight();
}

void CFlare::WeaponIdle()
{
	if (m_flTimeWeaponIdle > gpGlobals->time)
		return;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		RetireWeapon();
		return;
	}

	if (pev->dmgtime < gpGlobals->time)
	{
		SendWeaponAnim(CROWBAR_IDLE);
		pev->dmgtime = gpGlobals->time + 3;
	}

	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
	m_flTimeWeaponIdle = gpGlobals->time + 0.1;

	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 0)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = FLARE_MAX_LIFE;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
		m_flNextPrimaryAttack = m_flTimeWeaponIdle = gpGlobals->time + 1;
		SendWeaponAnim(CROWBAR_DRAW);
	}

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 0)
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;

	if (m_flNextFlareType < gpGlobals->time)
	{
		if (m_iFlareType < 6)
			m_iFlareType++;
		else
			m_iFlareType = 0;

		m_flNextFlareType = gpGlobals->time + 0.1f;
	}

	// emit dlight
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector src = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_DLIGHT);
	WRITE_COORD(src.x); // origin
	WRITE_COORD(src.y);
	WRITE_COORD(src.z);

	if (m_iFlareType < 3)
		WRITE_BYTE(15 + m_iFlareType);     // radius
	else
		WRITE_BYTE(15 + 5 - m_iFlareType);

	WRITE_BYTE(255);     // R old - 200 150 150
	WRITE_BYTE(160);     // G
	WRITE_BYTE(0);     // B
	WRITE_BYTE(1);     // life * 10
	WRITE_BYTE(0); // decay
	MESSAGE_END();

	// emit sound
	if (m_flNextFlareSound < gpGlobals->time)
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/flare_burning.wav", 5, ATTN_NORM, 0, 95 + RANDOM_LONG(0, 0x1f));
		m_flNextFlareSound = gpGlobals->time + 2.0f;
	}
}


// =================================================================================================================
// Flare - world item
// =================================================================================================================
class CFlareItem : public CBaseEntity
{
	void Spawn(void);
	void Precache(void);
	void BounceSound(void);

	void EXPORT FlareSlide(CBaseEntity* pOther);
	void EXPORT FlareThink(void);
	void EXPORT FlareUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() | FCAP_IMPULSE_USE; }

public:
	void Deactivate(void);

	float lerp[2];
	float m_flNextFlareSound;
	int m_iFlareType;
	float m_flNextFlareType;
};
LINK_ENTITY_TO_CLASS(item_flare, CFlareItem);

//=========================================================
// Deactivate - do whatever it is we do to an orphaned 
// satchel when we don't want it in the world anymore.
//=========================================================
void CFlareItem::Deactivate(void)
{
	pev->solid = SOLID_NOT;
	UTIL_Remove(this);
}


void CFlareItem::Spawn(void)
{
	Precache();
	// motor
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/w_flarewep.mdl");
	//UTIL_SetSize(pev, Vector( -16, -16, -4), Vector(16, 16, 32));	// Old box -- size of headcrab monsters/players get blocked by this
	UTIL_SetSize(pev, Vector(-4, -4, -4), Vector(4, 4, 4));	// Uses point-sized, and can be stepped over
	UTIL_SetOrigin(pev, pev->origin);

	SetTouch(&CFlareItem::FlareSlide);
	SetThink(&CFlareItem::FlareThink);
	SetUse(&CFlareItem::FlareUse);
	pev->nextthink = gpGlobals->time + 0.001;

	pev->gravity = 0.5;
	pev->friction = 0.8;

	// ResetSequenceInfo( );
	pev->sequence = 1;
	pev->dmgtime = gpGlobals->time + 120;
}


void CFlareItem::FlareSlide(CBaseEntity* pOther)
{
	entvars_t* pevOther = pOther->pev;

	// don't hit the guy that launched this grenade
	if (pOther->edict() == pev->owner)
		return;

	// pev->avelocity = Vector (300, 300, 300);
	pev->gravity = 1;// normal gravity now

	// HACKHACK - On ground isn't always set, so look for ground underneath
	TraceResult tr;
	UTIL_TraceLine(pev->origin, pev->origin - Vector(0, 0, 10), ignore_monsters, edict(), &tr);

	if (tr.flFraction < 1.0)
	{
		// add a bit of static friction
		pev->velocity = pev->velocity * 0.95;
		//pev->avelocity = pev->avelocity * 0.9;
		// play sliding sound, volume based on velocity
	}
	if (!(pev->flags & FL_ONGROUND) && pev->velocity.Length2D() > 10)
	{
		BounceSound();
	}

}

void CFlareItem::FlareThink(void)
{
	CBaseEntity* pEntity = NULL;	// iterate on all entities in the vicinity.
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 64)) != NULL)
	{
		if (FClassnameIs(pEntity->pev, "monster_zombie")
			|| FClassnameIs(pEntity->pev, "monster_zombie_barney")
			|| FClassnameIs(pEntity->pev, "monster_zombie_soldier"))
		{
			CBaseMonster* pTarget = pEntity->MyMonsterPointer();
			pTarget->m_bIsOnFire = TRUE;
		}
	}

	pev->nextthink = gpGlobals->time + 0.001;

	if (!IsInWorld())
	{
		UTIL_Remove(this);
		return;
	}

	if (pev->waterlevel == 3)
	{
		pev->movetype = MOVETYPE_FLY;
		pev->velocity = pev->velocity * 0.8;
		//pev->avelocity = pev->avelocity * 0.9;
		pev->velocity.z += 8;
	}
	else if (pev->waterlevel == 0)
	{
		pev->movetype = MOVETYPE_BOUNCE;
	}
	else
	{
		pev->velocity.z -= 8;
	}

	if ((!(pev->flags & FL_ONGROUND)))
	{
		pev->avelocity = pev->velocity;
		lerp[0] = pev->angles.x;
		lerp[1] = pev->angles.y;
	}

	if (pev->flags & FL_ONGROUND)
	{
		// ALERT(at_console, "angles %f %f \n", pev->angles[0], pev->angles[2]);
		lerp[0] = (0 * 0.03f) + (lerp[0] * (1.0 - 0.1f));
		pev->angles.x = lerp[0];

		lerp[1] = (0 * 0.03f) + (lerp[1] * (1.0 - 0.1f));
		pev->angles.z = lerp[1];
	}

	// emit sound
	if (m_flNextFlareSound < gpGlobals->time)
	{
		EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, "weapons/flare_burning.wav", 5, ATTN_NORM, 0, 95 + RANDOM_LONG(0, 0x1f));
		m_flNextFlareSound = gpGlobals->time + 2.0f;
	}

	// flickering effect
	if (m_flNextFlareType < gpGlobals->time)
	{
		if (m_iFlareType < 6)
			m_iFlareType++;
		else
			m_iFlareType = 0;

		m_flNextFlareType = gpGlobals->time + 0.1f;
	}

	// emit dlight
	Vector src = pev->origin;

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_DLIGHT);
	WRITE_COORD(src.x); // origin
	WRITE_COORD(src.y);
	WRITE_COORD(src.z);

	if (m_iFlareType < 3)
		WRITE_BYTE(13 + m_iFlareType);     // radius
	else
		WRITE_BYTE(13 + 5 - m_iFlareType);

	WRITE_BYTE(255);     // R old - 200 150 150
	WRITE_BYTE(160);     // G
	WRITE_BYTE(0);     // B
	WRITE_BYTE(0.1f);     // life * 10
	WRITE_BYTE(0); // decay
	MESSAGE_END();

	UTIL_Sparks(pev->origin);
	
	if (pev->dmgtime < gpGlobals->time)
	{
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_SMOKE);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_SHORT(g_sModelIndexSmoke);
		WRITE_BYTE(25); // scale * 10
		WRITE_BYTE(15); // framerate
		MESSAGE_END();

		UTIL_Remove(this);
	}
}

void CFlareItem::FlareUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	/*
	if (!pActivator->IsPlayer())
		return;

	CBasePlayer* pPlayer = static_cast<CBasePlayer*>(pActivator);
	if (pPlayer->HasNamedPlayerItem("weapon_flare"))
		pPlayer->GiveAmmo(pev->weapons, "flare_ammo", 250);
	else
		pPlayer->GiveNamedItem("weapon_flare");

	UTIL_Remove(this);
	*/
}

void CFlareItem::Precache(void)
{
	PRECACHE_MODEL("models/w_flarewep.mdl");
	PRECACHE_SOUND("weapons/g_bounce1.wav");
	PRECACHE_SOUND("weapons/g_bounce2.wav");
	PRECACHE_SOUND("weapons/g_bounce3.wav");
}

void CFlareItem::BounceSound(void)
{
	switch (RANDOM_LONG(0, 2))
	{
	case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/g_bounce1.wav", 1, ATTN_NORM);	break;
	case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/g_bounce2.wav", 1, ATTN_NORM);	break;
	case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/g_bounce3.wav", 1, ATTN_NORM);	break;
	}
}
