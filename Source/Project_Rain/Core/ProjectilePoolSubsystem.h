// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ProjectilePoolSubsystem.generated.h"

/**
 * 
 */

class AMonsterProjectile;

USTRUCT()
struct FProjectilePoolArray
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<AMonsterProjectile*> Pool;
};

UCLASS()
class PROJECT_RAIN_API UProjectilePoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	AMonsterProjectile* SpawnFromPool(TSubclassOf<AMonsterProjectile> ProjectileClass, FVector Location, FRotator Rotation, APawn* Instigator);
private:
	UPROPERTY()
	TMap<UClass*, FProjectilePoolArray> ProjectilePools;
};
