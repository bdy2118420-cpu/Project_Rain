#include "SnapFreezeComponent.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "../Projectiles/SnapFreezeArea.h"

USnapFreezeComponent::USnapFreezeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	CooldownTime = 7.0f;
}

void USnapFreezeComponent::BeginPlay()
{
	Super::BeginPlay();
	if (OwnerCharacter)
	{
		IndicatorRef = OwnerCharacter->FindComponentByClass<UDecalComponent>();
	}
}

void USnapFreezeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsAiming && IndicatorRef && OwnerCharacter)
	{
		UCameraComponent* Camera = OwnerCharacter->FindComponentByClass<UCameraComponent>();
		if (Camera)
		{
			FVector StartLoc = Camera->GetComponentLocation();
			FVector EndLoc = StartLoc + (Camera->GetForwardVector() * MaxRange);

			FHitResult HitResult;
			FCollisionQueryParams CollisionParams;
			CollisionParams.AddIgnoredActor(OwnerCharacter);

			if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc, EndLoc, ECC_Visibility, CollisionParams))
			{
				IndicatorRef->SetWorldLocation(HitResult.ImpactPoint);
				IndicatorRef->SetWorldRotation(FRotator(-90.f, Camera->GetComponentRotation().Yaw, 0.f));
			}
			else
			{
				IndicatorRef->SetWorldLocation(EndLoc);
			}
		}
	}
}

void USnapFreezeComponent::StartSkill()
{
	if (!bIsReady) return;
	bIsAiming = true;
	if (IndicatorRef) IndicatorRef->SetVisibility(true);
}

void USnapFreezeComponent::ExecuteSkill()
{
	if (!bIsAiming || !bIsReady) return;

	bIsAiming = false;
	if (IndicatorRef)
	{
		IndicatorRef->SetVisibility(false);
		FVector SpawnLoc = IndicatorRef->GetComponentLocation();
		FRotator SpawnRot = FRotator(0.f, IndicatorRef->GetComponentRotation().Yaw, 0.f);

		Server_Execute(SpawnLoc, SpawnRot);
	}

	// 부모 함수 호출 (쿨타임 시작 및 UI 업데이트)
	Super::ExecuteSkill();
}

void USnapFreezeComponent::Server_Execute_Implementation(FVector SpawnLocation, FRotator SpawnRotation)
{
	if (AreaClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = Cast<APawn>(OwnerCharacter);

		GetWorld()->SpawnActor<ASnapFreezeArea>(AreaClass, SpawnLocation, SpawnRotation, SpawnParams);
	}
}

void USnapFreezeComponent::Multicast_Execute_Implementation(FVector SpawnLocation, FRotator SpawnRotation)
{
	
}