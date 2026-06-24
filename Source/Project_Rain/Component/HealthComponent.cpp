// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	if (GetOwnerRole() == ROLE_Authority)
	{
		CurrentHealth = MaxHealth;

		
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::TakeDamage);
		}
	}
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHealthComponent, CurrentHealth);
}

void UHealthComponent::OnRep_CurrentHealth()
{
	OnHealthChanged.Broadcast(this, CurrentHealth, MaxHealth);
}

void UHealthComponent::TakeDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.0f || bIsDead) return;

	if (InstigatedBy)
	{
		AActor* Attacker = InstigatedBy->GetPawn();
		if (!Attacker) Attacker = DamageCauser;

		if (Attacker)
		{
			if (DamagedActor->ActorHasTag(TEXT("Player")) && Attacker->ActorHasTag(TEXT("Player"))) return;

			if (DamagedActor->ActorHasTag(TEXT("Monster")) && Attacker->ActorHasTag(TEXT("Monster"))) return;
		}
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);

	OnHealthChanged.Broadcast(this, CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.0f && !bIsDead)
	{
		bIsDead = true;
		OnDeath.Broadcast(GetOwner());
	}
}

float UHealthComponent::GetHealthPercent() const
{
	if (MaxHealth <= 0.0f) return 0.0f;
	return CurrentHealth / MaxHealth;
}

void UHealthComponent::InitHealth(float NewMaxHealth)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		MaxHealth = NewMaxHealth;
		CurrentHealth = MaxHealth;
		bIsDead = false; 

		OnHealthChanged.Broadcast(this, CurrentHealth, MaxHealth);
	}
}
