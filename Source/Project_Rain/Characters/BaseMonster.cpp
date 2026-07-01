#include "BaseMonster.h"
#include "../Component/HealthComponent.h" 
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "../Item/DropItemBase.h"
#include "../Core/MyGameState.h"

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

	if (HasAuthority() && HealthComp)
	{
		HealthComp->OnDeath.AddDynamic(this, &ABaseMonster::OnHealthComponentDead);
	}

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

	if (HasAuthority())
	{
		float DiffCoeff = 1.0f;

		AMyGameState* GS = GetWorld()->GetGameState<AMyGameState>();
		if (GS)
		{
			DiffCoeff = GS->GetDifficultyCoefficient();
		}

		float FinalGold = BaseRewardGold * DiffCoeff;
		float FinalEXP = BaseRewardEXP * DiffCoeff;

		FVector SpawnLoc = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
		FRotator SpawnRot = FRotator::ZeroRotator;
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (GoldDropClass)
		{
			int32 CoinCount = FMath::RandRange(2, 4);
			float GoldPerCoin = FinalGold / CoinCount;
			for (int32 i = 0; i < CoinCount; i++)
			{
				ADropItemBase* SpawnedGold = GetWorld()->SpawnActor<ADropItemBase>(GoldDropClass, SpawnLoc, SpawnRot, SpawnParams);
				if (SpawnedGold) SpawnedGold->InitializeDrop(EDropItemType::Gold, GoldPerCoin);
			}
		}

		if (EXPDropClass)
		{
			int32 ExpCount = FMath::RandRange(3, 5);
			float ExpPerOrb = FinalEXP / ExpCount;
			for (int32 i = 0; i < ExpCount; i++)
			{
				ADropItemBase* SpawnedEXP = GetWorld()->SpawnActor<ADropItemBase>(EXPDropClass, SpawnLoc, SpawnRot, SpawnParams);
				if (SpawnedEXP) SpawnedEXP->InitializeDrop(EDropItemType::EXP, ExpPerOrb);
			}
		}
	}

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

void ABaseMonster::OnHealthComponentDead(AActor* DeadActor)
{
	Die();
}

