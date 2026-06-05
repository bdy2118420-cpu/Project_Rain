// Fill out your copyright notice in the Description page of Project Settings.


#include "MyProjectileBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "TimerManager.h"

// Sets default values
AMyProjectileBase::AMyProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

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
	ProjectileMovement->ProjectileGravityScale = 0.f; // 중력 0으로 일직선 비행

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	ProjectileMovement->bAutoActivate = false;

}

void AMyProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	// 충돌 이벤트 바인딩
	CollisionComponent->OnComponentHit.AddDynamic(this, &AMyProjectileBase::OnProjectileHit);
}

void AMyProjectileBase::ActivateProjectile(const FVector& SpawnLocation, const FRotator& SpawnRotation)
{

	SetActorLocationAndRotation(SpawnLocation, SpawnRotation);


	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);


	ProjectileMovement->SetUpdatedComponent(CollisionComponent);
	ProjectileMovement->Velocity = SpawnRotation.Vector() * ProjectileMovement->InitialSpeed;
	ProjectileMovement->Activate();

	GetWorldTimerManager().SetTimer(LifeTimeTimerHandle, this, &AMyProjectileBase::DeactivateProjectile, MaxLifeTime, false);
}

void AMyProjectileBase::DeactivateProjectile()
{

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);


	ProjectileMovement->Velocity = FVector::ZeroVector;
	ProjectileMovement->Deactivate();


	GetWorldTimerManager().ClearTimer(LifeTimeTimerHandle);
}

void AMyProjectileBase::SetChargeScale(float ChargeRatio)
{
	float FinalScale = FMath::Lerp(1.0f, 3.5f, ChargeRatio);

	SetActorScale3D(FVector(FinalScale));
}

void AMyProjectileBase::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor != GetOwner())
	{

		if (HasAuthority())
		{
		}

		
		DeactivateProjectile();
	}
}


