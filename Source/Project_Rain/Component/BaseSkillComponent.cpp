#include "BaseSkillComponent.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

UBaseSkillComponent::UBaseSkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBaseSkillComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());

	bIsReady = true;
}

void UBaseSkillComponent::StartSkill()
{
	if (!bIsReady)
	{
		return;
	}
}

void UBaseSkillComponent::ExecuteSkill()
{
	if (!bIsReady)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	bIsReady = false;

	if (OnSkillCooldownStarted.IsBound())
	{
		OnSkillCooldownStarted.Broadcast(CooldownTime);
	}

	World->GetTimerManager().SetTimer(CooldownTimerHandle, this, &UBaseSkillComponent::FinishCooldown, CooldownTime, false);
}

void UBaseSkillComponent::StopSkill()
{
}

void UBaseSkillComponent::FinishCooldown()
{
	bIsReady = true;

	if (OnSkillReady.IsBound())
	{
		OnSkillReady.Broadcast();
	}
}