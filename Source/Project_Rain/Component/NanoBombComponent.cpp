#include "NanoBombComponent.h"
#include "GameFramework/Character.h"
#include "../Projectiles/MyProjectileBase.h"
#include "../Characters/MyCharacterBase.h"

UNanoBombComponent::UNanoBombComponent()
{
	SetIsReplicatedByDefault(true);

	CooldownTime = 5.0f; 
}

void UNanoBombComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority() && ProjectileClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = Cast<APawn>(GetOwner());

		for (int32 i = 0; i < MaxPoolSize; ++i)
		{
			AMyProjectileBase* SpawnedProj = GetWorld()->SpawnActor<AMyProjectileBase>(
				ProjectileClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

			if (SpawnedProj)
			{
				ProjectilePool.Add(SpawnedProj);
			}
		}
	}
}

void UNanoBombComponent::StartSkill()
{
	if (!bIsReady) return;

	bIsCharging = true;
	ChargeStartTime = GetWorld()->GetTimeSeconds();

	Server_StartCharge();
}

void UNanoBombComponent::StopSkill()
{
	if (!bIsCharging || !bIsReady) return;

	bIsCharging = false;

	
	float CurrentTime = GetWorld()->GetTimeSeconds();
	CurrentChargeRatio = FMath::Clamp((CurrentTime - ChargeStartTime) / MaxChargeTime, 0.0f, 1.0f);

	ExecuteSkill();


	Server_Fire(CurrentChargeRatio);
}

void UNanoBombComponent::ExecuteSkill()
{
	Super::ExecuteSkill();
}


void UNanoBombComponent::Server_StartCharge_Implementation()
{
	Multicast_StartCharge();
}

void UNanoBombComponent::Multicast_StartCharge_Implementation()
{
	AMyCharacterBase* MyChar = Cast<AMyCharacterBase>(OwnerCharacter);
	if (!MyChar) return;

	if (ChargeMontage)
	{
		MyChar->PlayAnimMontage(ChargeMontage, 1.0f);
	}

}

void UNanoBombComponent::Server_Fire_Implementation(float ChargeRatio)
{
	Multicast_Fire();

	if (ProjectilePool.Num() > 0)
	{
		AMyProjectileBase* ProjectileToFire = ProjectilePool[PoolIndex];
		if (ProjectileToFire)
		{
			FVector SpawnLocation = OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorForwardVector() * 120.f;
			FRotator SpawnRotation = OwnerCharacter->GetControlRotation();

			ProjectileToFire->SetChargeScale(ChargeRatio);
			ProjectileToFire->ActivateProjectile(SpawnLocation, SpawnRotation);
		}
		PoolIndex = (PoolIndex + 1) % MaxPoolSize;
	}
}

void UNanoBombComponent::Multicast_Fire_Implementation()
{
	if (OwnerCharacter && FireMontage)
	{
		if (ChargeMontage) OwnerCharacter->StopAnimMontage(ChargeMontage);
		OwnerCharacter->PlayAnimMontage(FireMontage, 1.5f);
	}
}