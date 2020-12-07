/*  +======| File Info |===============================================================+
    |                                                                                  |
    |     Subdirectory:  /src                                                          |
    |    Creation date:  Undefined                                                     |
    |    Last Modified:  12/7/2020 4:41:36 PM                                          |
    |                                                                                  |
    +=====================| Sayed Abid Hashimi, Copyright � All rights reserved |======+  */

#if !defined(HANDMADE_ENTITY_H)

#define InvalidP V3(100000.0f, 100000.0f, 100000.0f)

inline bool32
IsSet(sim_entity *Entity, uint32_t Flag)
{
	bool32 Result = Entity->Flags & Flag;
	
	return Result;
}

inline void
AddFlag(sim_entity *Entity, uint32_t Flag)
{
	Entity->Flags |= Flag;
}

inline void
ClearFlag(sim_entity *Entity, uint32_t Flag)
{
	Entity->Flags &= ~Flag;
}

inline void
MakeEntityNonSpatial(sim_entity *Entity)
{
	AddFlag(Entity, EntityFlag_Nonspatial);
	Entity->P = InvalidP;
}

inline void
MakeEntitySpatial(sim_entity *Entity, v3 P, v3 dP)
{
	ClearFlag(Entity, EntityFlag_Nonspatial);
	Entity->P = P;
	Entity->dP = dP;
}

#define HANDMADE_ENTITY_H
#endif