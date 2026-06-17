// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseSkillComponent.h"
#include "SnapFreezeComponent.generated.h"

/**
 * 
 */
class UNiagaraSystem;
class UDecalComponent;
class ASnapFreezeArea;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_RAIN_API USnapFreezeComponent : public UBaseSkillComponent
{
	GENERATED_BODY()
public:
	USnapFreezeComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void StartSkill() override;
	virtual void ExecuteSkill() override; 

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "SnapFreeze")
	TObjectPtr<UNiagaraSystem> EffectAsset;

	UPROPERTY(EditDefaultsOnly, Category = "SnapFreeze")
	TSubclassOf<ASnapFreezeArea> AreaClass;

	UPROPERTY(EditAnywhere, Category = "SnapFreeze")
	float MaxRange = 1500.f;

	bool bIsAiming = false;

	
	UPROPERTY()
	TObjectPtr<UDecalComponent> IndicatorRef;

	UFUNCTION(Server, Reliable)
	void Server_Execute(FVector SpawnLocation, FRotator SpawnRotation);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Execute(FVector SpawnLocation, FRotator SpawnRotation);
};
