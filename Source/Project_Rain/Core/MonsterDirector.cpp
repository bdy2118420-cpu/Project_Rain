#include "MonsterDirector.h"
#include "../Characters/BaseMonster.h"
#include "MyGameState.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"

AMonsterDirector::AMonsterDirector()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMonsterDirector::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && SpawnDeck.Num() > 0)
	{
		for (const FMonsterSpawnCard& Card : SpawnDeck)
		{
			if (!Card.MonsterClass) continue;

			FMonsterPoolArray& PoolStruct = MonsterPools.FindOrAdd(Card.MonsterClass);

			for (int32 i = 0; i < PoolSizePerType; i++)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				ABaseMonster* NewMonster = GetWorld()->SpawnActor<ABaseMonster>(Card.MonsterClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				if (NewMonster)
				{
					NewMonster->DeactivateMonster();
					PoolStruct.Pool.Add(NewMonster);
				}
			}
		}

		GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &AMonsterDirector::SpawnMonster, BaseSpawnInterval, false);
	}
}

void AMonsterDirector::SpawnMonster()
{
	AMyGameState* GS = GetWorld()->GetGameState<AMyGameState>();
	float CurrentCoeff = GS ? GS->GetDifficultyCoefficient() : 1.0f;

	TArray<APawn*> AlivePlayers;
	if (GS)
	{
		for (APlayerState* PS : GS->PlayerArray)
		{
			if (PS && PS->GetPawn())
			{
				AlivePlayers.Add(PS->GetPawn());
			}
		}
	}

	if (AlivePlayers.Num() == 0)
	{
		GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &AMonsterDirector::SpawnMonster, BaseSpawnInterval, false);
		return;
	}

	int32 PlayerCount = AlivePlayers.Num();
	APawn* TargetPlayer = AlivePlayers[FMath::RandRange(0, PlayerCount - 1)];

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());

	if (TargetPlayer && NavSystem && SpawnDeck.Num() > 0)
	{
		TArray<FMonsterSpawnCard> AvailableCards;
		for (const FMonsterSpawnCard& Card : SpawnDeck)
		{
			if (CurrentCoeff >= Card.MinDifficulty)
			{
				AvailableCards.Add(Card);
			}
		}

		if (AvailableCards.Num() > 0)
		{
			int32 RandomIndex = FMath::RandRange(0, AvailableCards.Num() - 1);
			TSubclassOf<ABaseMonster> SelectedClass = AvailableCards[RandomIndex].MonsterClass;

			FNavLocation RandomLocation;
			if (NavSystem->GetRandomReachablePointInRadius(TargetPlayer->GetActorLocation(), SpawnRadius, RandomLocation))
			{
				FMonsterPoolArray* PoolStruct = MonsterPools.Find(SelectedClass);
				if (PoolStruct)
				{
					for (ABaseMonster* Monster : PoolStruct->Pool)
					{
						if (Monster && !Monster->bIsActive)
						{
							FVector Direction = (TargetPlayer->GetActorLocation() - RandomLocation.Location).GetSafeNormal();
							Direction.Z = 0.0f;

							Monster->ActivateMonster(RandomLocation.Location, Direction.Rotation(), CurrentCoeff);
							break; 
						}
					}
				}
			}
		}
	}

	float BaseDelay = BaseSpawnInterval / CurrentCoeff;
	float NextSpawnDelay = FMath::Max(BaseDelay / PlayerCount, 0.5f);

	GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &AMonsterDirector::SpawnMonster, NextSpawnDelay, false);
}