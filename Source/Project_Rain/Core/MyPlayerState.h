// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MyPlayerState.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGoldChangedSignature, int32, NewGold);

UCLASS()
class PROJECT_RAIN_API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	AMyPlayerState();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGoldChangedSignature OnGoldChanged;

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddGold(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	int32 GetCurrentGold() const { return CurrentGold; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentGold, VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 CurrentGold = 0;

	UFUNCTION()
	void OnRep_CurrentGold();
};
