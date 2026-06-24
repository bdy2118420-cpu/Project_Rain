#include "ProjectilePoolSubsystem.h"
#include "../Projectiles/MonsterProjectile.h"
#include "Engine/World.h"

AMonsterProjectile* UProjectilePoolSubsystem::SpawnFromPool(TSubclassOf<AMonsterProjectile> ProjectileClass, FVector Location, FRotator Rotation, APawn* Instigator)
{
	if (!ProjectileClass) return nullptr;
	FProjectilePoolArray& PoolStruct = ProjectilePools.FindOrAdd(ProjectileClass);

	for (AMonsterProjectile* Proj : PoolStruct.Pool)
	{
		if (Proj && !Proj->bIsActive)
		{
			Proj->ActivateProjectile(Location, Rotation, Instigator);
			return Proj;
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMonsterProjectile* NewProj = GetWorld()->SpawnActor<AMonsterProjectile>(ProjectileClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (NewProj)
	{
		PoolStruct.Pool.Add(NewProj);
		NewProj->ActivateProjectile(Location, Rotation, Instigator);
		return NewProj;
	}

	return nullptr;
}