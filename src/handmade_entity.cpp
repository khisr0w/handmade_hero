/*  +======| File Info |===============================================================+
    |                                                                                  |
    |     Subdirectory:  /src                                                          |
    |    Creation date:  Undefined                                                     |
    |    Last Modified:  11/27/2020 5:09:12 AM                                         |
    |                                                                                  |
    +=====================| Sayed Abid Hashimi, Copyright © All rights reserved |======+  */

inline move_spec
DefaultMoveSpec()
{
	move_spec Result;
	Result.UnitMaxAccelVector = false;
	Result.Speed = 1.0f;
	Result.Drag = 0.0f;

	return Result;
}

inline void 
UpdateFamiliar(sim_region *SimRegion, sim_entity *Entity, real32 dt)
{
	sim_entity *ClosestHero = 0;
	real32 ClosestHeroDSq = Square(10.f);

	sim_entity *TestEntity = SimRegion->Entities;
	for(uint32_t TestEntityIndex = 0;
		TestEntityIndex < SimRegion->MaxEntityCount;
		++TestEntityIndex, ++TestEntity)
	{
		if(TestEntity->Type == EntityType_Hero)
		{
			real32 TestDSq = LengthSq(TestEntity->P - Entity->P);
			if(TestEntity->Type == EntityType_Hero)
			{
				TestDSq *= 0.75f;
			}
			if(ClosestHeroDSq > TestDSq)
			{
				ClosestHero = TestEntity;
				ClosestHeroDSq = TestDSq;
			}
		}
	}

	v2 ddP = {};
	if(ClosestHero && (ClosestHeroDSq > Square(3.01f)))
	{
		// TODO PULL THE SPEED OUT OF MOVE ENTITY
		real32 Accleration = 0.5f;
		real32 OneOverLength = Accleration / SquareRoot(ClosestHeroDSq);
		ddP = OneOverLength*(ClosestHero->P - Entity->P);
	}

	move_spec MoveSpec = DefaultMoveSpec();
	MoveSpec.Speed = 50.0f;
	MoveSpec.UnitMaxAccelVector = true;
	MoveSpec.Drag = 8.0f;

	MoveEntity(SimRegion, Entity, dt, &MoveSpec, ddP);
}

inline void 
UpdateMonstar(sim_region *SimRegion, sim_entity *Entity, real32 dt)
{

}

inline void 
UpdateSword(sim_region *SimRegion, sim_entity *Entity, real32 dt)
{
	if(IsSet(Entity, EntityFlag_Nonspatial))
	{

	}
	else
	{
		move_spec MoveSpec = DefaultMoveSpec();
		MoveSpec.Speed = 0.0f;
		MoveSpec.UnitMaxAccelVector = false;
		MoveSpec.Drag = 0.0f;

		v2 OldP = Entity->P;
		MoveEntity(SimRegion, Entity, dt, &MoveSpec, V2(0, 0));
		real32 DistanceTraveled = Length(Entity->P - OldP);

		Entity->DistanceRemaining -= DistanceTraveled;
		if(Entity->DistanceRemaining < 0.0f)
		{
			MakeEntityNonSpatial(Entity);
		}
	}
}

