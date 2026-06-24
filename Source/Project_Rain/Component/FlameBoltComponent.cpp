#include "FlameBoltComponent.h"
#include "GameFramework/Character.h"
#include "../Projectiles/MyProjectileBase.h"
#include "../Characters/MyCharacterBase.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"

UFlameBoltComponent::UFlameBoltComponent()
{
	SetIsReplicatedByDefault(true);
}

void UFlameBoltComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentAmmo = MaxAmmo;

	if (GetOwner()->HasAuthority() && ProjectileClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = Cast<APawn>(GetOwner());

		for (int32 i = 0; i < MaxPoolSize; ++i)
		{
			AMyProjectileBase* SpawnedProj = GetWorld()->SpawnActor<AMyProjectileBase>(
				ProjectileClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (SpawnedProj) ProjectilePool.Add(SpawnedProj);
		}
	}
}

void UFlameBoltComponent::StartSkill()
{
	if (CurrentAmmo <= 0) return;

	CurrentAmmo--;
	if (OnAmmoChanged.IsBound()) OnAmmoChanged.Broadcast(CurrentAmmo);

	AMyCharacterBase* MyChar = Cast<AMyCharacterBase>(OwnerCharacter);
	if (MyChar)
	{
		MyChar->bIsAiming = true;
		GetWorld()->GetTimerManager().ClearTimer(AimingTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(AimingTimerHandle, this, &UFlameBoltComponent::StopAiming, 1.5f, false);
	}

	Server_PlayAttack(bIsRightHandNext);
	bIsRightHandNext = !bIsRightHandNext;

	if (!GetWorld()->GetTimerManager().IsTimerActive(RechargeTimerHandle))
	{
		GetWorld()->GetTimerManager().SetTimer(RechargeTimerHandle, this, &UFlameBoltComponent::RechargeAmmo, RechargeTime, true);
	}
}

void UFlameBoltComponent::StopAiming()
{
	AMyCharacterBase* MyChar = Cast<AMyCharacterBase>(OwnerCharacter);
	if (MyChar) MyChar->bIsAiming = false;
}

void UFlameBoltComponent::RechargeAmmo()
{
	if (CurrentAmmo < MaxAmmo)
	{
		CurrentAmmo++;
		if (OnAmmoChanged.IsBound()) OnAmmoChanged.Broadcast(CurrentAmmo);
	}
	if (CurrentAmmo >= MaxAmmo)
	{
		GetWorld()->GetTimerManager().ClearTimer(RechargeTimerHandle);
	}
}

float UFlameBoltComponent::GetRechargeProgress() const
{
	if (CurrentAmmo >= MaxAmmo) return 0.0f;
	if (GetWorld()->GetTimerManager().IsTimerActive(RechargeTimerHandle))
	{
		return GetWorld()->GetTimerManager().GetTimerRemaining(RechargeTimerHandle) / RechargeTime;
	}
	return 0.0f;
}

void UFlameBoltComponent::Server_PlayAttack_Implementation(bool bIsRight)
{
	Multicast_PlayAttack(bIsRight);

	if (ProjectilePool.Num() > 0)
	{
		AMyProjectileBase* ProjectileToFire = ProjectilePool[PoolIndex];
		AMyCharacterBase* MyChar = Cast<AMyCharacterBase>(OwnerCharacter);

		if (ProjectileToFire && MyChar)
		{
			UCameraComponent* Camera = MyChar->FindComponentByClass<UCameraComponent>();
			if (Camera)
			{
				FVector SpawnLocation = MyChar->GetActorLocation() + MyChar->GetActorForwardVector() * 120.f;
				FVector CameraLocation = Camera->GetComponentLocation();
				FVector TargetLocation = CameraLocation + (Camera->GetForwardVector() * 5000.f);

				FHitResult HitResult;
				FCollisionQueryParams CollisionParams;
				CollisionParams.AddIgnoredActor(MyChar);

				if (GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, TargetLocation, ECC_Visibility, CollisionParams))
				{
					TargetLocation = HitResult.ImpactPoint;
				}

				FRotator SpawnRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, TargetLocation);
				ProjectileToFire->ActivateProjectile(SpawnLocation, SpawnRotation);
			}
		}
		PoolIndex = (PoolIndex + 1) % MaxPoolSize;
	}
}

void UFlameBoltComponent::Multicast_PlayAttack_Implementation(bool bIsRight)
{
	AMyCharacterBase* MyChar = Cast<AMyCharacterBase>(OwnerCharacter);
	if (!MyChar) return;

	UAnimMontage* TargetMontage = bIsRight ? ShootRightMontage : ShootLeftMontage;
	if (TargetMontage)
	{
		MyChar->PlayAnimMontage(TargetMontage, (TargetMontage == ShootLeftMontage) ? 2.0f : 1.0f);
	}
}