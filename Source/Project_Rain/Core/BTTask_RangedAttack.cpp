#include "BTTask_RangedAttack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

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

	if (!TargetActor || !ProjectileClass) return EBTNodeResult::Failed;

	FVector SpawnLocation = Monster->GetActorLocation() + (Monster->GetActorForwardVector() * 100.0f);

	FVector Direction = (TargetActor->GetActorLocation() - SpawnLocation).GetSafeNormal();
	FRotator SpawnRotation = Direction.Rotation();

	GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, SpawnRotation);

	return EBTNodeResult::Succeeded;
}