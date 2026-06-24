#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterDirector.generated.h"

class ABaseMonster;

USTRUCT(BlueprintType)
struct FMonsterSpawnCard
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ABaseMonster> MonsterClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDifficulty = 1.0f;
};

USTRUCT()
struct FMonsterPoolArray
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<ABaseMonster*> Pool;
};

UCLASS()
class PROJECT_RAIN_API AMonsterDirector : public AActor
{
	GENERATED_BODY()

public:
	AMonsterDirector();

protected:
	virtual void BeginPlay() override;

	void SpawnMonster();

	UPROPERTY(EditAnywhere, Category = "Director")
	TArray<FMonsterSpawnCard> SpawnDeck;


	UPROPERTY(EditAnywhere, Category = "Director")
	int32 PoolSizePerType = 15;


	UPROPERTY(EditAnywhere, Category = "Director")
	float BaseSpawnInterval = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Director")
	float SpawnRadius = 1500.0f;


	UPROPERTY()
	TMap<UClass*, FMonsterPoolArray> MonsterPools;

	FTimerHandle SpawnTimerHandle;
};