#include "MyGameState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"

AMyGameState::AMyGameState()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AMyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyGameState, TotalSeconds);
	DOREPLIFETIME(AMyGameState, DifficultyCoefficient);
}

void AMyGameState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(TimeUpdateTimer, this, &AMyGameState::UpdateTimeAndDifficulty, 1.0f, true);
	}
}

void AMyGameState::UpdateTimeAndDifficulty()
{
	TotalSeconds++;

	DifficultyCoefficient = 1.0f + ((float)TotalSeconds / 60.0f) * DifficultyFactor;

	OnRep_TimeUpdated();
}

void AMyGameState::OnRep_TimeUpdated()
{
	if (OnDifficultyUpdated.IsBound())
	{
		OnDifficultyUpdated.Broadcast(TotalSeconds, DifficultyCoefficient);
	}
}