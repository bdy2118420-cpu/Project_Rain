// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseMonster.generated.h"

class UHealthComponent;
class UBehaviorTree;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterDiedSignature, class ABaseMonster*, DeadMonster);

UCLASS()
class PROJECT_RAIN_API ABaseMonster : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseMonster();

	UPROPERTY(BlueprintAssignable, Category = "Monster|Events")
	FOnMonsterDiedSignature OnMonsterDied;

	
	UPROPERTY(EditDefaultsOnly, Category = "Monster|AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

protected:
	virtual void BeginPlay() override;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHealthComponent> HealthComp;

	UPROPERTY(BlueprintReadOnly, Category = "Monster|State")
	bool bIsDead = false;


	UFUNCTION(BlueprintCallable, Category = "Monster|State")
	virtual void Die();


	UFUNCTION(BlueprintImplementableEvent, Category = "Monster|Events")
	void ReceiveOnDied();
};
