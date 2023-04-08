///***
//*
//*	CopyEthan (e) 2023, Ethan, Ethan.
//*
//****/
////=========================================================
//// Ethan - that thing you collect for some reason
////=========================================================
//
//#include "extdll.h"
//#include "util.h"
//#include "cbase.h"
//#include "player.h"
//#include "skill.h"
//
//extern int gmsgGem;
//
//class CItemGem : public CBaseEntity
//{
//public:
//	void	Spawn(void);
//	void	EXPORT ItemTouch(CBaseEntity* pOther);
//	BOOL	MyTouch(CBasePlayer* pPlayer);
//	void	EXPORT Think(void);
//};
//
//LINK_ENTITY_TO_CLASS(env_gem, CItemGem);
//
//LINK_ENTITY_TO_CLASS(env_gem_green, CItemGem);
//LINK_ENTITY_TO_CLASS(env_gem_yellow, CItemGem);
//LINK_ENTITY_TO_CLASS(env_gem_blue, CItemGem);
//LINK_ENTITY_TO_CLASS(env_gem_purple, CItemGem);
//
///*
//LINK_ENTITY_TO_CLASS(weapon_9mmhandgun, CItemGem);
//LINK_ENTITY_TO_CLASS(ammo_9mmAR, CItemGem);
//LINK_ENTITY_TO_CLASS(ammo_9mmclip, CItemGem);
//LINK_ENTITY_TO_CLASS(ammo_glockclip, CItemGem);
//
//LINK_ENTITY_TO_CLASS(weapon_glock, CItemGem);
//LINK_ENTITY_TO_CLASS(ammo_9mmbox, CItemGem);
//LINK_ENTITY_TO_CLASS(weapon_9mmAR, CItemGem);
//LINK_ENTITY_TO_CLASS(weapon_shotgun, CItemGem);
//LINK_ENTITY_TO_CLASS(ammo_buckshot, CItemGem);
//LINK_ENTITY_TO_CLASS(weapon_handgrenade, CItemGem);
//LINK_ENTITY_TO_CLASS(ammo_mp5clip, CItemGem);
//
//LINK_ENTITY_TO_CLASS(ammo_357, CItemGem);
//LINK_ENTITY_TO_CLASS(weapon_357, CItemGem);
//LINK_ENTITY_TO_CLASS(ammo_ARgrenades, CItemGem);
//LINK_ENTITY_TO_CLASS(weapon_satchel, CItemGem);
//LINK_ENTITY_TO_CLASS(weapon_tripmine, CItemGem);
//
//LINK_ENTITY_TO_CLASS(weapon_crossbow, CItemGem);
//LINK_ENTITY_TO_CLASS(ammo_crossbow, CItemGem);
//LINK_ENTITY_TO_CLASS(ammo_gaussclip, CItemGem);
//LINK_ENTITY_TO_CLASS(ammo_rpgclip, CItemGem);
//LINK_ENTITY_TO_CLASS(weapon_snark, CItemGem);
//LINK_ENTITY_TO_CLASS(weapon_crowbar, CItemGem);
//
//LINK_ENTITY_TO_CLASS(weapon_rpg, CItemGem);
//LINK_ENTITY_TO_CLASS(weapon_hornetgun, CItemGem);
//LINK_ENTITY_TO_CLASS(weapon_egon, CItemGem);
//LINK_ENTITY_TO_CLASS(weapon_gauss, CItemGem);
//*/
//
//
//void CItemGem::Spawn(void)
//{
//	Precache();
//
//	SET_MODEL(ENT(pev), "models/superbarney.mdl");
//
//	UTIL_SetSize(pev, Vector(-2, -2, 0), Vector(2, 2, 6));
//
//	pev->movetype = MOVETYPE_BOUNCE;
//	pev->solid = SOLID_TRIGGER;
//
//	pev->friction = 0.4;
//	pev->framerate = 1.0;
//	pev->sequence = 0;
//
//	pev->velocity = g_vecZero;
//	pev->velocity = gpGlobals->v_up * 235;
//
//	//SetThink( SUB_DoNothing );
//	SetTouch(ItemTouch);
//
//	if (DROP_TO_FLOOR(ENT(pev)) == 0)
//	{
//		ALERT(at_error, "Item %s fell out of level at %f,%f,%f\n", STRING(pev->classname), pev->origin.x, pev->origin.y, pev->origin.z);
//		UTIL_Remove(this);
//		return;
//	}
//
//	if (FClassnameIs(pev, "env_gem_green"))
//	{
//		pev->skin = 1;
//		pev->body = 1;
//	}
//	if (FClassnameIs(pev, "env_gem_yellow"))
//	{
//		pev->skin = 2;
//		pev->body = 2;
//	}
//	if (FClassnameIs(pev, "env_gem_blue"))
//	{
//		pev->skin = 3;
//		pev->body = 3;
//	}
//	if (FClassnameIs(pev, "env_gem_purple"))
//	{
//		pev->skin = 4;
//		pev->body = 4;
//	}
//	/*
//		if ( (FClassnameIs(pev, "env_gem_green")) || (FClassnameIs(pev, "ammo_9mmAR")) || (FClassnameIs(pev, "ammo_9mmbox")) ||
//			 (FClassnameIs(pev, "weapon_9mmAR")) || (FClassnameIs(pev, "weapon_glock")) || (FClassnameIs(pev, "weapon_shotgun")) ||
//			 (FClassnameIs(pev, "ammo_buckshot")) || (FClassnameIs(pev, "weapon_handgrenade")) || (FClassnameIs(pev, "ammo_mp5clip")) )
//		{
//			pev->skin = 1;
//			pev->body = 1;
//		}
//		if ( (FClassnameIs(pev, "env_gem_yellow")) || (FClassnameIs(pev, "ammo_357")) || (FClassnameIs(pev, "weapon_357")) ||
//			 (FClassnameIs(pev, "ammo_ARgrenades")) || (FClassnameIs(pev, "weapon_satchel")) || (FClassnameIs(pev, "weapon_tripmine")) )
//		{
//			pev->skin = 2;
//			pev->body = 2;
//		}
//		if ( (FClassnameIs(pev, "env_gem_blue")) || (FClassnameIs(pev, "weapon_crossbow")) || (FClassnameIs(pev, "ammo_crossbow")) ||
//			 (FClassnameIs(pev, "ammo_gaussclip")) || (FClassnameIs(pev, "ammo_rpgclip")) || (FClassnameIs(pev, "weapon_snark")) ||
//			 (FClassnameIs(pev, "weapon_crowbar")) )
//		{
//			pev->skin = 3;
//			pev->body = 3;
//		}
//		if ( (FClassnameIs(pev, "env_gem_purple")) || (FClassnameIs(pev, "weapon_rpg")) || (FClassnameIs(pev, "weapon_hornetgun")) ||
//			 (FClassnameIs(pev, "weapon_egon")) || (FClassnameIs(pev, "weapon_gauss")) )
//		{
//			pev->skin = 4;
//			pev->body = 4;
//		}
//	*/
//	UTIL_SetOrigin(pev, pev->origin);
//
//	pev->nextthink = gpGlobals->time;
//}
//
//BOOL CItemGem::MyTouch(CBasePlayer* pPlayer)
//{
//	if (pPlayer->m_iGemsCount > 9999)
//		return FALSE;
//
//	if (pev->skin == 4)
//		pPlayer->m_iGemsCount = pPlayer->m_iGemsCount + 25;
//	else if (pev->skin == 3)
//		pPlayer->m_iGemsCount = pPlayer->m_iGemsCount + 10;
//	else if (pev->skin == 2)
//		pPlayer->m_iGemsCount = pPlayer->m_iGemsCount + 5;
//	else if (pev->skin == 1)
//		pPlayer->m_iGemsCount = pPlayer->m_iGemsCount + 2;
//	else
//		pPlayer->m_iGemsCount++;
//
//	//	MESSAGE_BEGIN( MSG_ONE, gmsgGem, NULL, pPlayer->pev );
//	//		WRITE_SHORT( pPlayer->m_iGemsCount );
//	//	MESSAGE_END();
//
//	UTIL_Sparks(pev->origin);
//
//	EMIT_SOUND(ENT(pev), CHAN_ITEM, "player/GemGet1.wav", 1, ATTN_NORM);
//	return TRUE;
//}
//
//void CItemGem::ItemTouch(CBaseEntity* pOther)
//{
//	pev->velocity.x = 0;
//	pev->velocity.y = 0;
//
//	// if it's not a player, ignore
//	if (!pOther->IsPlayer())
//	{
//		return;
//	}
//
//	CBasePlayer* pPlayer = (CBasePlayer*)pOther;
//
//	if (MyTouch(pPlayer))
//	{
//		SUB_UseTargets(pOther, USE_TOGGLE, 0);
//		SetTouch(NULL);
//		UTIL_Remove(this);
//	}
//}
//
//void CItemGem::Think(void)
//{
//	if (!IsInWorld())
//		UTIL_Remove(this);
//
//	pev->nextthink = gpGlobals->time;
//
//	if (FBitSet(pev->flags, FL_ONGROUND))
//	{
//	}
//	else
//	{
//		pev->velocity.x = 0;
//		pev->velocity.y = 0;
//	}
//
//	pev->framerate = 1.0;
//}