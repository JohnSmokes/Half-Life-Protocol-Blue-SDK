//Thx Bacon Thx Bacon Thx Bacon 

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "player.h"

class CPropPhysics : public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override;
	void Killed(entvars_t* pevAttacker, int iGib) override;
	void BounceSound();

	EXPORT void PropThink();
	EXPORT void PropTouch(CBaseEntity* pOther);
	EXPORT void PropUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	virtual int	ObjectCaps(void)
	{
		return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_CONTINUOUS_USE;
	}

	char PROPMODEL[512];
	int m_iShard;
	CBasePlayer* m_pPlayer = nullptr;
	float usedTime = 0.0f;
	float lerp[2];
};

LINK_ENTITY_TO_CLASS(prop_physics, CPropPhysics);

void CPropPhysics::Spawn()
{
	strcpy(PROPMODEL, (char*)STRING(pev->model));

	Precache();

	SET_MODEL(ENT(pev), PROPMODEL);
	//UTIL_SetSize(pev, Vector(-8, -8, 0), Vector(8, 8, 16));
	UTIL_SetSize(pev, Vector(-25, -20, 0), Vector(25, 0, 50));

	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;
	pev->takedamage = DAMAGE_YES;
	pev->health = 2500;

	SetThink(&CPropPhysics::PropThink);
	SetTouch(&CPropPhysics::PropTouch);
	SetUse(&CPropPhysics::PropUse);

	pev->nextthink = gpGlobals->time + 0.001f;
	//SetNextThink(0.001f);
}

void CPropPhysics::Precache()
{
	PRECACHE_MODEL(PROPMODEL);
	m_iShard = PRECACHE_MODEL("models/woodgibs.mdl");
	PRECACHE_SOUND("weapons/g_bounce1.wav");
	PRECACHE_SOUND("weapons/g_bounce2.wav");
	PRECACHE_SOUND("weapons/g_bounce3.wav");
}

void CPropPhysics::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	if (flDamage > 0)
	{
		UTIL_Sparks(Center());

		pev->velocity = g_vecAttackDir * -1;
		//pev->velocity = pev->velocity * 1000;
	}

	return CBaseEntity::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
}

void CPropPhysics::PropThink()
{
	pev->health = 2500;
	// ALERT(at_console, "angles %f %f %f", pev->angles.x, pev->angles.y, pev->angles.z);

	// HACKHACK - On ground isn't always set, so look for ground underneath
	TraceResult tr;
	UTIL_TraceLine(pev->origin, pev->origin + Vector(0, 0, pev->mins.z - 10), ignore_monsters, edict(), &tr);

	if (tr.flFraction < 1.0)
	{
		float target[2] = { 0, 0 };

		// these funny physics thing isnt ready to be used
		/*
		if (pev->angles.x >= 0 && pev->angles.x < 45)
			target[0] = 0;
		else if (pev->angles.x >= 45 && pev->angles.x < 135)
			target[0] = 90;
		else if (pev->angles.x >= 135 && pev->angles.x < 180)
			target[0] = 180;
		else if (pev->angles.x <= -180 && pev->angles.x > -135)
			target[0] = -179;
		else if (pev->angles.x <= -135 && pev->angles.x > -45)
			target[0] = -90;
		else if (pev->angles.x >= -45 && pev->angles.x < 0)
			target[0] = -1;
		if (pev->angles.z >= 0 && pev->angles.z < 45)
			target[1] = 0;
		else if (pev->angles.z >= 45 && pev->angles.z < 135)
			target[1] = 90;
		else if (pev->angles.z >= 135 && pev->angles.z < 180)
			target[1] = 180;
		else if (pev->angles.z <= -180 && pev->angles.z > -135)
			target[1] = -179;
		else if (pev->angles.z <= -135 && pev->angles.z > -45)
			target[1] = -90;
		else if (pev->angles.z >= -45 && pev->angles.z < 0)
			target[1] = -1;
		if (pev->angles.x == -1) { pev->angles.x = 0; target[0] == 0; }
		if (pev->angles.z == -1) { pev->angles.z = 0; target[1] == 0; }
		*/

		lerp[0] = (target[0] * 0.03f * 300 * gpGlobals->frametime) + (lerp[0] * (1.0 - 0.03f * 300 * gpGlobals->frametime));
		pev->angles.x = lerp[0];

		lerp[1] = (target[1] * 0.03f * 300 * gpGlobals->frametime) + (lerp[1] * (1.0 - 0.03f * 300 * gpGlobals->frametime));
		pev->angles.z = lerp[1];

	}
	else
	{
		if (usedTime < gpGlobals->time)
			pev->avelocity = pev->velocity / 5; // divide by 2 when I finished the physics formula

		lerp[0] = pev->angles.x;
		lerp[1] = pev->angles.z;
	}

	if (m_pPlayer && usedTime > gpGlobals->time)
	{
		pev->angles.x = pev->angles.z = 0;
		pev->angles.y = m_pPlayer->pev->angles.y;
	}

	pev->nextthink = gpGlobals->time + 0.001f;
}

void CPropPhysics::PropTouch(CBaseEntity* pOther)
{
	entvars_t* pevToucher = pOther->pev;

	if (FClassnameIs(pevToucher, "player") && VARS(pOther->pev->groundentity) != this->pev)
	{
		//pev->velocity = pOther->pev->velocity * 10;
		Vector dist = pev->origin - pOther->pev->origin;

		pev->velocity = pev->velocity + dist.Normalize() * 10;
		pev->velocity.z = 20;

		//ALERT(at_console, "balls");
		return;
	}

	pev->velocity = pev->velocity / 2;

	if (pev->velocity.Length() > 256)
		pOther->TakeDamage(pev, pev, 100, DMG_CLUB);

	if (!(pev->flags & FL_ONGROUND) && pev->velocity.Length2D() > 10)
	{
		BounceSound();
	}
}

void CPropPhysics::PropUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (FClassnameIs(pActivator->pev, "player"))
	{
		// experimental pull feature
		if (pActivator->pev->flags & FL_ONGROUND && VARS(pActivator->pev->groundentity) != this->pev)
		{
			UTIL_MakeVectors(pActivator->pev->v_angle);
			Vector vecSrc = pActivator->pev->origin + Vector(0, 0, 36);
			Vector vecEnd = vecSrc + gpGlobals->v_forward * 80;
			TraceResult tr;
			UTIL_TraceLine(vecSrc, vecEnd, ignore_monsters, dont_ignore_glass, ENT(pActivator->pev), &tr);

			pev->velocity.x = (tr.vecEndPos.x - pev->origin.x) * 10;
			pev->velocity.y = (tr.vecEndPos.y - pev->origin.y) * 10;
			pev->velocity.z = (tr.vecEndPos.z - pev->origin.z) * 10; // slightly lift it up due to fraction

			m_pPlayer = (CBasePlayer*)pActivator;
			usedTime = gpGlobals->time + 0.01f;;
		}
	}
}

void CPropPhysics::Killed(entvars_t* pevAttacker, int iGib)
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, Center());
	WRITE_BYTE(TE_BREAKMODEL);

	// position
	WRITE_COORD(Center().x);
	WRITE_COORD(Center().y);
	WRITE_COORD(Center().z);

	// size
	WRITE_COORD(pev->size.x); 
	WRITE_COORD(pev->size.y); 
	WRITE_COORD(pev->size.z);

	// velocity
	WRITE_COORD(pev->velocity.x);
	WRITE_COORD(pev->velocity.y);
	WRITE_COORD(pev->velocity.z);

	// randomization
	WRITE_BYTE(10);

	// Model
	WRITE_SHORT(m_iShard); //model id#

	// # of shards
	WRITE_BYTE(0); // let client decide

	// duration
	WRITE_BYTE(25); // 2.5 seconds

	// flags
	WRITE_BYTE(BREAK_WOOD);
	MESSAGE_END();

	// Fire targets on break
	SUB_UseTargets(NULL, USE_TOGGLE, 0);

	return CBaseEntity::Killed(pevAttacker, iGib);
}

void CPropPhysics::BounceSound()
{
	switch (RANDOM_LONG(0, 2))
	{
	case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/g_bounce1.wav", 1, ATTN_NORM);	break;
	case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/g_bounce2.wav", 1, ATTN_NORM);	break;
	case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/g_bounce3.wav", 1, ATTN_NORM);	break;
	}
}