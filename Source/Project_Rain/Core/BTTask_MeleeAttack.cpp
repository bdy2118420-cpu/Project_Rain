#include "BTTask_MeleeAttack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Engine/Engine.h" 

UBTTask_MeleeAttack::UBTTask_MeleeAttack()
{
	NodeName = TEXT("Melee Attack");
}

EBTNodeResult::Type UBTTask_MeleeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	ACharacter* Monster = Cast<ACharacter>(AIController->GetPawn());
	if (!Monster)
	{
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(FName("TargetActor")));

	if (!TargetActor)
	{
		return EBTNodeResult::Failed;
	}

	float Distance = FVector::Distance(Monster->GetActorLocation(), TargetActor->GetActorLocation());

	if (Distance <= AttackRange)
	{
		UGameplayStatics::ApplyDamage(TargetActor, AttackDamage, AIController, Monster, UDamageType::StaticClass());

		

		return EBTNodeResult::Succeeded;
	}
	else
	{

		return EBTNodeResult::Failed;
	}
}