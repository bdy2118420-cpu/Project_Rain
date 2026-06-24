#include "BaseMonster.h"
#include "../Component/HealthComponent.h" 
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "BrainComponent.h"

ABaseMonster::ABaseMonster()
{
	
	PrimaryActorTick.bCanEverTick = false;


	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;


	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));

	GetMesh()->bEnableUpdateRateOptimizations = true;

	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 480.0f, 0.0f);
}

void ABaseMonster::BeginPlay()
{
	Super::BeginPlay();
	DefaultMeshLocation = GetMesh()->GetRelativeLocation();
	DefaultMeshRotation = GetMesh()->GetRelativeRotation();

}

void ABaseMonster::Die()
{

	if (bIsDead) return;
	bIsDead = true;


	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController && AIController->GetBrainComponent())
	{
		AIController->GetBrainComponent()->StopLogic(TEXT("Monster Died"));
	}


	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true);


	ReceiveOnDied();

	
	if (OnMonsterDied.IsBound())
	{
		OnMonsterDied.Broadcast(this);
	}
}

void ABaseMonster::ActivateMonster(FVector SpawnLocation, FRotator SpawnRotation, float DifficultyMultiplier)
{
	bIsActive = true;

	float ScaledHealth = BaseHealth * DifficultyMultiplier;
	CurrentDamage = BaseDamage * DifficultyMultiplier;

	if (HealthComp)
	{
		HealthComp->InitHealth(ScaledHealth); 
	}

	SetActorLocationAndRotation(SpawnLocation, SpawnRotation);
	SetActorHiddenInGame(false);


	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	GetMesh()->SetSimulatePhysics(false);
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
	GetMesh()->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	GetMesh()->SetRelativeLocationAndRotation(DefaultMeshLocation, DefaultMeshRotation);

	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController && AIController->GetBrainComponent())
	{
		AIController->GetBrainComponent()->RestartLogic();
	}
}

void ABaseMonster::DeactivateMonster()
{
	bIsActive = false;

	GetMesh()->SetSimulatePhysics(false);

	SetActorHiddenInGame(true);
	SetActorLocation(FVector(0.0f, 0.0f, -10000.0f));

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController && AIController->GetBrainComponent())
	{
		AIController->GetBrainComponent()->StopLogic(TEXT("Deactivated"));
	}
}

