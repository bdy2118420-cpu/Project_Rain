// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MyGameState.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDifficultyUpdatedSignature, int32, CurrentSeconds, float, CurrentCoefficient);

UCLASS()
class PROJECT_RAIN_API AMyGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	AMyGameState();

	UPROPERTY(BlueprintAssignable, Category = "Difficulty")
	FOnDifficultyUpdatedSignature OnDifficultyUpdated;

	UFUNCTION(BlueprintCallable, Category = "Difficulty")
	float GetDifficultyCoefficient() const { return DifficultyCoefficient; }

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	float DifficultyFactor = 0.2f;

	UPROPERTY(ReplicatedUsing = OnRep_TimeUpdated, VisibleAnywhere, BlueprintReadOnly, Category = "Difficulty")
	int32 TotalSeconds = 0;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Difficulty")
	float DifficultyCoefficient = 1.0f;

	UFUNCTION()
	void OnRep_TimeUpdated();

private:
	FTimerHandle TimeUpdateTimer;

	void UpdateTimeAndDifficulty();
};
