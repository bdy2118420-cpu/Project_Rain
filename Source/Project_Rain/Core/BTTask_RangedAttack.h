// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_RangedAttack.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_RAIN_API UBTTask_RangedAttack : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_RangedAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// 블루프린트에서 어떤 투사체를 발사할지 결정할 수 있는 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Attack")
	TSubclassOf<AActor> ProjectileClass;
};
