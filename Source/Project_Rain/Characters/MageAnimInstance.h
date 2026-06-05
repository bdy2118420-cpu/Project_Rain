// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MageAnimInstance.generated.h"

/**
 * 
 */
class AMyCharacterBase;

UCLASS()
class PROJECT_RAIN_API UMageAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:

	UPROPERTY(BlueprintReadOnly, Category = "Character")
	TObjectPtr<AMyCharacterBase> MageCharacter;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Direction;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsOnGround;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsAiming = false;
	
};
