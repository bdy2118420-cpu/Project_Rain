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


	SetLifeSpan(5.0f);
}

