// Fill out your copyright notice in the Description page of Project Settings.

#include "MyProjectileBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMyProjectileBase::AMyProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	CollisionComponent->InitSphereRadius(15.f);
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootComponent = CollisionComponent;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 1500.f;
	ProjectileMovement->MaxSpeed = 1500.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.f;

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	ProjectileMovement->bAutoActivate = false;

	bReplicates = true;
	SetReplicateMovement(true);
	bAlwaysRelevant = true;
}

void AMyProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	CollisionComponent->OnComponentHit.AddDynamic(this, &AMyProjectileBase::OnProjectileHit);
}

void AMyProjectileBase::ActivateProjectile(const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	SetActorLocationAndRotation(SpawnLocation, SpawnRotation);
	ProjectileMovement->Velocity = SpawnRotation.Vector() * ProjectileMovement->InitialSpeed;

	GetWorldTimerManager().SetTimer(LifeTimeTimerHandle, this, &AMyProjectileBase::DeactivateProjectile, MaxLifeTime, false);

	bIsActive = true;
	ForceNetUpdate();
	OnRep_IsActive();
}

void AMyProjectileBase::DeactivateProjectile()
{
	GetWorldTimerManager().ClearTimer(LifeTimeTimerHandle);
	GetWorldTimerManager().ClearTimer(ExplosionTimerHandle); // 혹시 돌고 있는 폭발 타이머도 초기화

	bIsActive = false;
	OnRep_IsActive();
}

void AMyProjectileBase::SetChargeScale(float ChargeRatio)
{
	float FinalScale = FMath::Lerp(1.0f, 3.5f, ChargeRatio);
	SetActorScale3D(FVector(FinalScale));

	CurrentExplosionDamage = FMath::Lerp(MinChargeDamage, MaxChargeDamage, ChargeRatio);
}

void AMyProjectileBase::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor != GetOwner() && OtherActor != GetInstigator())
	{
		if (HasAuthority())
		{
			
			if (bIsMultiHitExplosion)
			{
				if (bHasExploded)
				{
					return;
				}
				ProjectileMovement->StopMovementImmediately(); 
				CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); 

				StartExplosion();
			}
		
			else
			{
				UGameplayStatics::ApplyDamage(OtherActor, BaseDamage, GetInstigatorController(), this, UDamageType::StaticClass());
				DeactivateProjectile();
			}
		}
	}
}


void AMyProjectileBase::StartExplosion()
{
	CurrentHitCount = 0;

	ExecuteExplosionHit(); 

	GetWorldTimerManager().SetTimer(ExplosionTimerHandle, this, &AMyProjectileBase::ExecuteExplosionHit, ExplosionInterval, true);
}

void AMyProjectileBase::ExecuteExplosionHit()
{
	CurrentHitCount++;

	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(GetInstigator());

	UGameplayStatics::ApplyRadialDamage(
		this,
		CurrentExplosionDamage,
		GetActorLocation(),
		ExplosionRadius,
		UDamageType::StaticClass(),
		IgnoredActors,
		this,
		GetInstigatorController(),
		true,
		ECC_Visibility
	);

	ReceiveOnExplosionHit();

	if (CurrentHitCount >= MaxExplosionHits)
	{
		GetWorldTimerManager().ClearTimer(ExplosionTimerHandle);
		DeactivateProjectile();
	}
}

void AMyProjectileBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMyProjectileBase, bIsActive);
}

void AMyProjectileBase::OnRep_IsActive()
{
	if (bIsActive)
	{
		bHasExploded = false; 
		SetActorHiddenInGame(false);
		SetActorHiddenInGame(false);
		SetActorEnableCollision(true);

		if (ProjectileMovement)
		{
			ProjectileMovement->SetUpdatedComponent(CollisionComponent);
			ProjectileMovement->SetComponentTickEnabled(true);
			ProjectileMovement->Velocity = GetActorForwardVector() * ProjectileMovement->InitialSpeed;
			ProjectileMovement->Activate();
		}
		ReceiveOnActivated();
	}
	else
	{
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);

		if (ProjectileMovement)
		{
			ProjectileMovement->Velocity = FVector::ZeroVector;
			ProjectileMovement->SetComponentTickEnabled(false);
			ProjectileMovement->Deactivate();
		}
		ReceiveOnDeactivated();
	}
}