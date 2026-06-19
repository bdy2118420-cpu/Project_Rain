#include "MonsterDirector.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "../Characters/BaseMonster.h"
#include "GameFramework/Character.h"

AMonsterDirector::AMonsterDirector()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMonsterDirector::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &AMonsterDirector::SpawnMonster, SpawnInterval, true);
}

void AMonsterDirector::SpawnMonster()
{
	if (!MonsterClassToSpawn) return;

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (!Player) return;

	FVector PlayerLocation = Player->GetActorLocation();

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSystem)
	{
		FNavLocation RandomLocation;
		if (NavSystem->GetRandomReachablePointInRadius(PlayerLocation, SpawnRadius, RandomLocation))
		{
			GetWorld()->SpawnActor<ABaseMonster>(MonsterClassToSpawn, RandomLocation.Location, FRotator::ZeroRotator);
		}
	}
}