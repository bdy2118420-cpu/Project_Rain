#include "FlameThrowerComponent.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UFlameThrowerComponent::UFlameThrowerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	CooldownTime = 8.0f;
}

void UFlameThrowerComponent::BeginPlay()
{
	Super::BeginPlay();

	// 캐릭터에 붙어있는 NiagaraComponent 중 이름에 Flamethrower가 들어간 것을 찾습니다.
	if (OwnerCharacter)
	{
		TArray<UNiagaraComponent*> Comps;
		OwnerCharacter->GetComponents<UNiagaraComponent>(Comps);
		for (UNiagaraComponent* Comp : Comps)
		{
			if (Comp->GetName().Contains(TEXT("Flamethrower")))
			{
				NiagaraRefs.Add(Comp);
			}
		}
	}
}

void UFlameThrowerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsUsing && OwnerCharacter)
	{
		UCameraComponent* Camera = OwnerCharacter->FindComponentByClass<UCameraComponent>();
		if (Camera)
		{
			FVector StartLoc = Camera->GetComponentLocation();
			FVector TargetLoc = StartLoc + (Camera->GetForwardVector() * 5000.f) + (Camera->GetUpVector() * 50.f);

			FHitResult HitResult;
			FCollisionQueryParams CollisionParams;
			CollisionParams.AddIgnoredActor(OwnerCharacter);

			if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc, TargetLoc, ECC_Visibility, CollisionParams))
			{
				TargetLoc = HitResult.ImpactPoint;
			}

			// 각 손의 파티클 위치와 회전 업데이트
			for (UNiagaraComponent* Niagara : NiagaraRefs)
			{
				FName SocketName = Niagara->GetName().Contains(TEXT("Left")) ? FName("hand_l") : FName("hand_r");
				FVector HandLoc = OwnerCharacter->GetMesh()->GetSocketLocation(SocketName);
				FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(HandLoc, TargetLoc);

				Niagara->SetWorldLocation(HandLoc);
				Niagara->SetWorldRotation(LookAt);
			}
		}
	}
}

void UFlameThrowerComponent::StartSkill()
{
	if (!bIsReady) return;

	bIsUsing = true;
	GetWorld()->GetTimerManager().SetTimer(DurationTimerHandle, this, &UFlameThrowerComponent::StopSkill, MaxDuration, false);

	Server_Start();
}

void UFlameThrowerComponent::StopSkill()
{
	if (!bIsUsing) return;

	bIsUsing = false;
	GetWorld()->GetTimerManager().ClearTimer(DurationTimerHandle);

	if (OwnerCharacter->IsLocallyControlled())
	{
		Server_Stop();
	}

	// 사용이 끝났으므로 쿨타임 돌리기
	ExecuteSkill();
}

void UFlameThrowerComponent::ApplyFlamethrowerDamage()
{
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority())
	{
		return;
	}

	UCameraComponent* Camera = OwnerCharacter->FindComponentByClass<UCameraComponent>();
	if (!Camera)
	{
		return;
	}

	FVector StartLoc = Camera->GetComponentLocation() + (Camera->GetForwardVector() * 100.f);
	FVector EndLoc = StartLoc + (Camera->GetForwardVector() * FlameRange);

	TArray<FHitResult> HitResults;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(FlameRadius);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter); 

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);

	bool bHit = GetWorld()->SweepMultiByObjectType(HitResults, StartLoc, EndLoc, FQuat::Identity, ObjectQueryParams, SphereShape, Params);

	TSet<AActor*> HitActors;

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();

			if (HitActor && !HitActors.Contains(HitActor))
			{
				HitActors.Add(HitActor);

				UGameplayStatics::ApplyDamage(HitActor, BaseDamage, OwnerCharacter->GetController(), OwnerCharacter, UDamageType::StaticClass());
			}
		}
	}
}

void UFlameThrowerComponent::Server_Start_Implementation()
{
	Multicast_Start();

	GetWorld()->GetTimerManager().SetTimer(DamageTimerHandle, this, &UFlameThrowerComponent::ApplyFlamethrowerDamage, DamageInterval, true);

	ApplyFlamethrowerDamage();
}

void UFlameThrowerComponent::Multicast_Start_Implementation()
{
	if (OwnerCharacter && Montage && !OwnerCharacter->IsLocallyControlled())
	{
		OwnerCharacter->PlayAnimMontage(Montage, 1.0f);
	}
	else if (OwnerCharacter && Montage && OwnerCharacter->IsLocallyControlled())
	{
		OwnerCharacter->PlayAnimMontage(Montage, 1.0f);
	}

	bIsUsing = true;
	for (UNiagaraComponent* Niagara : NiagaraRefs)
	{
		Niagara->Activate(true);
	}
}

void UFlameThrowerComponent::Server_Stop_Implementation()
{
	Multicast_Stop();

	GetWorld()->GetTimerManager().ClearTimer(DamageTimerHandle);
}

void UFlameThrowerComponent::Multicast_Stop_Implementation()
{
	bIsUsing = false;
	for (UNiagaraComponent* Niagara : NiagaraRefs)
	{
		Niagara->DeactivateImmediate();
	}
	if (OwnerCharacter && Montage)
	{
		OwnerCharacter->StopAnimMontage(Montage);
	}
}