#include "MyPlayerState.h"
#include "Net/UnrealNetwork.h"

AMyPlayerState::AMyPlayerState()
{
	bReplicates = true;
}

void AMyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMyPlayerState, CurrentGold);
}

void AMyPlayerState::AddGold(int32 Amount)
{
	if (HasAuthority())
	{
		CurrentGold += Amount;
		OnRep_CurrentGold();
	}
}

void AMyPlayerState::OnRep_CurrentGold()
{
	if (OnGoldChanged.IsBound())
	{
		OnGoldChanged.Broadcast(CurrentGold);
	}
}