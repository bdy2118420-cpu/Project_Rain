// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BaseSkillComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillCooldownStartedSignature, float, CooldownDuration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSkillReadySignature);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_RAIN_API UBaseSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBaseSkillComponent();
	// UI와 연결될 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Skill|UI")
	FOnSkillCooldownStartedSignature OnSkillCooldownStarted;

	UPROPERTY(BlueprintAssignable, Category = "Skill|UI")
	FOnSkillReadySignature OnSkillReady;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	TObjectPtr<ACharacter> OwnerCharacter;

	// --- 공통 스킬 스탯 ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Stats")
	float BaseDamage = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Stats")
	float CooldownTime = 5.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Skill|State")
	bool bIsReady = true;

	FTimerHandle CooldownTimerHandle;

	// 쿨타임 완료 처리 함수
	virtual void FinishCooldown();

public:	
	// Called every frame

	virtual void StartSkill();

	// 스킬 실행 (실제 발사, 타격 판정 등)
	virtual void ExecuteSkill();

	// 스킬 중지 (키를 떼거나, 캔슬되었을 때)
	virtual void StopSkill();

	// 스킬 상태 반환
	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool IsSkillReady() const { return bIsReady; }

		
};
