#include "MonsterProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AMonsterProjectile::AMonsterProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicatingMovement(true);
	bIsActive = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(15.0f);
	CollisionComp->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootComponent = CollisionComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComp"));
	MovementComp->UpdatedComponent = CollisionComp;
	MovementComp->InitialSpeed = 1500.0f;
	MovementComp->MaxSpeed = 1500.0f;
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->bAutoActivate = false; 
}

void AMonsterProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CollisionComp->OnComponentHit.AddDynamic(this, &AMonsterProjectile::OnHit);
	}
}

void AMonsterProjectile::ActivateProjectile(FVector SpawnLocation, FRotator SpawnRotation, APawn* NewInstigator)
{
	bIsActive = true;
	SetInstigator(NewInstigator);


	SetActorLocationAndRotation(SpawnLocation, SpawnRotation);

	
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MovementComp->SetUpdatedComponent(RootComponent);
	MovementComp->Velocity = SpawnRotation.Vector() * MovementComp->InitialSpeed;
	MovementComp->Activate();

	GetWorld()->GetTimerManager().SetTimer(LifeSpanTimer, this, &AMonsterProjectile::DeactivateProjectile, 3.0f, false);
}


void AMonsterProjectile::DeactivateProjectile()
{
	bIsActive = false;

	
	GetWorld()->GetTimerManager().ClearTimer(LifeSpanTimer);
	MovementComp->Deactivate();
	MovementComp->Velocity = FVector::ZeroVector;

	
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorLocation(FVector(0.0f, 0.0f, -10000.0f));
}


void AMonsterProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bIsActive) return;

	if (OtherActor && OtherActor != this && OtherActor != GetInstigator())
	{
	
		if (OtherActor->ActorHasTag(TEXT("Player")))
		{
			UGameplayStatics::ApplyDamage(OtherActor, DamageAmount, GetInstigatorController(), this, UDamageType::StaticClass());
		}

		DeactivateProjectile();
	}
}