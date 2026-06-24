#include "BTTask_RangedAttack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Engine/Engine.h" 
#include "ProjectilePoolSubsystem.h"
#include "../Projectiles/MonsterProjectile.h"

UBTTask_RangedAttack::UBTTask_RangedAttack()
{
	NodeName = TEXT("Ranged Attack");
}

EBTNodeResult::Type UBTTask_RangedAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	ACharacter* Monster = Cast<ACharacter>(AIController->GetPawn());
	if (!Monster) return EBTNodeResult::Failed;

	AActor* TargetActor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(FName("TargetActor")));

	if (!ProjectileClass)
	{
		return EBTNodeResult::Failed;
	}

	if (!TargetActor) return EBTNodeResult::Failed;

	if (!TargetActor->ActorHasTag(TEXT("Player")))
	{
		return EBTNodeResult::Failed;
	}
	float Distance = FVector::Distance(Monster->GetActorLocation(), TargetActor->GetActorLocation());

	
	if (Distance > AttackRange)
	{
		return EBTNodeResult::Failed;
	}

	FVector SpawnLocation = Monster->GetActorLocation() + (Monster->GetActorForwardVector() * 150.0f);
	FVector Direction = (TargetActor->GetActorLocation() - SpawnLocation).GetSafeNormal();
	FRotator SpawnRotation = Direction.Rotation();

	UProjectilePoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UProjectilePoolSubsystem>();

	if (PoolSubsystem)
	{
	
		PoolSubsystem->SpawnFromPool(ProjectileClass, SpawnLocation, SpawnRotation, Monster);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}