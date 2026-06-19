// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MeleeAttack.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_RAIN_API UBTTask_MeleeAttack : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_MeleeAttack();

protected:
	// 태스크가 실행될 때 호출되는 핵심 함수
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Attack")
	float AttackRange = 200.0f; // 공격 사거리

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Attack")
	float AttackDamage = 15.0f; // 공격 데미지
};
